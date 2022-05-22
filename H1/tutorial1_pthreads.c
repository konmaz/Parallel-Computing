#include <pthread.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4

int count;
pthread_mutex_t writeLock = PTHREAD_MUTEX_INITIALIZER;
int privateCount[NUM_THREADS * 16];

typedef struct ThreadArgs {
  int size;
  int *array;
  int threadIndex;
} ThreadArgs;

ThreadArgs *createThreadArgs(int size, int *array, int i) {
  ThreadArgs *args = malloc(sizeof(ThreadArgs));
  args->size = size;
  args->array = array;
  args->threadIndex = i;
  return args;
}

int *makeArray(int size) {
  int *array = malloc(size * sizeof(int));
  for (int i = 0; i < size; i++) {
    array[i] = 3;
  }
  return array;
}

void count3(int *array, int size) {
  count = 0;
  for (int i = 0; i < size; i++) {
    if (array[i] == 3) {
      count++;
    }
  }
}

void *count3Threaded(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  int lenPerThread = args->size / NUM_THREADS;
  int startIndex = args->threadIndex * lenPerThread;
  int *array = args->array;
  for (int i = startIndex; i < lenPerThread + startIndex; i++) {
    if (array[i] == 3) {
      count++;
    }
  }
  return EXIT_SUCCESS;
}

void *count3ThreadedV2(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  int lenPerThread = args->size / NUM_THREADS;
  int startIndex = args->threadIndex * lenPerThread;
  int *array = args->array;
  for (int i = startIndex; i < lenPerThread + startIndex; i++) {
    if (array[i] == 3) {
      pthread_mutex_lock(&writeLock);
      count++;
      pthread_mutex_unlock(&writeLock);
    }
  }
  return EXIT_SUCCESS;
}

void *count3ThreadedV3(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  int lenPerThread = args->size / NUM_THREADS;
  int startIndex = args->threadIndex * lenPerThread;
  int *array = args->array;
  int localCount = 0;
  for (int i = startIndex; i < lenPerThread + startIndex; i++) {
    if (array[i] == 3) {
      localCount++;
    }
  }
  pthread_mutex_lock(&writeLock);
  count += localCount;
  pthread_mutex_unlock(&writeLock);
  return EXIT_SUCCESS;
}

// These next versions are just for illustrating false sharing; V3 is the best
void *count3ThreadedV4(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  int lenPerThread = args->size / NUM_THREADS;
  int startIndex = args->threadIndex * lenPerThread;
  int *array = args->array;
  privateCount[args->threadIndex] = 0;
  for (int i = startIndex; i < lenPerThread + startIndex; i++) {
    if (array[i] == 3) {
      privateCount[args->threadIndex]++;
    }
  }
  pthread_mutex_lock(&writeLock);
  count += privateCount[args->threadIndex];
  pthread_mutex_unlock(&writeLock);
  return EXIT_SUCCESS;
}

void *count3ThreadedV5(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  int lenPerThread = args->size / NUM_THREADS;
  int startIndex = args->threadIndex * lenPerThread;
  int *array = args->array;
  int index = args->threadIndex * 16;
  privateCount[index] = 0;
  for (int i = startIndex; i < lenPerThread + startIndex; i++) {
    if (array[i] == 3) {
      privateCount[index]++;
    }
  }
  pthread_mutex_lock(&writeLock);
  count += privateCount[index];
  pthread_mutex_unlock(&writeLock);
  return EXIT_SUCCESS;
}

void startSerialProgram(int *array, int size) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  count3(array, size);

  clock_gettime(CLOCK_MONOTONIC, &end);
  double timeSpent = (end.tv_sec - start.tv_sec);
  timeSpent += (end.tv_nsec - start.tv_nsec) / 1e9;

  printf("Number of 3s: %d -- time spent: %lf\n", count, timeSpent);
}

void startThreadedProgram(int *array, int size, void *threadFunc) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  count = 0;
  pthread_t threadIds[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    ThreadArgs *args = createThreadArgs(size, array, i);
    if (pthread_create(&threadIds[i], NULL, threadFunc, (void *)args)) {
      fprintf(stderr, "Failed to create thread\n");
    }
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threadIds[i], NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  double timeSpent = (end.tv_sec - start.tv_sec);
  timeSpent += (end.tv_nsec - start.tv_nsec) / 1e9;

  printf("Parallel version: Number of 3s: %d -- time spent: %lf\n", count, timeSpent);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("No parameter passed\n");
    return EXIT_FAILURE;
  }
  int size = atoi(argv[1]);
  int *array = makeArray(size);

  startSerialProgram(array, size);
  startThreadedProgram(array, size, count3ThreadedV2);
  startThreadedProgram(array, size, count3ThreadedV3);
  startThreadedProgram(array, size, count3ThreadedV4);
  startThreadedProgram(array, size, count3ThreadedV5);

  free(array);
  return EXIT_SUCCESS;
}