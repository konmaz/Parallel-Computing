#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define LENGTH 12
#define THREAD_COUNT 4

int vector[LENGTH];

void merge(int low, int mid, int high)
{
    int *left = malloc(sizeof(*left) * (mid - low + 1));
    int *right = malloc(sizeof(*right) * (high - mid));
    int length_left = mid - low + 1, length_right = high - mid;

    for (int i = 0; i < length_left; i++)
        left[i] = vector[i + low];
    for (int i = 0; i < length_right; i++)
        right[i] = vector[i + mid + 1];

    int i = 0, j = 0, k = low;
    while (i < length_left && j < length_right)
    {
        if (left[i] <= right[j])
            vector[k++] = left[i++];
        else
            vector[k++] = right[j++];
    }

    while (i < length_left)
    {
        vector[k++] = left[i++];
    }

    while (j < length_right)
    {
        vector[k++] = right[j++];
    }
}

void merge_sort(int low, int high)
{
    int mid = low + (high - low) / 2;
    if (low < high)
    {
        merge_sort(low, mid);
        merge_sort(mid + 1, high);
        merge(low, mid, high);
    }
}

void *merge_sort_thread(void *arg)
{
    unsigned long thread_part = (unsigned long)arg;
    int low = thread_part * (LENGTH / THREAD_COUNT);
    int high = (thread_part + 1) * (LENGTH / THREAD_COUNT) - 1;

    if(LENGTH%THREAD_COUNT != 0 && (thread_part+1) == THREAD_COUNT){
        high += LENGTH % THREAD_COUNT;
    }
    int mid = low + (high - low) /2;
    printf("%ld, %d %d\n",thread_part ,low,high);
    if (low < high)
    {
        merge_sort(low, mid);
        merge_sort(mid + 1, high);
        merge(low, mid, high);
    }
    return 0;
}

int main()
{
    pthread_t threads[THREAD_COUNT];
    printf("[ ");
    for (int i = 0; i < LENGTH; i++){
        vector[i] = LENGTH-i;
        printf("%d,", vector[i]);
        }
    printf("]\n");

    for (int i = 0; i < THREAD_COUNT; i++)
        pthread_create(&threads[i], NULL, merge_sort_thread, (void *) i);
    for (int i = 0; i < THREAD_COUNT; i++)
        pthread_join(threads[i], NULL);

    merge(0, (LENGTH / 2 - 1) / 2, LENGTH / 2 - 1);                            // a
    merge(LENGTH / 2, LENGTH / 2 + (LENGTH - 1 - LENGTH / 2) / 2, LENGTH - 1); // b
    merge(0, (LENGTH - 1) / 2, LENGTH - 1);                                    // c

    for (int i = 0; i < THREAD_COUNT; i++)
        pthread_join(threads[i], NULL);
    printf("---------------------------\n\n\n\n");
    printf("[ ");
    for (int i = 0; i < LENGTH; i++){
        printf("%d,", vector[i]);
        }
    printf("]\n");

    return 0;
}
