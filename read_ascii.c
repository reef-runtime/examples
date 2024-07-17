#include "reef.h"

void greet_n_times(int n) {
  for (int i = 0; i < n; i++) {
    reef_puts("Hello World!");
    reef_sleep(0.2);
    reef_progress((float) i / (float) n);
  }
}

void run(uint8_t *dataset, size_t ds_len) {
  int pos = 0;

  #define l 120

  char * buf = malloc(l + 1);
  memset(buf, 0x0, l);

  int buf_cursor = 0;
  while (pos < ds_len) {
    // Artificial delay.
    reef_progress((float) pos / (float) ds_len);
    reef_sleep(0.1);

    // Read file line by line.
    buf[buf_cursor++] = dataset[pos];
    if (buf_cursor == l - 1 || dataset[pos++] == '\n') {
      buf_cursor = 0;
      if (strlen(buf) > 0)
        reef_puts(buf);
      memset(buf, 0x0, l);
    }
  }
}
