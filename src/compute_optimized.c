#include <omp.h>
#include <x86intrin.h>

#include "compute.h"
int32_t blockwise(uint32_t start_location, matrix_t *b_matrix, matrix_t *a_matrix, int total_items);
// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
// flipping matrix b 
    uint32_t total_items = (b_matrix->rows) * (b_matrix->cols);
    /*uint32_t a_total = (a_matrix->rows) * (a_matrix->cols);*/
  
  //unrolling the flip by 4
      #pragma omp parallel for
      for(uint32_t i = 0; i < total_items / 2 /4 * 4; i+=4) {
        int32_t temp1 = (b_matrix->data)[i];
        int32_t temp2 = (b_matrix->data)[i + 1];
        int32_t temp3 = (b_matrix->data)[i + 2];
        int32_t temp4 = (b_matrix->data)[i + 3];

        (b_matrix->data)[i] = (b_matrix->data)[total_items - i - 1];
        (b_matrix->data)[i + 1] = (b_matrix->data)[total_items - i + 1 - 1];
        (b_matrix->data)[i + 2] = (b_matrix->data)[total_items - i + 2 - 1];
        (b_matrix->data)[i + 3] = (b_matrix->data)[total_items - i + 3 - 1];

        (b_matrix-> data)[total_items - i - 1] = temp1;
        (b_matrix-> data)[total_items - i + 1 - 1] = temp2;
        (b_matrix-> data)[total_items - i + 2 - 1] = temp3;
        (b_matrix-> data)[total_items - i + 3 - 1] = temp4;

    }
      //tail case for elements beyond factor of 4
      for(uint32_t i = total_items / 2 /4 * 4; i < total_items / 2; i++) {
          int32_t temp = (b_matrix->data)[i];
          
          (b_matrix->data)[i] = (b_matrix->data)[total_items - i - 1];
          (b_matrix-> data)[total_items - i - 1] = temp;
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
      //number of calls within one row is the shifter
      int shifter = 1;
      int row_counter = 1;

      #pragma omp parallel for
      for (uint32_t output_counter = 0; output_counter < (output_cols * output_rows); output_counter++) {
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
      int b_rows = b_matrix->rows;
      int a_cols = a_matrix->cols;
      int sum = 0;

      int32_t a_copy[total_items];
      int a_copy_index = 0;

      //cut a_array into size B with offset
      if (a_cols > b_cols) {
          #pragma omp parallel for
          for(int i = 0; i < b_rows; i++) {
              for(int j = 0; j < b_cols; j++){
                a_copy[a_copy_index] = a_array[start_location + (i * a_cols) + j];
                a_copy_index += 1;
              }
          }
          a_array = a_copy;
      } 
          //for b_matrix size = a_matrix size 
          //operate on b and a elements 8 at a time
          __m256i sumVec = _mm256_setzero_si256();
          #pragma omp parallel for  
          for(int vector_offset = 0; vector_offset < total_items / 8 * 8; vector_offset += 8)
          {
              //vectorize 8 elements from both a and b
              __m256i b_temp = _mm256_loadu_si256((__m256i*)(b_array + vector_offset));
              __m256i a_temp = _mm256_loadu_si256((__m256i*)(a_array + vector_offset));
            
              //multiply element-wise then add on top of sum vector
              __m256i product = _mm256_mullo_epi32(b_temp, a_temp);
              sumVec = _mm256_add_epi32(sumVec, product);
          }
          //tail case for summing products of elements beyond factor 8
          for (int i = total_items / 8 * 8; i < total_items; i++) {
              sum += b_array[i] * a_array[i];
          }
          //copy sum vector into array
          int temp_arr[8];
          _mm256_storeu_si256((__m256i*)temp_arr, sumVec);
          //add all products
          sum += temp_arr[0] + temp_arr[1] + temp_arr[2] + temp_arr[3] + temp_arr[4] + temp_arr[5]+ temp_arr[6]+ temp_arr[7];
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
