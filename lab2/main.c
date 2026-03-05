#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>

#define MATRIX_SIZE 20000
#define THREAD_NUMS_SIZE 11

int matrix[MATRIX_SIZE][MATRIX_SIZE];
int thread_nums[THREAD_NUMS_SIZE] = {1, 2, 3, 4, 5, 6, 8, 12, 16, 20, 32};

double manual_order_results[THREAD_NUMS_SIZE];
double manual_zero_results[THREAD_NUMS_SIZE];
double outer_rand_results[THREAD_NUMS_SIZE];
double inner_rand_results[THREAD_NUMS_SIZE];
double collapse_rand_results[THREAD_NUMS_SIZE];

void parallel_fill_matrix_by_order(int thread_nums) {
  int i, j;

  omp_set_num_threads(thread_nums);

#pragma omp parallel shared(thread_nums, matrix) private(i, j)
  {
    int thread_num = omp_get_thread_num();
    int rows_per_thread = MATRIX_SIZE / thread_nums;
    int start_row = thread_num * rows_per_thread;
    
    for (i = start_row; i < start_row + rows_per_thread; i++) {
      for (j = 0; j < MATRIX_SIZE; j++) {
        matrix[i][j] = i * MATRIX_SIZE + j;
      }
    }
  }
}

void parallel_fill_matrix_zeroes(int thread_nums) {
  int i, j;

  omp_set_num_threads(thread_nums);

#pragma omp parallel shared(thread_nums, matrix) private(i, j)
  {
    int thread_num = omp_get_thread_num();
    int rows_per_thread = MATRIX_SIZE / thread_nums;
    int start_row = thread_num * rows_per_thread;

    for (i = start_row; i < start_row + rows_per_thread; i++) {
      for (j = 0; j < MATRIX_SIZE; j++) {
        matrix[i][j] = 0;
      }
    }
  }
}

void parallel_outer_fill_matrix_rand(int thread_nums) {
  omp_set_num_threads(thread_nums);

#pragma omp parallel
  {
    int i, j;
    unsigned int seed = (unsigned int)time(NULL) ^ omp_get_thread_num();

#pragma omp for schedule(dynamic, 100)  
    for (i = 0; i < MATRIX_SIZE; i++) {

      for (j = 0; j < MATRIX_SIZE; j++) {
        matrix[i][j] = rand_r(&seed);
      }
    }
  }
}

void parallel_inner_fill_matrix_rand(int thread_nums) {
  omp_set_num_threads(thread_nums);

#pragma omp parallel 
  {
    int i, j;
    unsigned int seed = (unsigned int)time(NULL) ^ omp_get_thread_num();

    for (i = 0; i < MATRIX_SIZE; i++) {

#pragma omp for schedule(dynamic, 100)
      for (j = 0; j < MATRIX_SIZE; j++) {

        matrix[i][j] = rand_r(&seed);
      }
    }
  }
}

void parallel_collapse_fill_matrix_rand(int thread_nums) {
  omp_set_num_threads(thread_nums);

#pragma omp parallel
  {
    int i, j;
    unsigned int seed = (unsigned int)time(NULL) ^ omp_get_thread_num();

#pragma omp for collapse(2) schedule(dynamic, 100)
    for (i = 0; i < MATRIX_SIZE; i++) {
      for (j = 0; j < MATRIX_SIZE; j++) {
        matrix[i][j] = rand_r(&seed);
      }
    }
  }
}

int test_ordered_matrix() {
  int i, j, k = 0;

  for (i = 0; i < MATRIX_SIZE; i++) {
    for (j = 0; j < MATRIX_SIZE; j++) {
      if (matrix[i][j] != k++) {
        return 1;
      }
    }
  }
  
  return 0;
}

int test_zeroes_matrix() {
  int i, j;

  for (i = 0; i < MATRIX_SIZE; i++) {
    for (j = 0; j < MATRIX_SIZE; j++) {
      if (matrix[i][j] != 0) {
        return 1;
      }
    }
  }

  return 0;
}

void output_function_runtime_for_all_thread_nums(void (*function)(int), char* message, double result_array[], int (*test_function)()) {
  int i;
  for (i = 0; i < THREAD_NUMS_SIZE; i++) {
    double start = omp_get_wtime();
    
    function(thread_nums[i]);

    double end = omp_get_wtime();
    double time = end - start;
    result_array[i] = time;

    int test_result = -1;
    if (test_function != NULL) {
      test_result = test_function();
    }

    printf("FUNCTION:\t%s\n", message);
    printf("THREADS:\t%d\n", thread_nums[i]);
    printf("RUNTIME:\t%.2lfs\n", time);

    if (test_result == 1) {
      printf("TEST:\t\t\033[31mFAILED\033[0m\n\n");
    } else if (test_result == 0) {
      printf("TEST:\t\t\033[32mPASSED\033[0m\n\n");
    } else {
      printf("TEST:\t\tNONE\n\n");
    }
  }
}

void output_results(char* message, double result_array[]) {
  printf("\n==== %s =====\n", message);
  printf("| %7s | %4s |\n", "THREADS", "TIME");
  for (int i = 0; i < THREAD_NUMS_SIZE; i++) {
    printf("| %7i | %4.2lf |\n", thread_nums[i], result_array[i]);
  }
}

int main() {
  #ifdef _OPENMP
    printf("OpenMP is loaded v.%d\n\n", _OPENMP);
  #else
    printf("OpenMP not found.\n");
    return 1;
  #endif

  srand(time(NULL));

  parallel_fill_matrix_zeroes(1);

  output_function_runtime_for_all_thread_nums(parallel_fill_matrix_zeroes, "[MANUAL] Zeros", manual_zero_results, test_zeroes_matrix);
  output_function_runtime_for_all_thread_nums(parallel_fill_matrix_by_order, "[MANUAL] Order", manual_order_results, test_ordered_matrix);
  output_function_runtime_for_all_thread_nums(parallel_outer_fill_matrix_rand, "[OUTER] Rand", outer_rand_results, NULL);
  output_function_runtime_for_all_thread_nums(parallel_inner_fill_matrix_rand, "[INNER] Rand", inner_rand_results, NULL);
  output_function_runtime_for_all_thread_nums(parallel_collapse_fill_matrix_rand, "[COLLAPSE] Rand", collapse_rand_results, NULL);

  output_results("ZEROS", manual_zero_results);
  output_results("ORDER", manual_order_results);
  output_results("OUTER", outer_rand_results);
  output_results("INNER", inner_rand_results);
  output_results("COLLAPSE", collapse_rand_results);

  return 0;
}