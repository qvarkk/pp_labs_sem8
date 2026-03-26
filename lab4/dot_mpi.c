#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void build_scounts_displs(int n, int size, int* scounts, int* displs) {
    int base = n / size;
    int mod  = n % size;

    int offset = 0;
    for (int i = 0; i < size; i++) {
        scounts[i] = base + (i < mod ? 1 : 0);
        displs[i] = offset;
        offset += scounts[i];
    }
}

void* vmalloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        printf("malloc failed\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    return p;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 10000000;

    int *scounts = (int*)vmalloc(size * sizeof(int));
    int *displs = (int*)vmalloc(size * sizeof(int));
    build_scounts_displs(N, size, scounts, displs);

    int local_n = scounts[rank];

    int *local_a = (int*)vmalloc(local_n * sizeof(int));
    int *local_b = (int*)vmalloc(local_n * sizeof(int));

    int *a = NULL;
    int *b = NULL;

    if (rank == 0) {
        a = (int*)vmalloc(N * sizeof(int));
        b = (int*)vmalloc(N * sizeof(int));

        srand(time(NULL) + rank);

        for (int i = 0; i < N; i++) {
            a[i] = i + 1;
            b[i] = rand() % 100;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    MPI_Scatterv(a, scounts, displs, MPI_INT,
                 local_a, local_n, MPI_INT,
                 0, MPI_COMM_WORLD);

    MPI_Scatterv(b, scounts, displs, MPI_INT,
                 local_b, local_n, MPI_INT,
                 0, MPI_COMM_WORLD);

    long local_sum = 0;
    for (int i = 0; i < local_n; i++) {
        local_sum += (long)local_a[i] * (long)local_b[i];
    }

    if (rank == 0) {
        long total_sum = local_sum;

        for (int src = 1; src < size; src++) {
            long part = 0;
            MPI_Recv(&part, 1, MPI_LONG, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_sum += part;
        }

        double t1 = MPI_Wtime();

        printf("Processes = %d\n", size);
        printf("N         = %d\n", N);
        printf("Res       = %ld\n", total_sum);
        printf("Time      = %.6fs\n", t1 - t0);
    } else {
        MPI_Send(&local_sum, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD);
    }

    free(local_a);
    free(local_b);
    free(scounts);
    free(displs);

    if (rank == 0) {
        free(a);
        free(b);
    }

    MPI_Finalize();
    return 0;
}