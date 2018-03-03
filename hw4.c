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
  //read/check input arguments and store
  if (argc < 3 || argc > 4) {
    fprintf(stderr, "usage: ./a.out num_threads option [arg]\n");
    exit(1);
  } else {
    InputArgs *input = (InputArgs *) malloc(sizeof(InputArgs));
    if (input == NULL) {
      perror("MALLOC ERROR");
      exit(1);
    }
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
  if (image == NULL) {
    perror("MALLOC ERROR");
    exit(1);
  }

  //read in and store image header info
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

  return image;
}

ThreadSegment *createSegments(ImageInfo *image, int threadCount) {
  ThreadSegment *segments = (ThreadSegment *) malloc(sizeof(ThreadSegment) * threadCount);
  if (segments == NULL) {
    perror("MALLOC ERROR");
    exit(1);
  }

  //split rows up into segments for each thread to work on
  int segment_length = image->rows / threadCount;
  int i;
  for (i = 0; i < threadCount; i++) {
    if (i != threadCount - 1) {
      segments[i].start = segment_length * i;
      segments[i].end = segment_length * (i+1);
    } else {
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
  if (pixels == NULL) {
    perror("MALLOC ERROR");
    exit(1);
  }
}

void allocatePixelsRotated(ImageInfo *image) {
  //columns and rows swapped for rotated image
  int r, c;
  pixelsRotated = (int ***) malloc(sizeof(int **) * image->columns);
  for (r = 0; r < image->columns; r++) {
    pixelsRotated[r] = (int **) malloc(sizeof(int*) * image->rows);
    for (c = 0; c < image->rows; c++) {
      pixelsRotated[r][c] = (int *) malloc(sizeof(int) * 3);
    }
  }
  if (pixelsRotated == NULL) {
    perror("MALLOC ERROR");
    exit(1);
  }
}

void processPixels(ImageInfo *image) {
  allocatePixels(image);

  //read in pixels to global variable from stdin
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
  //output image header
  printf("%s\n", image->type);
  printf("%d %d\n", image->columns, image->rows);
  printf("%d\n", image->max);

  //output image pixels
  int r, c;
  for (r = 0; r < image->rows; r++) {
    for (c = 0; c < image->columns; c++) {
        printf("%d %d %d\n", pixels[r][c][0], pixels[r][c][1], pixels[r][c][2]);
    }
  }
}

void outputPixelsRotated(ImageInfo *image) {
  //rows and columns swapped for rotated image
  //output image header
  printf("%s\n", image->type);
  printf("%d %d\n", image->rows, image->columns);
  printf("%d\n", image->max);

  //output image pixels
  int r, c;
  for (r = 0; r < image->columns; r++) {
    for (c = 0; c < image->rows; c++) {
        printf("%d %d %d\n", pixelsRotated[r][c][0], pixelsRotated[r][c][1], pixelsRotated[r][c][2]);
    }
  }
}

void *invertPixels(void *args) {
  //unpack
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int max = threadArgs->image->max;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  //invert pixels
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
  //unpack
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int max = threadArgs->image->max;
  float contrast = threadArgs->contrast;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  //add contrast
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
  //unpack
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  //remove greens and blues
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
  //unpack
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  //remove reds and blues
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
  //unpack
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  //remove reds and greens
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
  //unpack
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int rows = threadArgs->image->rows;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  //rotate right
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
  //unpack
  ThreadArgs *threadArgs = (ThreadArgs *) args;
  int columns = threadArgs->image->columns;
  int start = threadArgs->segment->start;
  int end = threadArgs->segment->end;

  //rotate left
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
  //allocate threads
  pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * input->num_threads);
  if (threads == NULL) {
    perror("MALLOC ERROR");
    exit(1);
  }

  //segments for threads to work on (start and stop indexes)
  ThreadSegment *segments = createSegments(image, input->num_threads);

  //arguments to send to option function
  ThreadArgs *args = (ThreadArgs *) malloc(sizeof(ThreadArgs));
  if (args == NULL) {
    perror("MALLOC ERROR");
    exit(1);
  }
  args->image = image;
  args->contrast = input->contrast;

  //create thread, call conversion, close thread
  int i;
  for (i = 0; i < input->num_threads; i++) {
    args->segment = &segments[i];
    pthread_create(&threads[i], NULL, option, (void *) args);
    pthread_join(threads[i], NULL);
  }

  //free thread/segment memory
  free(threads);
  free(segments);
  free(args);
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
  //columns and rows swapped for rotated image
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


int main (int argc, char **argv) {
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

  //output
  if (image->rotated == FALSE) {
    outputPixels(image);
    freePixels(image);
  } else {
    outputPixelsRotated(image);
    freePixelsRotated(image);
  }

  //free image/input memory
  free(image);
  free(input);

  return 0;
}
