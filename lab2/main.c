#include <stdio.h>
#include <omp.h>

#define MATRIX_SIZE 14976
#define NUM_THREADS_SIZE 11

int matrix[MATRIX_SIZE][MATRIX_SIZE];
int thread_nums[NUM_THREADS_SIZE] = {1, 2, 3, 4, 5, 6, 8, 12, 16, 20, 32};

void fill_matrix_by_order(int num_threads) {
  int i, j, k;

  omp_set_num_threads(num_threads);

  #pragma omp parallel private(i, j, k)
  {
    int thread_num = omp_get_thread_num();
    int rows_per_thread = MATRIX_SIZE / num_threads;
    int start_row = thread_num * rows_per_thread;
    k = start_row * MATRIX_SIZE;
    
    for (i = start_row; i < start_row + rows_per_thread; i++)
      for (j = 0; j < MATRIX_SIZE; j++)
        matrix[i][j] = k++;
  }
}

int test_ordered_matrix() {
  int i, j, k = 0;

  for (i = 0; i < MATRIX_SIZE; i++)
    for (j = 0; j < MATRIX_SIZE; j++)
      if (matrix[i][j] != k++)
        return 1;
  
  return 0;
}

void fill_matrix_with_zeroes(int num_threads) {
  int i, j;

  omp_set_num_threads(num_threads);

  #pragma omp parallel private(i, j)
  {
    int thread_num = omp_get_thread_num();
    int rows_per_thread = MATRIX_SIZE / num_threads;
    int start_row = thread_num * rows_per_thread;

    for (i = start_row; i < start_row + rows_per_thread; i++)
      for (j = 0; j < MATRIX_SIZE; j++)
        matrix[i][j] = 0;
  }
}

int test_zeroes_matrix() {
  int i, j;

  for (i = 0; i < MATRIX_SIZE; i++)
    for (j = 0; j < MATRIX_SIZE; j++)
      if (matrix[i][j] != 0)
        return 1;

  return 0;
}

void output_function_runtime_for_all_thread_nums(void (*function)(int), char* function_name, int (*test_function)()) {
  int i;
  for (i = 0; i < NUM_THREADS_SIZE; i++) {
    double start = omp_get_wtime();
    
    function(thread_nums[i]);

    double end = omp_get_wtime();
    double time = end - start;

    int test_result = test_function();

    printf("FUNCTION:\t%s\n", function_name);
    printf("THREADS:\t%d\n", thread_nums[i]);
    printf("RUNTIME:\t%.2lfs\n", time);

    if (test_result != 0) {
      printf("TEST:\t\t\033[31mFAILED\033[0m\n\n");
    } else {
      printf("TEST:\t\t\033[32mPASSED\033[0m\n\n");
    }
  }
}

int main() {
  #ifdef _OPENMP
    printf("OpenMP is loaded v.%d\n\n", _OPENMP);
  #else
    printf("OpenMP not found.\n");
    return 1;
  #endif

  output_function_runtime_for_all_thread_nums(fill_matrix_with_zeroes, "fill_matrix_with_zeroes", test_zeroes_matrix);
  output_function_runtime_for_all_thread_nums(fill_matrix_by_order, "fill_matrix_by_order", test_ordered_matrix);

  return 0;
}