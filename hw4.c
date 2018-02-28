#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct InputArgsData {
  char *num_threads;
  char *option;
  char *contrast;
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
    input->num_threads = argv[1];
    input->option = argv[2];
    if (argc == 4) {
      input->contrast = argv[3];
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
  int ***pixels = (int ***) malloc(sizeof(int **) * imageInfo->rows);
  for (int r = 0; r < imageInfo->rows; r++) {
    pixels[r] = (int **) malloc(sizeof(int*) * imageInfo->columns);
    for (int c = 0; c < imageInfo->columns; c++) {
      pixels[r][c] = (int *) malloc(sizeof(int) * 3);
    }
  }
  return pixels;
}

int ***processPixels(ImageInfo *imageInfo) {
  int ***pixels = allocatePixels(imageInfo);

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

int main (int argc, char **argv) {

  InputArgsData *input = processInputArgs(argc, argv);

  ImageInfo *imageInfo = processImageInfo();

  // char image[100];
  // fread(&image, 1, 100, stdin);
  // fprintf(stderr, "%s\n", image);

  int ***pixels = processPixels(imageInfo);

  


  //free pixels
  free(imageInfo);
  free(input);

  return 0;


}
