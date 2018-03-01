#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int ***pixels;

typedef struct InputArgsData {
  int num_threads;
  char *option;
  float contrast;
} InputArgsData;

typedef struct ImageInfo {
  char type[10];
  int rows;
  int columns;
  int max;
} ImageInfo;

InputArgsData *processInputArgs(int argc, char **argv) {
  if (argc < 3 || argc > 4) {
    fprintf(stderr, "usage: ./a.out num_threads option [arg]\n");
    exit(1);
  } else {
    InputArgsData *input = (InputArgsData *) malloc(sizeof(InputArgsData));
    input->num_threads = atoi(argv[1]);
    input->option = argv[2];
    if (argc == 4) {
      if (atof(argv[3]) > 0 && atof(argv[3]) < 1) {
        input->contrast = atof(argv[3]);
      } else {
        fprintf(stderr, "ERROR: contrast value must be between 0 and 1\n");
        exit(1);
      }
    }
    return input;
  }
}

ImageInfo *processImageInfo() {
  ImageInfo *imageInfo;
  imageInfo = (ImageInfo *) malloc(sizeof(ImageInfo));

  if (scanf("%s", imageInfo->type) < 0) {
    perror("SCANF ERROR");
    exit(1);
  }
  if (scanf("%d", &imageInfo->rows) < 0) {
    perror("SCANF ERROR");
    exit(1);
  }
  if (scanf("%d", &imageInfo->columns) < 0) {
    perror("SCANF ERROR");
    exit(1);
  }
  if (scanf("%d", &imageInfo->max) < 0) {
    perror("SCANF ERROR");
    exit(1);
  }

  fprintf(stderr, "Image Type: %s\n", imageInfo->type);
  fprintf(stderr, "Image Rows: %d\n", imageInfo->rows);
  fprintf(stderr, "Image Colums: %d\n", imageInfo->columns);
  fprintf(stderr, "Image Max: %d\n", imageInfo->max);

  return imageInfo;
}

int ***allocatePixels(ImageInfo *imageInfo) {
  pixels = (int ***) malloc(sizeof(int **) * imageInfo->rows);
  for (int r = 0; r < imageInfo->rows; r++) {
    pixels[r] = (int **) malloc(sizeof(int*) * imageInfo->columns);
    for (int c = 0; c < imageInfo->columns; c++) {
      pixels[r][c] = (int *) malloc(sizeof(int) * 3);
    }
  }
  return pixels;
}

int ***processPixels(ImageInfo *imageInfo) {
  pixels = allocatePixels(imageInfo);

  for (int r = 0; r < imageInfo->rows; r++) {
    for (int c = 0; c < imageInfo->columns; c++) {
      for (int p = 0; p < 3; p++) {
        if (scanf("%d", &pixels[r][c][p]) < 0) {
          perror("SCAN ERROR");
          exit(1);
        }
      }
    }
  }
  return pixels;
}

void outputPixels(int ***pixels, ImageInfo *imageInfo) {
  printf("%s\n", imageInfo->type);
  printf("%d %d\n", imageInfo->rows, imageInfo->columns);
  printf("%d\n", imageInfo->max);
  for (int r = 0; r < imageInfo->rows; r++) {
    for (int c = 0; c < imageInfo->columns; c++) {
        printf("%d %d %d\n", pixels[r][c][0], pixels[r][c][1], pixels[r][c][2]);
    }
  }
}

int ***invertPixels(int ***pixels, ImageInfo *imageInfo) {
  for (int r = 0; r < imageInfo->rows; r++) {
    for (int c = 0; c < imageInfo->columns; c++) {
      for (int p = 0; p < 3; p++) {
        pixels[r][c][p] = imageInfo->max - pixels[r][c][p];
      }
    }
  }
  return pixels;
}

int ***contrastPixels(int ***pixels, ImageInfo *imageInfo, float contrast) {
  for (int r = 0; r < imageInfo->rows; r++) {
    for (int c = 0; c < imageInfo->columns; c++) {
      for (int p = 0; p < 3; p++) {
        if (pixels[r][c][p] <= (imageInfo->max / 2)) {
          pixels[r][c][p] -= imageInfo->max * contrast;
        } else {
          pixels[r][c][p] += imageInfo->max * contrast;
        }
      }
    }
  }
  return pixels;
}

int ***redPixels(int ***pixels, ImageInfo *imageInfo) {
  for (int r = 0; r < imageInfo->rows; r++) {
    for (int c = 0; c < imageInfo->columns; c++) {
      for (int p = 0; p < 3; p++) {
        if (p != 0) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
  return pixels;
}

int ***greenPixels(int ***pixels, ImageInfo *imageInfo) {
  for (int r = 0; r < imageInfo->rows; r++) {
    for (int c = 0; c < imageInfo->columns; c++) {
      for (int p = 0; p < 3; p++) {
        if (p != 1) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
  return pixels;
}

int ***bluePixels(int ***pixels, ImageInfo *imageInfo) {
  for (int r = 0; r < imageInfo->rows; r++) {
    for (int c = 0; c < imageInfo->columns; c++) {
      for (int p = 0; p < 3; p++) {
        if (p != 2) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
  return pixels;
}


int main (int argc, char **argv) {

  InputArgsData *input = processInputArgs(argc, argv);

  ImageInfo *imageInfo = processImageInfo();

  // char image[100];
  // fread(&image, 1, 100, stdin);
  // fprintf(stderr, "%s\n", image);

  pixels = processPixels(imageInfo);

  if (strcmp(input->option, "-I") == 0) {
    pixels = invertPixels(pixels, imageInfo);
  }
  if (strcmp(input->option, "-C") == 0) {
    pixels = contrastPixels(pixels, imageInfo, input->contrast);
  }
  if (strcmp(input->option, "-red") == 0) {
    pixels = redPixels(pixels, imageInfo);
  }
  if (strcmp(input->option, "-green") == 0) {
    pixels = greenPixels(pixels, imageInfo);
  }
  if (strcmp(input->option, "-blue") == 0) {
    pixels = bluePixels(pixels, imageInfo);
  }

  outputPixels(pixels, imageInfo);

  //free pixels
  free(imageInfo);
  free(input);

  return 0;
}
