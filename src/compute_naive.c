#include "compute.h"
int blockwise(uint32_t start_location, matrix_t *b_matrix, matrix_t *a_matrix, int total_items);
// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
  // flipping matrix b 
  uint32_t total_items = (b_matrix->rows) * (b_matrix->cols);

  int32_t temp;
  for(uint32_t i = 0; i < total_items / 2; i++) {
      temp = (b_matrix->data)[i];
      (b_matrix->data)[i] = (b_matrix->data)[total_items - i - 1];
      (b_matrix-> data)[total_items - i - 1] = temp;
  }
  //now we malloc space for the output_matrix
  uint32_t output_rows = (a_matrix->rows) / (b_matrix -> rows);
  uint32_t output_cols = (a_matrix->cols) / (b_matrix -> cols);
  int32_t *output = (int32_t*) malloc((output_rows + output_cols) * sizeof(int32_t));
  matrix_t *omatrix = (matrix_t *) malloc(sizeof(matrix_t));
  
  if (output == NULL || output_matrix == NULL) {
      return -1;
  }
  //set *output_matrix to be a point to resulting matrix where matrix is allocated
  *output_matrix = omatrix;
  omatrix->rows = output_rows;
  omatrix->cols = output_cols;
  omatrix->data = output;
  
  //now we element wise multiple
  uint32_t counter = 0; //starting location for the a matrix
  uint32_t output_counter = 0; //location for output matrix
  while ((counter + (b_matrix -> cols) < (a_matrix->cols)) && (counter + (b_matrix -> rows) < (a_matrix->rows))) {
    //check that the block is within the constraints of the right corner
  while (((counter - 1 + (b_matrix -> cols)) % a_matrix->cols) != 0 ) {
      //check that the block is within the constraints of the right side
    uint32_t sum = blockwise(counter, b_matrix, a_matrix, total_items); 
    output[output_counter] = sum;
    output_counter += 1;
    counter += 1;
  }
  counter += (b_matrix-> cols);
  }
  
  return 0;
}
int blockwise(uint32_t start_location, matrix_t *b_matrix, matrix_t *a_matrix, int total_items) {
    //pointers to array's data
    int32_t *b_array = b_matrix->data;
    int32_t *a_array = a_matrix->data;
    int b_cols = b_matrix->cols;
    int counter = 0;
    int col_counter = 0;
    uint32_t sum = 0;
    //parsing through a array until we have multiplied all elements
    while (counter < total_items) {
        if (col_counter % b_cols) {
            //move down one row
            start_location = start_location + b_cols;
            col_counter = 0;
        }
        sum += b_array[counter] * a_array[start_location];
        counter++;
        start_location++;
        col_counter++;
    }
    return sum;

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
