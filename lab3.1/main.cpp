//------------------------------------------------------------
// Программа решения уравнений Пуассона методом Гаусса-Зейделя
//------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <omp.h>
#include "locale.h"

using namespace std;

// Функции решения уравнения (результаты всех версий должны быть идентичны!)
int Calc_ser(double** u, double** f, int N, double eps, double& time);  // последовательная
int Calc_par_block(double** u, double** f, int N, double eps, double& time); // параллельная блочная
int Calc_par_block1(double** u, double** f, int N, double eps, double& time); // параллельная блочная

// Инициализация массивов
void Init(double **u, double **f, int N);
double** new_arr(int N);
void delete_arr(double** arr, int N);

// Вывод части массива для контроля
void Output(double** u, int N);

bool CompareResults(double** u1, double** u2, int N, double tolerance = 1e-10);

int main(int argc, char **argv)
{
	double **f=NULL, **u_blk_par=NULL, **u_blk=NULL, **u_ser=NULL;
	
	const int N = 600;        // Количество точек сетки по каждой размерности
	const double eps = 0.0001;   // Точность вычислений
	int icnt;                  // Количество итераций
	double stime = -1;         // Время решения
	
	f = new_arr(N);      // Выделение памяти под правую часть значений уравнения
	u_blk_par = new_arr(N + 2);
	u_blk = new_arr(N + 2);
	u_ser = new_arr(N + 2);
	
	//	 Последовательная реализация
	cout << "\n\t*** Serial version ***\n";
	Init(u_ser, f, N);                  // Инициализация краевых условий и правой части уравнения
	icnt = Calc_ser(u_ser, f, N, eps, stime);  // Вызов функции расчета по методу Гаусса-Зейделя
	cout << "Solution time = " << stime << endl;
	cout << "Iterations =    " << icnt << endl;
	cout << "Results:\n";
	Output(u_ser, N);                   // Вывод результатов на экран

	// Параллельная блочная реализация
	cout << "\n\t*** Parallel block version 1 ***\n";
	Init(u_blk, f, N);                  // Инициализация краевых условий и правой части
	icnt = Calc_par_block1(u_blk, f, N, eps, stime);  // Вызов параллельной блочной функции расчета
	cout << "Solution time = " << stime << " seconds" << endl;
	cout << "Iterations =    " << icnt << endl;
	cout << "Results:\n";
	Output(u_blk, N);
    
	if (CompareResults(u_ser, u_blk, N))
			cout << "Parallel block 1 matches serial!\n";
	else
			cout << "Parallel block 1 DOES NOT match serial!\n";

		// Параллельная блочная реализация
	cout << "\n\t*** Parallel block version ***\n";
	Init(u_blk_par, f, N);                  // Инициализация краевых условий и правой части
	icnt = Calc_par_block(u_blk_par, f, N, eps, stime);  // Вызов параллельной блочной функции расчета
	cout << "Solution time = " << stime << " seconds" << endl;
	cout << "Iterations =    " << icnt << endl;
	cout << "Results:\n";
	Output(u_blk_par, N);
    
	if (CompareResults(u_ser, u_blk_par, N))
			cout << "Parallel block matches serial!\n";
	else
			cout << "Parallel block DOES NOT match serial!\n";

  // Освобождение памяти массивов
  delete_arr(f, N);
  delete_arr(u_blk_par, N + 2);
  delete_arr(u_blk, N + 2);
	delete_arr(u_ser, N + 2);
  
	return 0;
}

// Последовательная функция, реализующая алгоритм Гаусса-Зейделя
// Входные параметры: массив неизвестных и краевых значений, массив правых частей, количество точек сетки по каждому направлению, точность вычислений
int Calc_ser(double** u, double** f, int N, double eps, double& time)
{
	double max;                // Максимальная ошибка на итерации
	double h = 1.0 / (N + 1);  // Величина шага
	int icnt = 0;              // Количество итераций

	double start_time = omp_get_wtime();

	do
	{
		icnt++;
		max = 0;
		for (int i = 1; i <= N; i++)
		{
			for (int j = 1; j <= N; j++)
			{
				double u0 = u[i][j];
				u[i][j] = 0.25 * (u[i - 1][j] + u[i + 1][j] + u[i][j - 1] + u[i][j + 1] - h * h * f[i - 1][j - 1]);
				double d = fabs(u[i][j] - u0);
				if (d > max)
				{
					max = d;
				}
			}
		}
	}
  while (max > eps);

	time = omp_get_wtime() - start_time;
	return icnt;
}

// Параллельная реализия блочного алгоритма Гаусса-Зейделя
int Calc_par_block1(double** u, double** f, int N, double eps, double& time)
{
	double max;
	double h = 1.0 / (N + 1);
	int icnt = 0;

	const int BlockSize = 20;  // Размер блока
	int bcnt;                  // Количество блоков в ряд

	double start_time = omp_get_wtime();

	if (N % BlockSize == 0) // Если количество точек по каждому из направлений сетки делится нацело на размер блока, то проводятся вычисления
	{
		bcnt = N / BlockSize;
		// а сколько блоков в волне?
		do
		{
			icnt++;
			max = 0;

			// здесь реализация
			for (int wave = 0; wave < 2 * bcnt - 1; wave++) 
			{
				int start_i = (wave < bcnt) ? 0 : wave - bcnt + 1;
				int end_i = (wave < bcnt) ? wave : bcnt - 1;
			
				#pragma omp parallel for reduction(max:max) schedule(dynamic)
				for (int ib = start_i; ib <= end_i; ib++)
				{
					int jb = wave - ib;
					
					for (int i = ib * BlockSize + 1; i <= (ib + 1) * BlockSize; i++)
					{
						for (int j = jb * BlockSize + 1; j <= (jb + 1) * BlockSize; j++)
						{
							double u0 = u[i][j];
							u[i][j] = 0.25 * (u[i - 1][j] + u[i + 1][j] + u[i][j - 1] + u[i][j + 1] - h * h * f[i - 1][j - 1]);
							double d = fabs(u[i][j] - u0);
							if (d > max)
								max = d;
						}
					}
				}
			}
		}
    while (max > eps);
	}
	else
	{
		cout << "Error!!! N must be divisible by BlockSize" << endl;
		time = 0;
		return 0;
	}
	
	time = omp_get_wtime() - start_time;
	return icnt;
}

// Параллельная реализия блочного алгоритма Гаусса-Зейделя
int Calc_par_block(double** u, double** f, int N, double eps, double& time)
{
	double max;
	double h = 1.0 / (N + 1);
	int icnt = 0;

	const int BlockSize = 20;  // Размер блока
	int bcnt;                  // Количество блоков в ряд

	double start_time = omp_get_wtime();

	if (N % BlockSize == 0) // Если количество точек по каждому из направлений сетки делится нацело на размер блока, то проводятся вычисления
	{
		bcnt = N / BlockSize;
		// а сколько блоков в волне?

		do
		{
			icnt++;
			max = 0;

			#pragma omp parallel shared(u, f, max, N, bcnt, h)
			{
				#pragma omp single
				{
					for (int wave = 0; wave < 2 * bcnt - 1; wave++) 
					{
						int start_i = (wave < bcnt) ? 0 : wave - bcnt + 1;
						int end_i = (wave < bcnt) ? wave : bcnt - 1;

						for (int ib = start_i; ib <= end_i; ib++)
						{
							int jb = wave - ib;

							#pragma omp task firstprivate(ib, jb) shared(u, f, max, h)
							{
								double thread_max = 0;

								for (int i = ib * BlockSize + 1; i <= (ib + 1) * BlockSize; i++)
								{
									for (int j = jb * BlockSize + 1; j <= (jb + 1) * BlockSize; j++)
									{
										double u0 = u[i][j];
										u[i][j] = 0.25 * (u[i - 1][j] + u[i + 1][j] + u[i][j - 1] + u[i][j + 1] - h * h * f[i - 1][j - 1]);
										double d = fabs(u[i][j] - u0);
										
										if (d > thread_max)
											thread_max = d;
									}
								}

								#pragma omp critical
								{
									if (thread_max > max)
										max = thread_max;
								}
							}
						}

						#pragma omp taskwait
					}
				}
			}
		}
    while (max > eps);
	}
	else
	{
		cout << "Error!!! N must be divisible by BlockSize" << endl;
		time = 0;
		return 0;
	}
	
	time = omp_get_wtime() - start_time;
	return icnt;
}

// Функция выделения памяти под 2D массив
double** new_arr(int N)
{
	double** f = new double* [N];
	for (int i = 0; i < N; i++)
	{
		f[i] = new double [N];
	}
	return f;
}

// Функция освобождения памяти 2D массива
void delete_arr(double** arr, int N)
{
	for (int i = 0; i < N; i++)
	{
		delete[] arr[i];
	}
	delete[] arr;
}

// Задание граничных значений
double G(double x, double y)
{
	if (x == 0) return 1 - 2 * y;
	if (x == 1) return -1 + 2 * y;
	if (y == 0) return 1 - 2 * x;
	if (y == 1) return -1 + 2 * x;
	cout << "Error in G" << endl;
	return 0;
}

// Задание правой части
double F(double x, double y)
{
	return 2.2;
}

// Инициализация массивов правой части и краевых условий
void Init(double **u, double **f, int N)
{
	double h = 1.0 / (N + 1);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			f[i][j] = F((i + 1) * h, (j + 1) * h);
	}
	for (int i = 1; i < N + 1; i++)
	{
		for (int j = 1; j < N + 1; j++)
			u[i][j] = 0.2;
		u[i][0] = G(i * h, 0);
		u[i][N + 1] = G(i * h, (N + 1) * h);
	}
	for (int j = 0; j < N + 2; j++)
	{
		u[0][j] = G(0, j * h);
		u[N + 1][j] = G((N + 1) * h, j * h);
	}
}

// Функция вывода прореженной матрицы решения
void Output(double** u, int N)
{
  const int K = 5;
	cout << fixed << setprecision(8);
	for (int i = 0; i <= K; i++)
	{
		for (int j = 0; j <= K; j++)
			cout << setw(12) << u[i * (N + 1) / K][j * (N + 1) / K];
		cout << endl;
	}
	cout << endl;
}

bool CompareResults(double** u1, double** u2, int N, double tolerance = 0.001)
{
	double max_diff = 0;
	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			double diff = fabs(u1[i][j] - u2[i][j]);
			if (diff > max_diff)
				max_diff = diff;
		}
	}
	cout << "max difference: " << max_diff << endl;
	return max_diff < tolerance;
}
