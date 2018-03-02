#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

int ***pixels;
int ***pixelsRotated;

typedef struct InputArgs {
  int num_threads;
  char *option;
  float contrast;
} InputArgs;

typedef struct ImageInfo {
  char type[10];
  int rows;
  int columns;
  int max;
  int rotated;
} ImageInfo;

typedef struct ThreadSegment {
  int start;
  int end;
} ThreadSegment;

InputArgs *processInputArgs(int argc, char **argv) {
  if (argc < 3 || argc > 4) {
    fprintf(stderr, "usage: ./a.out num_threads option [arg]\n");
    exit(1);
  } else {
    InputArgs *input = (InputArgs *) malloc(sizeof(InputArgs));
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
  ImageInfo *image;
  image = (ImageInfo *) malloc(sizeof(ImageInfo));

  if (scanf("%s", image->type) < 0) {
    perror("SCANF ERROR");
    exit(1);
  }
  if (scanf("%d", &image->columns) < 0) {
    perror("SCANF ERROR");
    exit(1);
  }
  if (scanf("%d", &image->rows) < 0) {
    perror("SCANF ERROR");
    exit(1);
  }
  if (scanf("%d", &image->max) < 0) {
    perror("SCANF ERROR");
    exit(1);
  }
  image->rotated = FALSE;

  fprintf(stderr, "Image Type: %s\n", image->type);
  fprintf(stderr, "Image Columns: %d\n", image->columns);
  fprintf(stderr, "Image Rows: %d\n", image->rows);
  fprintf(stderr, "Image Max: %d\n", image->max);

  return image;
}

ThreadSegment *createSegments(ImageInfo *image, InputArgs *input) {
  ThreadSegment *segments = (ThreadSegment *) malloc(sizeof(ThreadSegment) * input->num_threads);

  int segment_length = image->rows / input->num_threads;
  int i;
  for (i = 0; i < input->num_threads; i++) {
    if (i != input->num_threads - 1) {
      fprintf(stderr, "start: %d| end: %d| diff: %d\n", segment_length * i, segment_length * (i+1), (segment_length * (i+1)) - (segment_length * i));
      segments[i].start = segment_length * i;
      segments[i].end = segment_length * (i+1);
    } else {
      fprintf(stderr, "start: %d| end: %d| diff: %d\n", segment_length * i, image->rows, image->rows - (segment_length * i));
      segments[i].start = segment_length * i;
      segments[i].end = image->rows;
    }
  }
  return segments;
}

void allocatePixels(ImageInfo *image) {
  int r, c;
  pixels = (int ***) malloc(sizeof(int **) * image->rows);
  for (r = 0; r < image->rows; r++) {
    pixels[r] = (int **) malloc(sizeof(int*) * image->columns);
    for (c = 0; c < image->columns; c++) {
      pixels[r][c] = (int *) malloc(sizeof(int) * 3);
    }
  }
}

void allocatePixelsRotated(ImageInfo *image) {
  int r, c;
  pixelsRotated = (int ***) malloc(sizeof(int **) * image->columns);
  for (r = 0; r < image->columns; r++) {
    pixelsRotated[r] = (int **) malloc(sizeof(int*) * image->rows);
    for (c = 0; c < image->rows; c++) {
      pixelsRotated[r][c] = (int *) malloc(sizeof(int) * 3);
    }
  }
}

void processPixels(ImageInfo *image) {
  allocatePixels(image);

  int r, c, p;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (scanf("%d", &pixels[r][c][p]) < 0) {
          perror("SCAN ERROR");
          exit(1);
        }
      }
    }
  }
}

void outputPixels(ImageInfo *image) {
  printf("%s\n", image->type);
  printf("%d %d\n", image->columns, image->rows);
  printf("%d\n", image->max);

  int r, c;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
        printf("%d %d %d\n", pixels[r][c][0], pixels[r][c][1], pixels[r][c][2]);
    }
  }
}

void outputPixelsRotated(ImageInfo *image) {
  printf("%s\n", image->type);
  printf("%d %d\n", image->rows, image->columns);
  printf("%d\n", image->max);

  int r, c;
  for (r = 0; r < image->columns; r++) {
    for (c = 0; c < image->rows; c++) {
        printf("%d %d %d\n", pixelsRotated[r][c][0], pixelsRotated[r][c][1], pixelsRotated[r][c][2]);
    }
  }
}

void invertPixels(ImageInfo *image) {
  int r, c, p;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
      for (p = 0; p < 3; p++) {
        pixels[r][c][p] = image->max - pixels[r][c][p];
      }
    }
  }
}

void contrastPixels(ImageInfo *image, float contrast) {
  int r, c, p;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (pixels[r][c][p] <= (image->max / 2)) {
          pixels[r][c][p] -= image->max * contrast;
        } else {
          pixels[r][c][p] += image->max * contrast;
        }
      }
    }
  }
}

void *redPixels(ImageInfo *image) {
  int r, c, p;
  //r = segment->start ; r < segment->end
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (p != 0) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
}

void greenPixels(ImageInfo *image) {
  int r, c, p;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (p != 1) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
}

void bluePixels(ImageInfo *image) {
  int r, c, p;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
      for (p = 0; p < 3; p++) {
        if (p != 2) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
}

void rotateRight(ImageInfo *image) {
  allocatePixelsRotated(image);

  int r, c, p;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
      for (p = 0; p < 3; p++) {
        pixelsRotated[c][image->rows - r - 1][p] = pixels[r][c][p];
      }
    }
  }
  image->rotated = TRUE;
  // int tmp = image->rows;
  // image->rows = image->columns;
  // image->columns = tmp;
}

void rotateLeft(ImageInfo *image) {
  allocatePixelsRotated(image);

  int r, c, p;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
      for (p = 0; p < 3; p++) {
        pixelsRotated[image->columns - c - 1][r][p] = pixels[r][c][p];
      }
    }
  }
  image->rotated = TRUE;
}

void freePixels(ImageInfo *image) {
  int r, c;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
      free(pixels[r][c]);
    }
  }
  for (r = 0; r < image->rows; r++) {
    free(pixels[r]);
  }
  free(pixels);
}

void freePixelsRotated(ImageInfo *image) {
  int r, c;
  for (r = 0; r < image->columns; r++) {
    for (c = 0; c < image->rows; c++) {
      free(pixelsRotated[r][c]);
    }
  }
  for (r = 0; r < image->columns; r++) {
    free(pixelsRotated[r]);
  }
  free(pixelsRotated);
}

void freeMemory(ImageInfo *image, InputArgs *input) {
  freePixels(image);
  free(image);
  free(input);
}


int main (int argc, char **argv) {
  //error check mallocs

  //structs
  InputArgs *input = processInputArgs(argc, argv);
  ImageInfo *image = processImageInfo();

  //read pixels
  processPixels(image);

  //allocate threads
  pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * input->num_threads);
  //create segments
  ThreadSegment *segments = createSegments(image, input);
  //create threads
  int i;
  for (i = 0; i < input->num_threads; i++) {
    pthread_create(&threads[i], NULL, function*, function args)
  }


  //conversions
  if (strcmp(input->option, "-I") == 0) {
    invertPixels(image);
  }
  if (strcmp(input->option, "-C") == 0) {
    contrastPixels(image, input->contrast);
  }
  if (strcmp(input->option, "-red") == 0) {
    redPixels(image);
  }
  if (strcmp(input->option, "-green") == 0) {
    greenPixels(image);
  }
  if (strcmp(input->option, "-blue") == 0) {
    bluePixels(image);
  }
  if (strcmp(input->option, "-R") == 0) {
    rotateRight(image);
  }
  if (strcmp(input->option, "-L") == 0) {
    rotateLeft(image);
  }

  //output            /try to get one output function by swapping width and height
  if (image->rotated == FALSE) {
    outputPixels(image);
  } else {
    outputPixelsRotated(image);
    freePixelsRotated(image);
  }

  //free
  freeMemory(image, input);

  return 0;
}





// char image[100];
// fread(&image, 1, 100, stdin);
// fprintf(stderr, "%s\n", image);
