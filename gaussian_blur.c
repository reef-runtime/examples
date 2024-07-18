#include "reef.h"

#define KERNEL_WIDTH 5
#define KERNEL_HEIGHT 5
#define KERNEL_SIGMA 10.0

void *calloc(size_t nelem, size_t elsize) {
  void *p;

  p = malloc(nelem * elsize);
  if (p == 0)
    return (p);

  memset(p, 0, nelem * elsize);
  return (p);
}

// Ported from
// https://gist.github.com/OmarAflak/aca9d0dc8d583ff5a5dc16ca5cdda86a

typedef struct {
  double **buf;
  size_t height;
  size_t width;
} Matrix;

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} Pixel;

typedef struct {
  Pixel **data;
  size_t height;
  size_t width;
} Image;

size_t append(uint8_t *buf, size_t idx, uint8_t *to_add, size_t len) {
  for (int j = 0; j < len; j++) {
    buf[idx++] = to_add[j];
  }

  return idx;
}

#define PI 3.14

// ported from https://gist.github.com/jrade/293a73f89dfef51da6522428c857802d
float exp(float x) {
  float a = (1 << 23) / 0.69314718f;
  float b = (1 << 23) * (127 - 0.043677448f);
  x = a * x + b;

  // Remove these lines if bounds checking is not needed
  float c = (1 << 23);
  float d = (1 << 23) * 255;
  if (x < c || x > d)
    x = (x < c) ? 0.0f : d;

  uint32_t n = *(uint32_t *)&x;
  memcpy(&x, &n, 4);
  return x;
}

Matrix *getGaussian(int height, int width, double sigma) {
  double **kernel = (double **)malloc(height * sizeof(double *));
  for (int i = 0; i < height; i++) {
    kernel[i] = (double *)malloc(width * sizeof(double));
  }

  double sum = 0.0;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      kernel[i][j] = exp(-(i * i + j * j) / (2 * sigma * sigma)) /
                     (2 * PI * sigma * sigma);
      sum += kernel[i][j];
    }
  }

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      kernel[i][j] /= sum;
    }
  }

  Matrix *m = (Matrix *)malloc(sizeof(Matrix));

  m->height = height;
  m->width = width;
  m->buf = kernel;

  return m;
}

int str_to_int(char *str) {
  int num = 0;

  for (int i = 0; str[i] != '\0'; i++) {
    num = num * 10 + (str[i] - 48);
  }

  return num;
}

Image *parse_ppm(uint8_t *dataset, size_t ds_len) {
  if (dataset[0] != 'P' || dataset[1] != '3') {
    reef_puts("Invalid .ppm file.");
  }

  int i;

  char num[10] = {0};
  int j = 0;
  for (i = 3; dataset[i] != ' ' && j < 10; i++) {
    num[j] = dataset[i];
    j++;
  }

  int width = str_to_int(&num[0]);

  // Skip over space
  i++;

  memset(num, 0, 10);

  for (j = 0; dataset[i] != '\n' && j < 10; i++) {
    num[j] = dataset[i];
    j++;
  }

  int height = str_to_int(&num[0]);

  // Skip over newline
  i += 1;

  // Skip over value range (should always be 255)
  i += 4;

  Pixel **buf = (Pixel **)malloc(height * sizeof(Pixel *));

  unsigned char r, g, b;
  Pixel p;
  for (int h = 0; h < height; h++) {
    buf[h] = (Pixel *)malloc(width * sizeof(Pixel));

    for (int w = 0; w < width; w++) {
      memset(num, 0, 10);

      while (dataset[i] == ' ' || dataset[i] == '\n') {
        i++;
      }

      for (j = 0; dataset[i] != ' ' && dataset[i] != '\n' && j < 10; i++) {
        num[j] = dataset[i];
        j++;
      }
      p.red = (uint8_t)str_to_int(&num[0]);

      while (dataset[i] == ' ' || dataset[i] == '\n') {
        i++;
      }

      memset(num, 0, 10);
      for (j = 0; dataset[i] != ' ' && dataset[i] != '\n' && j < 10; i++) {
        num[j] = dataset[i];
        j++;
      }
      p.green = (uint8_t)str_to_int(&num[0]);

      while (dataset[i] == ' ' || dataset[i] == '\n') {
        i++;
      }

      memset(num, 0, 10);
      for (j = 0; dataset[i] != ' ' && dataset[i] != '\n' && j < 10; i++) {
        num[j] = dataset[i];
        j++;
      }
      p.blue = (uint8_t)str_to_int(&num[0]);

      buf[h][w] = p;
    }
  }

  Image *img = (Image *)malloc(sizeof(Image));
  img->height = height;
  img->width = width;
  img->data = buf;

  return img;
}

void saveImage(Image *image) {
  char width[10];
  itoa(image->width, &width[0], 10);

  char height[10];
  itoa(image->height, &height[0], 10);

  // magic byte + image sizes + value range
  size_t final_image_sz = 3 + strlen(width) + 1 + strlen(height) + 6;

  // whitespace
  final_image_sz += 3 * image->height * (image->width + 1);

  // pixel values
  final_image_sz += image->height * image->width * 3 * 3;

  uint8_t *output_buffer = (uint8_t *)malloc(final_image_sz);
  size_t idx = 0;

  idx = append(output_buffer, idx, (uint8_t *)"P3\n", 3);

  idx = append(output_buffer, idx, (uint8_t *)&width[0], strlen(width));
  idx = append(output_buffer, idx, (uint8_t *)" ", 1);
  idx = append(output_buffer, idx, (uint8_t *)&height[0], strlen(height));
  idx = append(output_buffer, idx, (uint8_t *)"\n255\n", 5);

  char num[4];
  for (int h = 0; h < image->height; h++) {
    for (int w = 0; w < image->width; w++) {
      memset(num, 0, 4);
      itoa((uint8_t)image->data[h][w].red, &num[0], 10);
      idx = append(output_buffer, idx, (uint8_t *)&num[0], strlen(num));

      idx = append(output_buffer, idx, (uint8_t *)" ", 1);

      memset(num, 0, 4);
      itoa((uint8_t)image->data[h][w].green, &num[0], 10);
      idx = append(output_buffer, idx, (uint8_t *)&num[0], strlen(num));

      idx = append(output_buffer, idx, (uint8_t *)" ", 1);

      memset(num, 0, 4);
      itoa((uint8_t)image->data[h][w].blue, &num[0], 10);
      idx = append(output_buffer, idx, (uint8_t *)&num[0], strlen(num));

      idx = append(output_buffer, idx, (uint8_t *)" ", 1);
    }

    idx = append(output_buffer, idx, (uint8_t *)"\n", 1);
  }

  reef_result_bytes(output_buffer, idx);
}

Image *applyFilter(Image *image, Matrix *filter) {
  int height = image->height;
  int width = image->width;
  int filterHeight = filter->height;
  int filterWidth = filter->width;
  int newImageHeight = height - filterHeight + 1;
  int newImageWidth = width - filterWidth + 1;
  int d, i, j, h, w;

  Image *newImage = (Image *)malloc(sizeof(Image));
  newImage->height = newImageHeight;
  newImage->width = newImageWidth;

  Pixel **buf = (Pixel **)malloc(newImageHeight * sizeof(Pixel *));

  for (int h = 0; h < newImageHeight; h++) {
    buf[h] = (Pixel *)malloc(newImageWidth * sizeof(Pixel));
    memset(buf[h], 0, newImageWidth * sizeof(Pixel));
  }

  newImage->data = buf;

  for (d = 0; d < 3; d++) {
    for (i = 0; i < newImageHeight; i++) {
      for (j = 0; j < newImageWidth; j++) {
        for (h = i; h < i + filterHeight; h++) {
          for (w = j; w < j + filterWidth; w++) {
            switch (d) {
            case 0:
              newImage->data[i][j].red += (int)(filter->buf[h - i][w - j] *
                                                (double)image->data[h][w].red);
              break;
            case 1:
              newImage->data[i][j].green +=
                  (int)(filter->buf[h - i][w - j] *
                        (double)image->data[h][w].green);
              break;
            case 2:
              newImage->data[i][j].blue +=
                  (int)(filter->buf[h - i][w - j] *
                        (double)image->data[h][w].blue);
              break;
            }
          }
        }
      }
    }
  }

  return newImage;
}

void run(uint8_t *dataset, size_t ds_len) {
  Matrix *filter = getGaussian(KERNEL_HEIGHT, KERNEL_WIDTH, KERNEL_SIGMA);

  reef_puts("Loading image...");
  reef_progress(0.05);

  Image *image = parse_ppm(dataset, ds_len);
  reef_puts("Applying filter...");
  reef_progress(0.15);

  Image *newImage = applyFilter(image, filter);
  reef_puts("Saving image...");
  reef_progress(0.8);

  saveImage(newImage);
  reef_puts("Done!");
}
