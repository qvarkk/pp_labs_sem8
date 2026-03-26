#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <mpi.h>

using namespace std;

void build_counts_displs(int n, int size, int* scounts, int* displs) {
    int base = n / size;
    int mod  = n % size;

    int offset = 0;
    for (int i = 0; i < size; i++) {
        scounts[i] = base + (i < mod ? 1 : 0);
        displs[i] = offset;
        offset += scounts[i];
    }
}

void init_data(int n, double* A, double* x) {
    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i * n + j] = (double)rand() / RAND_MAX;
        }
        x[i] = (double)rand() / RAND_MAX;
    }
}

double mult_mv_seq(int n, double* A, double* x, double* y) {
    double t0 = MPI_Wtime();

    for (int k = 0; k < 100; ++k)
    {
        for (int i = 0; i < n; ++i)
        {
            double sum = 0.0;
            double* row = A + i * n;

            for (int j = 0; j < n; ++j)
            {
                sum += row[j] * x[j];
            }
            y[i] = sum;
        }

        memcpy(x, y, n * sizeof(double));
    }

    double t1 = MPI_Wtime();
    return t1 - t0;
}

double mult_mv_mpi(int n, double* A_global, double* x0_global, double* y_global, int rank, int size) {
    int* scounts_rows = new int[size];
    int* displs_rows = new int[size];
    build_counts_displs(n, size, scounts_rows, displs_rows);

    int local_n = scounts_rows[rank];

    int* scounts_A = new int[size];
    int* displs_A = new int[size];
    for (int i = 0; i < size; i++) {
        scounts_A[i] = scounts_rows[i] * n;
        displs_A[i] = displs_rows[i] * n;
    }

    int local_A_size = local_n * n;
    int local_y_size = local_n;

    double* A_local = new double[local_A_size];
    double* x = new double[n];
    double* y_local = new double[local_y_size];

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    MPI_Scatterv(static_cast<double*>(A_global), scounts_A, displs_A, MPI_DOUBLE,
                 A_local, local_n * n, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    if (rank == 0) {
        memcpy(x, x0_global, n * sizeof(double));
    }

    MPI_Bcast(x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int k = 0; k < 100; k++) {
        for (int i = 0; i < local_n; i++) {
            double sum = 0.0;
            double* row = A_local + i * n;

            for (int j = 0; j < n; ++j)
            {
                sum += row[j] * x[j];
            }
            y_local[i] = sum;
        }

        MPI_Gatherv(y_local, local_n, MPI_DOUBLE,
                    (rank == 0 ? x : nullptr), scounts_rows, displs_rows, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);

        if (k != 99) {
            MPI_Bcast(x, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }
    }

    double local_time = MPI_Wtime() - t0;

    double parallel_time = 0.0;
    MPI_Reduce(&local_time, &parallel_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        memcpy(y_global, x, n * sizeof(double));
    }

    delete[] A_local;
    delete[] x;
    delete[] y_local;

    delete[] scounts_rows;
    delete[] displs_rows;
    delete[] scounts_A;
    delete[] displs_A;

    return parallel_time;
}

double max_abs_error(int n, const double* a, const double* b) {
    double max = 0.0;
    for (int i = 0; i < n; i++) {
        double err = fabs(a[i] - b[i]);
        if (err > max) max = err;
    }
    return max;
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = 2500;
 
    double* A = nullptr;
    double* x_init = nullptr;

    double* x_seq = nullptr;
    double* y_seq = nullptr;

    double* y_par = nullptr;

    if (rank == 0) {
        A = new double[n * n];
        x_init = new double[n];

        x_seq = new double[n];
        y_seq = new double[n];
        y_par = new double[n];

        init_data(n, A, x_init);

        memcpy(x_seq, x_init, n * sizeof(double));
    }

    double t_seq = 0.0;
    if (rank == 0) {
        t_seq = mult_mv_seq(n, A, x_seq, y_seq);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    double t_par = mult_mv_mpi(n, A, x_init, y_par, rank, size);

    if (rank == 0) {
        double err = max_abs_error(n, y_seq, y_par);

        cout << "n               = " << n << endl;
        cout << "MPI processes   = " << size << endl;
        cout << "Sequential time = " << t_seq << " s" << endl;
        cout << "Parallel time   = " << t_par << " s" << endl;
        cout << "Improve         = " << (t_seq / t_par) << endl;
        cout << "Max abs error   = " << err << endl;
    }

    if (rank == 0) {
        delete[] A;
        delete[]x_init;
        delete[]x_seq;
        delete[]y_seq;
        delete[]y_par;
    }

    MPI_Finalize();
    return 0;
}