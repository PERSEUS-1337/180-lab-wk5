// CMSC 180 Lab 03
// Author: Aron Resty Ramillano | 2020-01721
// Section: T3L

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h> //for threads
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>

#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define NUM_CORES 12

// Resources:
// https://www.geeksforgeeks.org/multidimensional-arrays-c-cpp/
// https://www.geeksforgeeks.org/dynamically-allocate-2d-array-c/
// https://stackoverflow.com/questions/5201708/how-to-return-a-2d-array-from-a-function-in-c
// https://stackoverflow.com/questions/2150291/how-do-i-measure-a-time-interval-in-c

typedef struct
{
    float **M;
    int n;
    int start;
    int end;
    int core_id;
} ThreadArgs;

/**
 * It creates a matrix of size n x n
 *
 * @param n the number of rows and columns in the matrix
 *
 * @return A pointer to a pointer to a float.
 */
float **createMatx(int n)
{
    float *values = malloc(n * sizeof(float));
    float **rows = malloc(n * sizeof(float *));
    for (int i = 0; i < n; ++i)
    {
        rows[i] = malloc(sizeof(float) * n);
    }
    return rows;
}

/**
 * It frees the memory allocated for the matrix.
 *
 * @param matx The matrix to be destroyed.
 */
void destroyMatx(float **matx)
{
    free(*matx);
    free(matx);
}

int getMin(int n)
{
    int min = 0;

    if (n > 1)
        min = floor((n - 1) / 10) * 10;

    return min;
}

int getMax(int n)
{
    int max = 10;

    if (n > 0)
        max = ceil(n / 10.0) * 10;

    return max;
}

/**
 * It prints a matrix of size n x n
 *
 * @param matx The matrix to be printed
 * @param n the number of rows and columns in the matrix
 */
void printMatx(float **matx, int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%.3f ", matx[i][j]);
        }
        printf("\n");
    }
}

/**
 * This function populates the matrix with random numbers. It will only populate on coordinates that is divisible by 10, including 0,0
 *
 * @param matx the matrix to be populated
 * @param n the size of the matrix
 */
void populateMatx(float **matx, int n)
{
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if ((i % 10 == 0) && (j % 10 == 0))
                matx[i][j] = (rand() % (1000));
}

/**
 * It takes a 2D array of floats, and for each element in the array, it calculates the average
 * elevation of the surrounding elements
 *
 * @param arg The argument passed to the thread function.
 *
 * @return a void pointer.
 */
void *thread_func(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    const pthread_t pid = pthread_self();
    const int core_id = args->core_id;

    // cpu_set_t: This data set is a bitset where each bit represents a CPU.
    cpu_set_t cpuset;
    // CPU_ZERO: This macro initializes the CPU set set to be the empty set.
    CPU_ZERO(&cpuset);
    // CPU_SET: This macro adds cpu to the CPU set set.
    CPU_SET(core_id, &cpuset);

    pthread_setaffinity_np(pid, sizeof(cpu_set_t), &cpuset);

    for (int i = args->start; i < args->end; i++)
    {
        int min_x = getMin(i);
        int max_x = getMax(i);
        for (int j = 0; j < args->n; j++)
        {
            if (!((i % 10 == 0) && (j % 10 == 0)))
            {
                int min_y = getMin(j);
                int max_y = getMax(j);

                int area_d = (abs(min_x - i) * abs(min_y - j));
                int area_c = (abs(max_x - i) * abs(min_y - j));
                int area_b = (abs(min_x - i) * abs(max_y - j));
                int area_a = (abs(max_x - i) * abs(max_y - j));

                float elev_a = args->M[min_x][min_y];
                float elev_b = args->M[max_x][min_y];
                float elev_c = args->M[min_x][max_y];
                float elev_d = args->M[max_x][max_y];

                float elevation = ((area_a * elev_a) + (area_b * elev_b) + (area_c * elev_c) + (area_d * elev_d)) / (float)(area_a + area_b + area_c + area_d);

                args->M[i][j] = elevation;
                // printf("Boundary: %i %i | Coords: %i %i | Val: %f\n", args->start, args->end, i, j, elevation);
            }
        }
    }

    return NULL;
}

/**
 * It creates a thread for each submatrix that we are going to create
 *
 * @param M The matrix that we are going to be iterating through
 * @param n The size of the matrix
 * @param num_threads The number of threads that will be created
 */
void terrain_inter(float **M, int n, int num_threads)
{
    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    // This is basically the submatrices that we are going to make.
    int chunk_size = n / num_threads;

    for (int i = 0; i < num_threads; i++)
    {
        args[i].M = M;
        args[i].n = n;
        args[i].start = i * chunk_size;
        args[i].end = (i + 1) * chunk_size;
        args[i].core_id = (i%NUM_CORES);

        // Because the matrix should include the 0th coordinate, we would have to adjust our calculation to have the last submatrix to be able to handle a second row.
        if (num_threads > 1 && i == num_threads - 1)
        {
            args[i].end++;
        }

        // Actual implementation of creating a thread
        // We pass the arguments stated above on where on the matrix they should start
        pthread_create(&threads[i], NULL, thread_func, &args[i]);
    }

    // This will join the threads if they are finished in their operation, therefore finishing this terrain_inter function
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

/**
 * It creates a matrix of size n, populates it with random values, and then calls the terrain_inter
 * function.
 * This benchmark is designed to output the necessary information needed for the exercise lab report
 *
 * @param n The size of the matrix.
 */
void run_benchmark(int n, int t)
{
    srand(time(NULL));
    n++;

    float **matx = createMatx(n);
    populateMatx(matx, n);

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // The actual interpolation
    terrain_inter(matx, n, t);

    gettimeofday(&end_time, NULL);
    long seconds = end_time.tv_sec - start_time.tv_sec;
    long micros = ((seconds * 1000000) + end_time.tv_usec) - (start_time.tv_usec);
    double elapsed_time = (double)micros / 1000000.0;

    // Printing the matrix in a clean way
    // printMatx(matx, n);

    printf("Threads: %i, Elapsed time: %.5f seconds\n", t, elapsed_time);

    destroyMatx(matx);
}

int main()
{
    // // This first part of the code is for using the code with inputs
    // int n = 0, t = 0;
    // printf("Enter n divisible by 10: ");
    // int check = scanf("%d", &n);

    // while (!check || n <= 9 || n % 10 != 0)
    // {
    //     printf("Wrong Input! Try again: ");
    //     check = scanf("%d", &n);
    // }

    // printf("Enter t that can divide n: ");
    // check = scanf("%d", &t);

    // while (!check || n % t != 0 || t < 1)
    // {
    //     printf("Wrong Input! Try again: ");
    //     check = scanf("%d", &t);
    // }
    // // +1 to include the actual 10th coordinate
    // n++;

    // // Reinitializes the randomizer for C
    // srand(time(NULL));

    // float **matx = createMatx(n);
    // populateMatx(matx, n);

    // // This time measurement is different from  my first 2 implementations, but fundamentally results the same numbers. I have tested the times that they measure on both my non-threaded with the original clock function, and this new threaded program with a new time function and they are within margins.
    // // The reason for this is that for the first clock measurement implementation, it measures also the CPU Clock Cycles that the threeds have used in the smaller amount of time, resulting in inconsistencies of data output.
    // struct timeval start_time, end_time;
    // gettimeofday(&start_time, NULL);

    // // The actual interpolation
    // terrain_inter(matx, n, t);

    // gettimeofday(&end_time, NULL);
    // long seconds = end_time.tv_sec - start_time.tv_sec;
    // long micros = ((seconds * 1000000) + end_time.tv_usec) - (start_time.tv_usec);
    // double elapsed_time = (double)micros / 1000000.0;

    // // Printing the matrix in a clean way
    // // printMatx(matx, n);

    // printf("\nElapsed time: %.5f seconds\n", elapsed_time);

    // destroyMatx(matx);

    // ==============================================================
    // This part of the code is for the benchmarking for lab03
    // printf("Calculating an 8000x8000 matrix\n");
    // for (int i = 0; i < 3; i++)
    // {
        // printf("Run # %d\n", i+1);
        // run_benchmark(10, 1);
        // run_benchmark(10, 2);
        // run_benchmark(10, 4);
        // run_benchmark(10, 8);
        // run_benchmark(10, 16);
        // run_benchmark(10, 32);
        // run_benchmark(10, 64);
        // run_benchmark(1000, 1);
        // run_benchmark(1000, 2);
        // run_benchmark(1000, 4);
        // run_benchmark(1000, 8);
        // run_benchmark(1000, 16);
        // run_benchmark(1000, 32);
        // run_benchmark(1000, 64);
        run_benchmark(8000, 1);
        run_benchmark(8000, 2);
        run_benchmark(8000, 4);
        run_benchmark(8000, 8);
        run_benchmark(8000, 16);
        run_benchmark(8000, 32);
        run_benchmark(8000, 64);
        // run_benchmark(16000, 1);
        // run_benchmark(16000, 2);
        // run_benchmark(16000, 4);
        // run_benchmark(16000, 8);
        // run_benchmark(16000, 16);
        // run_benchmark(16000, 32);
        // run_benchmark(16000, 64);
        // run_benchmark(20000, 1);
        // run_benchmark(20000, 2);
        // run_benchmark(20000, 4);
        // run_benchmark(20000, 8);
        // run_benchmark(20000, 16);
        // run_benchmark(20000, 32);
        // run_benchmark(20000, 64);
        // run_benchmark(50000, 1);
        // run_benchmark(50000, 2);
        // run_benchmark(50000, 4);
        // run_benchmark(50000, 8);
        // run_benchmark(50000, 16);
        // run_benchmark(50000, 32);
        // run_benchmark(50000, 64);
        // run_benchmark(100000, 1);
        // run_benchmark(100000, 2);
        // run_benchmark(100000, 4);
        // run_benchmark(100000, 8);
        // run_benchmark(100000, 16);
        // run_benchmark(100000, 32);
        // run_benchmark(100000, 64);
        //     printf("Sleep for 10 seconds\n");
        //     sleep(10);
        // }

        return 0;
}