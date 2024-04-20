#include <omp.h>
#include <x86intrin.h>
#include <immintrin.h>

#include "compute.h"
int32_t blockwise(uint32_t start_location, matrix_t *b_matrix, matrix_t *a_matrix, int total_items);
//void flip(matrix_t *b_matrix, uint32_t total_items);
// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
  // flipping matrix b 
  uint32_t total_items = (b_matrix->rows) * (b_matrix->cols);
  /*uint32_t a_total = (a_matrix->rows) * (a_matrix->cols);*/

//flip b matrix
  int32_t *b_array = b_matrix->data;
  __m256i reverse_order = _mm256_set_epi32(0,1,2,3,4,5,6,7);
#pragma omp parallel for
  for(int i = 0; i < (total_items/2)/8 * 8; i += 8){
      //load front and back 8 from b matrix
      __m256i front = _mm256_loadu_si256((__m256i*)&b_array[i]);
      __m256i back = _mm256_loadu_si256((__m256i*)&b_array[total_items - i - 8]);
      //flip front and back 8 of b matrix
      front =  _mm256_permutevar8x32_epi32(front, reverse_order);
      back =  _mm256_permutevar8x32_epi32(back, reverse_order);

    //store back into the b_array by flipping spots
      _mm256_storeu_si256((__m256i*)&b_array[total_items - i - 8], front);
      _mm256_storeu_si256((__m256i*)&b_array[i], back);

  }
  //flip tail case for elements after highest factor of 8
#pragma omp parallel for
  for (int i = (total_items/2)/8 * 8; i < total_items / 2; i++) {
        // Swap the remaining elements using scalar operations
        int temp = b_array[i];
        b_array[i] = b_array[total_items - i - 1];
        b_array[total_items - i - 1] = temp;
    }

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
    //counter is location within a
    uint32_t output_counter = 0;
    //number of calls within one row is the shifter
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

int32_t blockwise(uint32_t start_location, matrix_t *b_matrix, matrix_t *a_matrix, int total_items) {
    //start location must be such that the entire block is in the block
    //pointers to array's data
    int32_t *b_array = b_matrix->data;
    int32_t *a_array = a_matrix->data;
    int b_cols = b_matrix->cols;
    int a_cols = a_matrix->cols;


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
        __m256i sumVec = _mm256_setzero_si256();
        for(int vector_offset = 0; vector_offset < total_items / 8 * 8; vector_offset += 8)
          {
              __m256i b_temp = _mm256_loadu_si256((__m256i*)(b_array + vector_offset));
              __m256i a_temp = _mm256_loadu_si256((__m256i*)(a_array + vector_offset));

              __m256i product = _mm256_mullo_epi32(b_temp, a_temp);
              sumVec = _mm256_add_epi32(sumVec, product);
          }
          for (int i = total_items / 8 * 8; i < total_items; i ++) {
              sum += b_array[i] * a_array[i];
          }
          int temp_arr[8];
          _mm256_storeu_si256((__m256i*)temp_arr, sumVec);
          sum += temp_arr[0] + temp_arr[1] + temp_arr[2] + temp_arr[3] + temp_arr[4] + temp_arr[5]+ temp_arr[6]+ temp_arr[7];
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

