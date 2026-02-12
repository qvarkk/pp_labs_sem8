#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define N 15000

int matrix[N][N];

void fill_zero_matrix_by_rows() {
  int i, j;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      matrix[i][j] = 0; 
}

void fill_zero_matrix_by_columns() {
  int i, j;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      matrix[j][i] = 0; 
}

void fill_ordered_matrix_by_rows() {
  int i, j, k = 1;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      matrix[i][j] = k++; 
}

void fill_ordered_matrix_by_columns() {
  int i, j, k = 1;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      matrix[j][i] = k++; 
}

void fill_rand_matrix_by_rows() {
  int i, j;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      matrix[i][j] = rand(); 
}

void fill_rand_matrix_by_columns() {
  int i, j;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      matrix[j][i] = rand(); 
}

void output_function_runtime(void (*function)(), char* function_name) {
  clock_t begin = clock();

  function();

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("Execution time of %s: %.2lfs\n", function_name, time_spent);
}

int main() {
  srand(time(NULL));

  printf("Matrix size: %d\n", N);

  output_function_runtime(fill_zero_matrix_by_rows, "initial fill_zero_matrix_by_rows");

  output_function_runtime(fill_zero_matrix_by_rows, "fill_zero_matrix_by_rows");
  output_function_runtime(fill_ordered_matrix_by_rows, "fill_ordered_matrix_by_rows");
  output_function_runtime(fill_rand_matrix_by_rows, "fill_rand_matrix_by_rows");

  output_function_runtime(fill_zero_matrix_by_columns, "fill_zero_matrix_by_columns");
  output_function_runtime(fill_ordered_matrix_by_columns, "fill_ordered_matrix_by_columns");
  output_function_runtime(fill_rand_matrix_by_columns, "fill_rand_matrix_by_columns");

  return 0;
}