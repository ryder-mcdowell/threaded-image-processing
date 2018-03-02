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

typedef struct ThreadArgs {
  struct ImageInfo *image;
  struct ThreadSegment *segment;
  float contrast;
} ThreadArgs;

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

ThreadSegment *createSegments(ImageInfo *image, int threadCount) {
  ThreadSegment *segments = (ThreadSegment *) malloc(sizeof(ThreadSegment) * threadCount);

  int segment_length = image->rows / threadCount;
  int i;
  for (i = 0; i < threadCount; i++) {
    if (i != threadCount - 1) {
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

void *invertPixels(void *args) {
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int max = threadArgs->image->max;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  int r, c, p;
  for (r = start; r < end; r++) {
    for (c = 0; c < columns; c++) {
      for (p = 0; p < 3; p++) {
        pixels[r][c][p] = max - pixels[r][c][p];
      }
    }
  }
  return NULL;
}

void *contrastPixels(void *args) {
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int max = threadArgs->image->max;
  float contrast = threadArgs->contrast;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  int r, c, p;
  for (r = start; r < end; r++) {
    for (c = 0; c < columns; c++) {
      for (p = 0; p < 3; p++) {
        if (pixels[r][c][p] <= (max / 2)) {
          pixels[r][c][p] -= max * contrast;
        } else {
          pixels[r][c][p] += max * contrast;
        }
      }
    }
  }
  return NULL;
}

void *redPixels(void *args) {
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  int r, c, p;
  for (r = start; r < end; r++) {
    for (c = 0; c < columns; c++) {
      for (p = 0; p < 3; p++) {
        if (p != 0) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
  return NULL;
}

void *greenPixels(void *args) {
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  int r, c, p;
  for (r = start; r < end; r++) {
    for (c = 0; c < columns; c++) {
      for (p = 0; p < 3; p++) {
        if (p != 1) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
  return NULL;
}

void *bluePixels(void *args) {
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  int r, c, p;
  for (r = start; r < end; r++) {
    for (c = 0; c < columns; c++) {
      for (p = 0; p < 3; p++) {
        if (p != 2) {
          pixels[r][c][p] = 0;
        }
      }
    }
  }
  return NULL;
}

void *rotateRight(void *args) {
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int rows = threadArgs->image->rows;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  fprintf(stderr, "start: %d| end: %d| col: %d\n", start, end, columns);
  int r, c, p;
  for (r = start; r < end; r++) {
    for (c = 0; c < columns; c++) {
      for (p = 0; p < 3; p++) {
        pixelsRotated[c][rows - r - 1][p] = pixels[r][c][p];
      }
    }
  }
  threadArgs->image->rotated = TRUE;
  return NULL;
}

void *rotateLeft(void *args) {
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  fprintf(stderr, "start: %d| end: %d| col: %d\n", start, end, columns);
  int r, c, p;
  for (r = start; r < end; r++) {
    for (c = 0; c < columns; c++) {
      for (p = 0; p < 3; p++) {
        pixelsRotated[columns - c - 1][r][p] = pixels[r][c][p];
      }
    }
  }
  threadArgs->image->rotated = TRUE;
  return NULL;
}

void callThreads(void *option, InputArgs *input, ImageInfo *image) {
  pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * input->num_threads);

  ThreadSegment *segments = createSegments(image, input->num_threads);

  ThreadArgs *args = (ThreadArgs *) malloc(sizeof(ThreadArgs));
  args->image = image;
  args->contrast = input->contrast;

  fprintf(stderr, "---------------------\n");
  int i;
  for (i = 0; i < input->num_threads; i++) {
    args->segment = &segments[i];
    pthread_create(&threads[i], NULL, option, (void *) args);
    pthread_join(threads[i], NULL);
  }
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
  //free ThreadSegment and ThreadArgs

  //structs
  InputArgs *input = processInputArgs(argc, argv);
  ImageInfo *image = processImageInfo();

  //read pixels
  processPixels(image);

  //conversions
  if (strcmp(input->option, "-I") == 0) {
    callThreads(invertPixels, input, image);
  }
  if (strcmp(input->option, "-C") == 0) {
    callThreads(contrastPixels, input, image);
  }
  if (strcmp(input->option, "-red") == 0) {
    callThreads(redPixels, input, image);
  }
  if (strcmp(input->option, "-green") == 0) {
    callThreads(greenPixels, input, image);
  }
  if (strcmp(input->option, "-blue") == 0) {
    callThreads(bluePixels, input, image);
  }
  if (strcmp(input->option, "-R") == 0) {
    allocatePixelsRotated(image);
    callThreads(rotateRight, input, image);
  }
  if (strcmp(input->option, "-L") == 0) {
    allocatePixelsRotated(image);
    callThreads(rotateLeft, input, image);
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
