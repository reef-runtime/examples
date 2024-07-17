#include "reef.h"

#define ROWS 30
#define COLS 3

enum {
   DATE, 
   TEMPERATURE,
   PRECIPITATION
};

int get_max(uint32_t **arr, int type) {
   int max = 0;
   for (int i = 0; i < ROWS; i++) {
       if (arr[i][type] > max) {
           max = arr[i][type];
       }
   }
   return max;
}

void concat(char* str, char* to_add, size_t len) {
   int i = 0;
   for (; str[i] != '\0'; i++);

   for (int j = 0; j < len; j++) {
      str[i+j] = to_add[j];
   }
   str[i+len] = '\0';
}

void run(uint8_t *dataset, size_t ds_len) {
   uint32_t *ints = from_little_endian(dataset, ds_len);
   uint32_t **arr = malloc(ROWS * sizeof(uint32_t *));
   for (int i = 0; i < ROWS; i++) {
      arr[i] = ints + i * COLS;
   }

   char result[5000];
   result[0] = '\0';
   
   int type = TEMPERATURE;
   int max = get_max(arr, type);
   concat(result, "^ ", 2);
   char max_str[3];
   itoa(max, max_str, 10);
   concat(result, max_str, strlen(max_str));
   concat(result, "\n", 1);
   for (int i = max-1; i >= 0; i = i-(max/10)) {
      concat(result, "|", 1);
      for (int j = 0; j < ROWS; j++) {
         if (arr[j][type] > i) {
            concat(result, " X", 2);
         } else {
            concat(result, "  ", 2);
         }
      }
      concat(result, "\n", 1);
   }
   concat(result, "+", 1);
   for (int i = 0; i < ROWS; i++) {
      concat(result, "-|", 2);
   }
   concat(result, "-->\n", 4);
   reef_result_string(result, strlen(result));
}