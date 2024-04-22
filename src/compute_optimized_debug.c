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
}// Executes a task
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
