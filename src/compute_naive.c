#include "compute.h"
int32_t blockwise(uint32_t start_location, matrix_t *b_matrix, matrix_t *a_matrix, int total_items);
void flip(matrix_t *b_matrix, uint32_t total_items);
// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
  // flipping matrix b 
  uint32_t total_items = (b_matrix->rows) * (b_matrix->cols);
  /*uint32_t a_total = (a_matrix->rows) * (a_matrix->cols);*/

  //flip b matrix matrix flip should be right
  flip(b_matrix, total_items);

  //now we malloc space for the output_matrix
  uint32_t output_rows = (a_matrix->rows) - (b_matrix -> rows) + 1;
  uint32_t output_cols = (a_matrix->cols) - (b_matrix -> cols) + 1;
  int32_t *output = (int32_t*) malloc((output_rows * output_cols) * sizeof(int32_t));
  matrix_t *omatrix = (matrix_t *) malloc(sizeof(matrix_t));
  
  if (output == NULL || output_matrix == NULL) {
      return -1;
  }
 
  
  //set *output_matrix to be a point to resulting matrix where matrix is allocated
  *output_matrix = omatrix;
  omatrix->rows = output_rows;
  omatrix->cols = output_cols;
  omatrix->data = output;
    //location in a is the counter

    uint32_t counter = 0;
    uint32_t output_counter = 0;
    int shifter = 1;
    int row_counter = 1;
    while (output_counter < (output_cols * output_rows)) {
        //figuring out counter lol
        output[output_counter] = blockwise(counter, b_matrix, a_matrix, total_items);  
        output_counter++;
        if(shifter == output_cols) {
            counter = row_counter * (a_matrix->cols);
            shifter = 1;
            row_counter++;
       } else {
        counter++;
        shifter++;
       }
        
    }
    

    
  return 0;
}
void flip(matrix_t *b_matrix, uint32_t total_items) {
  for(uint32_t i = 0; i < total_items / 2; i++) {
      int32_t temp = (b_matrix->data)[i];
      (b_matrix->data)[i] = (b_matrix->data)[total_items - i - 1];
      (b_matrix-> data)[total_items - i - 1] = temp;
  }
}
int32_t blockwise(uint32_t start_location, matrix_t *b_matrix, matrix_t *a_matrix, int total_items) {
    //start location must be such that the entire block is in the block
    //pointers to array's data
    int32_t *b_array = b_matrix->data;
    int32_t *a_array = a_matrix->data;
    int b_cols = b_matrix->cols;
    int a_cols = a_matrix->cols;


    int counter = 0;
    int col_counter = 1;
    int32_t sum = 0;
    int moving_location = start_location;
    int row_counter = 1;

    //parsing through a array until we have multiplied all elements
    if (a_cols > b_cols) {
        while (counter < total_items) {
            sum += b_array[counter] * a_array[moving_location];
           /* printf("%d", b_array[counter]);
            printf("%d", a_array[moving_location]);*/

           if (col_counter - b_cols == 0) {
                //move down one row
                moving_location = start_location + (a_cols * row_counter) - 1; //-1 for accounting for double counting lol
                col_counter = 0;
                row_counter++; 
            }
            counter++;
            moving_location++;
            col_counter++;
        //    printf('%d',b_array[counter]);
        
        }
    } else {
        while (counter < total_items) {
            printf("this is mistake");
            sum += b_array[counter] * a_array[start_location];
            counter++;
            start_location++;
        }
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
