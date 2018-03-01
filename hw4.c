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

void allocatePixels(ImageInfo *imageInfo) {
  int r, c;
  pixels = (int ***) malloc(sizeof(int **) * imageInfo->rows);
  for (r = 0; r < imageInfo->rows; r++) {
    pixels[r] = (int **) malloc(sizeof(int*) * imageInfo->columns);
    for (c = 0; c < imageInfo->columns; c++) {
      pixels[r][c] = (int *) malloc(sizeof(int) * 3);
    }
  }
}

void processPixels(ImageInfo *imageInfo) {
  allocatePixels(imageInfo);

  int r, c, p;
  for (r = 0; r < imageInfo->rows; r++) {
    for (c = 0; c < imageInfo->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (scanf("%d", &pixels[r][c][p]) < 0) {
          perror("SCAN ERROR");
          exit(1);
        }
      }
    }
  }
}

void outputPixels(ImageInfo *imageInfo) {
  printf("%s\n", imageInfo->type);
  printf("%d %d\n", imageInfo->rows, imageInfo->columns);
  printf("%d\n", imageInfo->max);

  int r, c;
  for (r = 0; r < imageInfo->rows; r++) {
    for (c = 0; c < imageInfo->columns; c++) {
        printf("%d %d %d\n", pixels[r][c][0], pixels[r][c][1], pixels[r][c][2]);
    }
  }
}

void invertPixels(ImageInfo *imageInfo) {
  int r, c, p;
  for (r = 0; r < imageInfo->rows; r++) {
    for (c = 0; c < imageInfo->columns; c++) {
      for (p = 0; p < 3; p++) {
        pixels[r][c][p] = imageInfo->max - pixels[r][c][p];
      }
    }
  }
}

void contrastPixels(ImageInfo *imageInfo, float contrast) {
  int r, c, p;
  for (r = 0; r < imageInfo->rows; r++) {
    for (c = 0; c < imageInfo->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (pixels[r][c][p] <= (imageInfo->max / 2)) {
          pixels[r][c][p] -= imageInfo->max * contrast;
        } else {
          pixels[r][c][p] += imageInfo->max * contrast;
        }
      }
    }
  }
}

void redPixels(ImageInfo *imageInfo) {
  int r, c, p;
  for (r = 0; r < imageInfo->rows; r++) {
    for (c = 0; c < imageInfo->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (p != 0) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
}

void greenPixels(ImageInfo *imageInfo) {
  int r, c, p;
  for (r = 0; r < imageInfo->rows; r++) {
    for (c = 0; c < imageInfo->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (p != 1) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
}

void bluePixels(ImageInfo *imageInfo) {
  int r, c, p;
  for (r = 0; r < imageInfo->rows; r++) {
    for (c = 0; c < imageInfo->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (p != 2) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
}

void freePixels(ImageInfo *imageInfo) {
  int r, c;
  for (r = 0; r < imageInfo->rows; r++) {
    for (c = 0; c < imageInfo->columns; c++) {
      free(pixels[r][c]);
    }
  }
  for (r = 0; r < imageInfo->rows; r++) {
    free(pixels[r]);
  }
  free(pixels);
}

void freeMemory(ImageInfo *imageInfo, InputArgsData *input) {
  freePixels(imageInfo);
  free(imageInfo);
  free(input);
}

int main (int argc, char **argv) {

  InputArgsData *input = processInputArgs(argc, argv);

  ImageInfo *imageInfo = processImageInfo();

  // char image[100];
  // fread(&image, 1, 100, stdin);
  // fprintf(stderr, "%s\n", image);

  processPixels(imageInfo);

  if (strcmp(input->option, "-I") == 0) {
    invertPixels(imageInfo);
  }
  if (strcmp(input->option, "-C") == 0) {
    contrastPixels(imageInfo, input->contrast);
  }
  if (strcmp(input->option, "-red") == 0) {
    redPixels(imageInfo);
  }
  if (strcmp(input->option, "-green") == 0) {
    greenPixels(imageInfo);
  }
  if (strcmp(input->option, "-blue") == 0) {
    bluePixels(imageInfo);
  }

  outputPixels(imageInfo);

  freeMemory(imageInfo, input);

  return 0;
}
