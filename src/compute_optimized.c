#include <omp.h>
#include <x86intrin.h>
#include <immintrin.h>

#include "compute.h"

// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
 // output_matrix
  int32_t *b_data = b_matrix->data;
  int32_t *a_data = a_matrix->data;
  int b_rows = b_matrix->rows;
  int b_cols = b_matrix->cols;
  int a_rows = a_matrix->rows;
  int a_cols = a_matrix->cols;

  uint32_t total_items = (b_rows) * (b_cols);
  int32_t *b_reversed = malloc(sizeof(int32_t) * total_items);

  //now we malloc space for the output_matrix
  uint32_t output_rows = (a_rows) - (b_rows) + 1;
  uint32_t output_cols = (a_cols) - (b_cols) + 1;
  int32_t *output = (int32_t*) malloc((output_rows * output_cols) * sizeof(int32_t));
  matrix_t *omatrix = (matrix_t *) malloc(sizeof(matrix_t));

  //set *output_matrix to be a point to resulting matrix where matrix is allocated
  *output_matrix = omatrix;
  omatrix->rows = output_rows;
  omatrix->cols = output_cols;
  omatrix->data = output;
    //location in a is the counter

  if (output == NULL || output_matrix == NULL) {
      return -1;
  }

  //flip b matrix matrix flip should be right
  #pragma omp parallel for
  for(int i = 0; i < total_items; i++) {
      *(b_reversed + i) = *(b_data + total_items - i - 1);
  }


  b_matrix->data = b_reversed;
  free(b_data);
  b_data = b_matrix->data;

  int output_index = 0;
  int32_t sum = 0;
  #pragma omp parallel for 
  for (int row = 0; row < output_rows; row++) {
      for (int col = 0; col < output_cols; col++) {
          sum = 0;
          for (int i = 0; i < b_rows; i++) {
              __m256i sum_groups = _mm256_setzero_si256();

            for(int j = 0; j < b_cols/8 * 8; j+=8) {
                __m256i a_vector = _mm256_loadu_si256((__m256i *)(a_data + (i * a_cols) + (row * a_cols) + (j + col)));
                __m256i b_vector = _mm256_loadu_si256((__m256i*)(b_data + (i * b_cols) + j));
                sum_groups = _mm256_add_epi32(sum_groups, _mm256_mullo_epi32(a_vector, b_vector));
          }
            //tail case
            int32_t temp_arr[8];
            _mm256_storeu_si256((__m256i*)temp_arr, sum_groups);
            sum += temp_arr[0] + temp_arr[1] + temp_arr[2] + temp_arr[3] + temp_arr[4] + temp_arr[5] + temp_arr[6] + temp_arr[7];
            //#pragma omp parallel for
            for (int c = b_cols/8 * 8; c < b_cols; c++) {
                sum += b_data[i * b_cols + c] * a_data[(i * a_cols) + (row * a_cols) + c + col];
            }

      }
          output[output_index] = sum;
          output_index++;
  }
  }

  return 0;
}


// Executes a task
int execute_task(task_t *task) {
  matrix_t *a_matrix, *b_matrix, *output_matrix;

  char *a_matrix_path = get_a_matrix_path(task);
  if (read_matrix(a_matrix_path, &a_matrix)) {
    printf("Error reading matrix from %s\n", a_matrix_path);
    return -1;
  }
  free(a_matrix_path);

  char *b_matrix_path = get_b_matrix_path(task);
  if (read_matrix(b_matrix_path, &b_matrix)) {
    printf("Error reading matrix from %s\n", b_matrix_path);
    return -1;
  }
  free(b_matrix_path);

  if (convolve(a_matrix, b_matrix, &output_matrix)) {
    printf("convolve returned a non-zero integer\n");
    return -1;
  }

  char *output_matrix_path = get_output_matrix_path(task);
  if (write_matrix(output_matrix_path, output_matrix)) {
    printf("Error writing matrix to %s\n", output_matrix_path);
    return -1;
  }
  free(output_matrix_path);

  free(a_matrix->data);
  free(b_matrix->data);
  free(output_matrix->data);
  free(a_matrix);
  free(b_matrix);
  free(output_matrix);
  return 0;
}

