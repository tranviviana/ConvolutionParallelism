#include "compute.h"

// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
  // flipping matrix b 
  uint32_t total_items = (b_matrix->rows) * (b_matrix->cols);

  int32_t temp;
  for(uint32_t i = 0; i < total_items / 2; i++) {
      temp = b_matrix[i];
      b_matrix[i] = b_matrix[total_items - i - 1];
      b_matrix[total_items - i - 1] = temp;
  }
  //now we malloc space for the output_matrix
  uint32_t output_rows = (a_matrix->rows) / (b_matrix -> rows);
  uint32_t output_cols = (a_matrix->cols) / (b_matrix -> cols);
  int32_t *output = (int32_t*) malloc (output_rows + output_cols) * sizeof(int32_t);
  uint32_t output_numitems = output_rows * out_cols;
  
  //now we element wise multiple
  uint32_t counter = 0;
  while (((counter + b_matrix -> rows) < output_numitems) && (counter + b_matrix -> cols) < output_numitems) {
  uint32_t row_counter = 0;
  uint32_t col_counter = 0;
  while 




  col_counter = 0;
  row_counter ++;
  }
  
      
  
  
    




  return -1;
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
