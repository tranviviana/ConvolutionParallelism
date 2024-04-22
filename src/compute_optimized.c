#include <omp.h>
#include <x86intrin.h>
#include <immintrin.h>

#include "compute.h"
int32_t blockwise(uint32_t start_location, matrix_t *b_matrix, matrix_t *a_matrix, int b_cols, int a_cols, int total_items);

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
  for (int row = 0; row < a_rows - b_rows; row++) {
    for (int col = 0; col < a_cols - b_cols; col++) {
        int32_t sum = 0;
        for (int i = 0; i < b_rows; i++) {
            for(int j = 0; j < b_cols; j++) {
                sum += *(b_data + (i * b_cols) + j) * (*(a_data + (i * a_cols) + (row * a_cols) + j + col));
            }
        }
        output[output_index] = sum;
        output_index ++;
    }
}
   /* uint32_t counter = 0;
    //counter is location within a
    uint32_t output_counter = 0;
    //number of calls within one row is the shifter
    int shifter = 1;
    int row_counter = 1;
    while (output_counter < (output_cols * output_rows)) {
        //figuring out counter lol
        output[output_counter] = blockwise(counter, b_matrix, a_matrix, b_cols, a_cols, total_items);  
        output_counter++;
        if(shifter == output_cols) {
            counter = row_counter * (a_cols);
            shifter = 1;
            row_counter++;
       } else {
        counter++;
        shifter++;
       }
        
    }*/
  return 0;
}

/*int32_t blockwise(uint32_t start_location, matrix_t *b_matrix, matrix_t *a_matrix, int b_cols, int a_cols, int total_items) {
    //start location must be such that the entire block is in the block
    //pointers to array's data
    int32_t *b_array = b_matrix->data;
    int32_t *a_array = a_matrix->data;


    int counter = 0;
    //counter in this case represents pairs being multiplied
    int col_counter = 1;
    int32_t sum = 0;
    int moving_location = start_location;
    //moving location represents the location within a
    int row_counter = 1;

    //parsing through a array until we have multiplied all elements
    if (a_cols > b_cols) {
        while (counter < total_items) {
            sum += b_array[counter] * a_array[moving_location];

           if (col_counter - b_cols == 0) {
                //move down one row
                moving_location = start_location + (a_cols * row_counter) - 1; //-1 for accounting for double counting lol
                col_counter = 0;
                row_counter++; 
            }
            counter++;
            moving_location++;
            col_counter++;
        
        }
    } else {
        while (counter < total_items) {
            sum += b_array[counter] * a_array[start_location];
            counter++;
            start_location++;
        }
    }
    return sum;
}*/

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

