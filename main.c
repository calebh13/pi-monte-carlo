#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <omp.h>
#include <time.h>
#include <assert.h>
#include <string.h>

typedef unsigned long long ull;

const char* PI_STR = "3.14159265358979323846264438327";
const int DEBUG = 0;

int is_inside_circle(double x, double y)
{
    return sqrt(pow(y - 0.5, 2) + pow(x - 0.5, 2)) < 0.5;
}

double estimate_pi(ull n, long p)
{
    /* General procedure:
     * Throw a random dart in the square from (0,0) to (1, 1). 
     * That is, generate two random numbers x and y both in the interval [0,1].
     * Then, determine if it lies in the half-unit circle embedded in the square.
     * That circle has its center at (0.5, 0.5) and radius 0.5.
     * Equivalently we must determine if the distance between the point and center of the circle
     * is less than 0.5. 
     * (Less than or equal works too - statistically, this makes no difference due to continuity).
     * Calculate the proportion of darts that land inside the circle. Multiply this by 4, and that's pi.
    */
    int seed = (int)time(NULL);
    ull points_in_circle = 0;
    
    // For rand_r, we need a unique seed per thread. So we'll use seed + tid.
    omp_set_num_threads(p);
    #pragma omp parallel reduction(+:points_in_circle)
    {
        assert(p == omp_get_num_threads());
        uint thread_seed = omp_get_thread_num() + seed;
        // todo: benchmark dynamic vs static scheduling
        #pragma omp for schedule(static)
        for(int i = 0; i < n; i++) {
            double x = (double)rand_r(&thread_seed) / RAND_MAX;
            double y = (double)rand_r(&thread_seed) / RAND_MAX;
            points_in_circle += is_inside_circle(x, y);
        }
    }

    // area of circle / area of square = pi*r^2 / l^2 = pi * (0.5)^2 / 1 = pi/4
    return (double)points_in_circle / n * 4;
}

int calculate_precision(double pi_estimate)
{
    // See how many digits are the same between our estimate and a precomputed version
    char estimate_str[64];
    snprintf(estimate_str, sizeof(estimate_str), "%.14f", pi_estimate);
    int len = strlen(estimate_str);
    int i = 0;
    for(; i < len; i++) {
        if (estimate_str[i] != PI_STR[i]) break;
    }
    if (i >= 2) i--; // don't include the '.' as an accurate digit
    return i;
}

int main(int argc, char* argv[])
{
    /* 
     * Arguments:
     * n: number of darts to throw
     * p: number of threads
     */    

    if (argc < 3) {
        printf("Usage: %s <n> <p>\nn: number of darts to throw\np: number of threads\n", argv[0]);
        return 1;
    }
    ull n = strtoull(argv[1], NULL, 10);
    long p = strtol(argv[2], NULL, 10);
    if (n == 0 || n == ULLONG_MAX) {
        printf("Invalid value for n\n");
        return 1;
    }
    if (p == 0 || p == LONG_MAX) {
        printf("Invalid value for p\n");
        return 1;
    }
    int procs = omp_get_num_procs();
    if (p > (long)procs) {
        printf("Warning: creating more threads (%ld) than available processors (%d).\n", p, procs);
        printf("This may result in significant performance degradation.\n");
    }
    
    double pi_estimate = estimate_pi(n, p);
    printf("Pi estimate: %.14f\n", pi_estimate);
    int precision = calculate_precision(pi_estimate);
    printf("Accurate digits: %d\n", precision);

    return 0;
}