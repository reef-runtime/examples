#include "reef.h"

#define FIRST_N 100

void concat(char* str, char* to_add, size_t len) {
   int i = 0;
   for (; str[i] != '\0'; i++);

   for (int j = 0; j < len; j++) {
      str[i+j] = to_add[j];
   }
   str[i+len] = '\0';
}

void sort(uint32_t *arr, size_t len)
{
  float progress = 0;
  int i, key, j;
  for (i = 1; i < len; i++) {
    key = arr[i];
    j = i - 1;
    
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j = j - 1;
    }
    arr[j + 1] = key;
    
    progress += 1 / (float) len;

    if (i % (len / 10) == 0) {
	  reef_progress(progress);
    }
  }
}

void run(uint8_t *dataset, size_t ds_len) {
  uint32_t *data = from_little_endian(dataset, ds_len);
  size_t data_sz = ds_len / 4;

  sort(data, data_sz);

  char result[5000];
  result[0] = '\0';

  char num[16];
  size_t len;
  for (int i = 0; i < FIRST_N; i++) {
    itoa(data[i], &num[0], 10);
	len = strlen(num);

    if (i != 0 && i % 15 == 0) {
      num[len] = ',';
      num[len+1] = '\n';
      num[len+2] = '\0';
      concat(result, num, len+2);
      continue;
    }
  
	num[len] = ',';
	num[len+1] = ' ';
    num[len+2] = '\0';  
	concat(result, num, len+2);
	
    memset(num, 0, 16);
  }

  reef_result_string(result, strlen(result));
}

