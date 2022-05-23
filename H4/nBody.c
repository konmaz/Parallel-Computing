#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
// Small epsilon
#define EPS 0.0000001
pthread_barrier_t   barrier; // used for barrier synchronization
/**
 * @brief Macro for easy termination in case of errors. Usage is similar to printf:
 * FATAL("This prints the number 5 and exits with an error code: %d\n", 5);
 */
#define FATAL(fmt, ...)                                                                                              \
    do {                                                                                                             \
        fprintf(stderr, "Fatal error: %s in %s() on line %d:\n\t" fmt, __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE);                                                                                          \
    } while (0)

/**
 * @brief 3D vector.
 */
typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

/**
 * @brief A particle consisting of a position, velocity and mass.
 */
typedef struct Particle {
    vec3 pos;
    vec3 v;
    float mass;
} Particle;
/**
 * @brief A struct that helps to pass arguments to a thread
 */
typedef struct ThreadArgs {
    Particle *particlesArray;
    int n; // size of array particlesArray
    int threadIndex;
    vec3 *acc;
    int numThreads;
} ThreadArgs;

/**
 * @brief Creates a ThreadArgs struct sets its values and returns the pointer of it.
 *
 * @param particlesArray The particles to simulate.
 * @param n The size of the particlesArray.
 * @param threadIndex The index of a thread it is used inside a thread to calculate the array positions.
 */
ThreadArgs *createThreadArgs(Particle *particlesArray, int n, int threadIndex, vec3* acc, int numThreads) {
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    args->particlesArray = particlesArray;
    args->n = n;
    args->threadIndex = threadIndex;
    args->acc = acc;
    args->numThreads = numThreads;
    return args;
}

/**
 * @brief Reads input particles from a given file. The first line of the file is the number of particles N. The next N
 * lines each contains 7 floats: the first 3 are the initial position of the particle, the next 3 are the current
 * velocity of the particle and the last float is the mass of the particle.
 *
 * @param inputFile Input file.
 * @param numParticles This pointer will be updated with the number of particles found in the file.
 * @return Particle* Array containing the particles from the file.
 */
Particle *readInput(FILE *inputFile, int *numParticles) {
    int n;
    fscanf(inputFile, "%d\n", &n);
    Particle *particles = malloc(n * sizeof(Particle));
    for (int i = 0; i < n; i++) {
        fscanf(inputFile, "%f %f %f ", &particles[i].pos.x, &particles[i].pos.y, &particles[i].pos.z);
        fscanf(inputFile, "%f %f %f ", &particles[i].v.x, &particles[i].v.y, &particles[i].v.z);
        fscanf(inputFile, "%f\n", &particles[i].mass);
    }
    *numParticles = n;
    return particles;
}

/**
 * @brief Saves the state of all particles to the provided file.
 *
 * @param file File to save the particle data to.
 * @param particles The particles to save.
 * @param numParticles Number of particles to save.
 */
void saveParticles(FILE *file, Particle *particles, int numParticles) {
    fprintf(file, "%d\n", numParticles);
    for (int i = 0; i < numParticles; i++) {
        fprintf(file, "%.1f %.1f %.1f ", particles[i].pos.x, particles[i].pos.y, particles[i].pos.z);
        fprintf(file, "%.1f %.1f %.1f ", particles[i].v.x, particles[i].v.y, particles[i].v.z);
        fprintf(file, "%.1f\n", particles[i].mass);
    }
}
/**
 * @brief Start to process a segment of the array particles.
 *
 * @param arg A void pointer that point to a ThreadArgs struct.
 * @return NULL
 */
void* computeForcesThread(void *arg){
    ThreadArgs *args = (ThreadArgs *)arg;
    Particle *particles = args->particlesArray;
    int threadIndex = args->threadIndex;
    int n = args->n;
    vec3 *acc = args->acc;
    int numThreads = args->numThreads;

    int start = threadIndex * (n / numThreads);
    int end = start + (n / numThreads);
    if(numThreads != 1 && n % numThreads != 0 && (threadIndex + 1) == numThreads){
        end += n % numThreads;
    }
    // Compute all-to-all forces and accelerations
    for (int i = start; i < end; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                // Skip interaction with self
                continue;
            }
            float rx = particles[j].pos.x - particles[i].pos.x;
            float ry = particles[j].pos.y - particles[i].pos.y;
            float rz = particles[j].pos.z - particles[i].pos.z;
            float dd = rx * rx + ry * ry + rz * rz + EPS;
            float d = 1 / sqrtf(dd * dd * dd);
            float s = particles[j].mass * d;

            acc[i].x += rx * s;
            acc[i].y += ry * s;
            acc[i].z += rz * s;
        }
    }

    // Barrier Synchronization wait for all the threads to reach this point
    pthread_barrier_wait(&barrier);

    // Update positions and velocities
    for (int i = start; i < end; i++) {
        particles[i].pos.x += particles[i].v.x;
        particles[i].pos.y += particles[i].v.y;
        particles[i].pos.z += particles[i].v.z;
        particles[i].v.x += acc[i].x;
        particles[i].v.y += acc[i].y;
        particles[i].v.z += acc[i].z;
    }
    return NULL;
}
/**
 * @brief Performs a threaded N-Body simulation with the given particles for the provided number of time-steps.
 *
 * @param particles The particles to simulate.
 * @param n The total number of particles.
 * @param timeSteps The number of time-steps to simulate for.
 * @param numThreads Number of threads to use for the simulation.
 */
void startSimulationThreaded(Particle *particles, int n, int timeSteps, int numThreads) {
    // Accelerations
    vec3 *acc = malloc(n * sizeof(vec3));

    pthread_t threadsArray[numThreads]; // an array that contains all the threads
    ThreadArgs *threadArgsArray[numThreads]; // an array that contains all the threads arguments

    for (int i = 0; i < numThreads; i++) {
        threadArgsArray[i] = createThreadArgs(particles,n,-1,acc,numThreads ); // malloc once instead of many times
    }

    for (int t = 0; t < timeSteps; t++) {
        memset(acc, 0, n * sizeof(vec3));

        pthread_barrier_init(&barrier, NULL, numThreads); // set the barrier

        for (int i = 0; i < numThreads; i++){ // start the threads
            threadArgsArray[i]->threadIndex = i; // all the arguments of the threads are the same except the thread index
            pthread_create(&threadsArray[i], NULL, computeForcesThread, (void *) threadArgsArray[i]);
        }

        for (int i = 0; i < numThreads; i++) { // wait for all the threads to finish
            pthread_join(threadsArray[i], NULL);
        }
        pthread_barrier_destroy(&barrier);
    }
    for (int i = 0; i < numThreads; i++)
        free(threadArgsArray[i]);
    free(acc);

}

/**
 * Old Serial Version
 * @brief Performs a serial N-Body simulation with the given particles for the provided number of time-steps.
 *
 * @param particles The particles to simulate.
 * @param n The total number of particles.
 * @param timeSteps The number of time-steps to simulate for.
 * @param numThreads Number of threads to use for the simulation. Currently unused.
 */
void startSimulation(Particle *particles, int n, int timeSteps, int numThreads) {
    // Accelerations
    vec3 *acc = malloc(n * sizeof(vec3));

    for (int t = 0; t < timeSteps; t++) {
        memset(acc, 0, n * sizeof(vec3));
        // Compute all-to-all forces and accelerations
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) {
                    // Skip interaction with self
                    continue;
                }
                float rx = particles[j].pos.x - particles[i].pos.x;
                float ry = particles[j].pos.y - particles[i].pos.y;
                float rz = particles[j].pos.z - particles[i].pos.z;
                float dd = rx * rx + ry * ry + rz * rz + EPS;
                float d = 1 / sqrtf(dd * dd * dd);
                float s = particles[j].mass * d;
                acc[i].x += rx * s;
                acc[i].y += ry * s;
                acc[i].z += rz * s;
            }
        }
        // Update positions and velocities
        for (int i = 0; i < n; i++) {
            particles[i].pos.x += particles[i].v.x;
            particles[i].pos.y += particles[i].v.y;
            particles[i].pos.z += particles[i].v.z;
            particles[i].v.x += acc[i].x;
            particles[i].v.y += acc[i].y;
            particles[i].v.z += acc[i].z;
        }
    }
    free(acc);
}

/**
 * @brief N-Body simulation. Needs at least arguments: a number of time-steps to simulate and a number of
 * threads. Optionally, an input file can be given as final argument.
 * Usage: ./a.out <time-steps> <num-threads> [inputFile]
 * Example usage: ./a.out 5 2
 * This will read the configuration from stdin and perform the simulation for 5 time steps. In the serial
 * version, the number of threads argument is unused.
 * Basic Compilation: gcc nBody.c -lm
 * When evaluating performance: compile with additional -O3 flag
 *
 * @param argc Argument count.
 * @param argv Arguments. Two arguments are expected: a number of time-step and number of threads (in
 * that order). A third argument - an input file - is optional.
 * @return int Exit code.
 */
int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        FATAL(
                "Please provide the number of time steps to simulate and number of threads to use.\n"
                "Optionally, provide an input file to read from.\n");
    }
    int timeSteps = atoi(argv[1]);
    int numThreads = atoi(argv[2]);
    char *inputFilePath = argv[3];

    int numParticles;
    fprintf(stderr, "Reading input...\n\n");
    Particle *particles;

    if (inputFilePath == NULL) {
        particles = readInput(stdin, &numParticles);
    } else {
        FILE *inputFile = fopen(inputFilePath, "r");
        if (inputFile == NULL) {
            FATAL("Cannot open file %s\n", inputFilePath);
        }
        particles = readInput(inputFile, &numParticles);
        fclose(inputFile);
    }

    fprintf(stderr, "Starting n-body simulation using %d threads\n", numThreads);
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    startSimulationThreaded(particles, numParticles, timeSteps, numThreads);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double timeSpent = (end.tv_sec - start.tv_sec);
    timeSpent += (end.tv_nsec - start.tv_nsec) / 1e9;
    fprintf(stderr, "Simulation of %d particles complete -- Took: %f seconds.\n\n", numParticles, timeSpent);

    fprintf(stderr, "Saving particles...\n");
    saveParticles(stdout, particles, numParticles);

    free(particles);
    return EXIT_SUCCESS;
}
