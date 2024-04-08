#include "main.h"


const int MAX_THREADS = 64;

int main()
{
    // VARIABLES FOR TESTING HERE -----------------
    // A = m x n, B = n x q, C = m x q
    const int m = 10;
    const int n = 8;
    const int q = 22;

    int p_count = 500000; // "Processor"/thread count

    // ------ Check Thread Count ------------------
    /*
    * Default Max is set to 64 just to be safe
    * Feel free to go way over depening on your PC
    */
    if (p_count > MAX_THREADS)
    {
        omp_set_num_threads(MAX_THREADS);
        p_count = MAX_THREADS;
    }
    //std::cout << "Number of Threads: " << p_count << std::endl;


    // ------ Create matricies --------------------
    Matrix matrixA = random_2D_vector(m, n); // m x n
    Matrix matrixB = random_2D_vector(n, q); // n x q
    Matrix matrixSequential;                 // Use for verify
    Matrix matrixParallel;                   // Result of parallel algorithms

    //print_2D_vector(matrixA);
    //print_2D_vector(matrixB);
    
    // ------ MM Algorithms -----------------------
    // Sequential
    matrixSequential = MM_sequential(matrixA, matrixB);
    std::cout << std::endl;

    // Simple Parallel
    matrixParallel = MM_simple_parallel(matrixA, matrixB, p_count);
    verify(matrixParallel, matrixSequential);
    matrixParallel.clear();

    // 1D Parallel
    matrixParallel = MM_1D_parallel(matrixA, matrixB, p_count);
    verify(matrixParallel, matrixSequential);
    matrixParallel.clear();

    // 2D Parallel

    return 0;
}


/*
* You will implement a serial MM algorithm using three nested for loops.
* This implementation will be used to conduct the performance study.
*/
Matrix MM_sequential(const Matrix matrixA, const Matrix matrixB)
{
    // Initilize Vector (matrix)

    // A (m x n) * B (n x q) = C (m x q)
    int m = matrixA.size();
    int n = matrixB.size();
    int q = matrixB[0].size();
    Matrix result(m, std::vector<int>(q));

    // Start Timer
    auto start = std::chrono::high_resolution_clock::now();

    // Serial Algorithm
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < q; j++)
        {
            int value_at_location = 0;
            for (int k = 0; k < n; k++)
            {
                value_at_location += matrixA[i][k] * matrixB[k][j];
            }
            result[i][j] = value_at_location;
        }
    }

    // Print Timer
    if (TIMER) {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Sequential execution time: " << duration.count() << " seconds" << std::endl;
    }

    // C (m x q)
    return result;
}
/*
* You will parallelize the for loops in the MM-ser implementation using relevant OpenMP directives. 
* This is different from the MM-1D and MM-2D implementation is that there is no explicit mapping of tasks and data items to individual threads.
*/
Matrix MM_simple_parallel(const Matrix matrixA, const Matrix matrixB, int p_count)
{
    // A (m x n) * B (n x q) = C (m x q)
    int m = matrixA.size();
    int n = matrixB.size();
    int q = matrixB[0].size();
    Matrix result(m, std::vector<int>(q));


    // Start Timer
    auto start = std::chrono::high_resolution_clock::now();

    // Serial Algorithm, but in parallel
    #pragma omp parallel for
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < q; j++)
        {
            int value_at_location = 0;
            for (int k = 0; k < n; k++)
            {
                value_at_location += matrixA[i][k] * matrixB[k][j];
            }
            result[i][j] = value_at_location;
        }
    }

    // Print Timer
    if (TIMER) {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Simple Parallel execution time: " << duration.count() << " seconds" << std::endl;
    }

    // C (m x q)
    return result;
}

/*
* You will implement the MM-1D algorithm with the following initial data layouts. (Check assignment)
* 
* Each processor has one row of A and is responsible for creating one row of C where the row (j) is the same for A and C
* 
* Note that each processor 𝑃𝑖 refers to a thread in the OpenMP implementation. 
*/
Matrix MM_1D_parallel(const Matrix matrixA, const Matrix matrixB, int p_count)
{
    // A (m x n) * B (n x q) = C (m x q)
    int m = matrixA.size();
    int n = matrixB.size();
    int q = matrixB[0].size();
    Matrix result(m, std::vector<int>(q, 0));

    // Check thread count
    if (p_count > m)
        p_count = m;

    // Start Timer
    auto start = std::chrono::high_resolution_clock::now();

    // 1D Parallel Algorithm
    #pragma omp parallel num_threads(p_count)
    {
        int r = omp_get_thread_num();
        // Each thread gets their row(s) of A
        // A (rn / p : (r + 1)n / p - 1, : )
        // No minus 1 to max because i < max, not i <= max                                             

        // Only do logic for this processor's values of A
        for (int i = r * m / p_count; i < (r + 1) * m / p_count; i++)
        {
            // Get all the columns from B
            for (int j = 0; j < q; j++)
            {   
                // Do calculation and set to spot in C
                int value_at_location = 0;
                for (int k = 0; k < n; k++)
                {
                    value_at_location += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = value_at_location;
            }
        }
    }

    if (TIMER) {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "1D Parallel execution time: " << duration.count() << " seconds" << std::endl;
    }
    return result;
}
