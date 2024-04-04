#include "CS5350_Parallel_Distributed.h"

int main()
{
    // Check that OpenMP works for your PC
    /*
    #pragma omp parallel num_threads(4)
        {
            printf("Hello from thread %d, nthreads %d\n", omp_get_thread_num(), omp_get_num_threads());
        }
        */

    // ------ Create matricies --------------------
    Matrix matrixA = random_2D_vector(m, n); // m x n
    Matrix matrixB = random_2D_vector(n, q); // n x q
    Matrix matrixSequential;                 // Use for verify
    Matrix matrix1D;

    print_2D_vector(matrixA);
    print_2D_vector(matrixB);

    // ------ MM Algorithms -----------------------
    // Sequential
    matrixSequential = MM_sequential(matrixA, matrixB);
    print_2D_vector(matrixSequential);

    // 1D Parallel
    matrix1D = MM_1D_parallel(matrixA, matrixB);
    print_2D_vector(matrix1D);
    verify(matrix1D, matrixSequential);
    

    return 0;
}




/*
* You will implement a serial MM algorithm using three nested for loops.
* This implementation will be used to conduct the performance study.
*/
Matrix MM_sequential(Matrix matrixA, Matrix matrixB)
{
    // Initilize Vector (matrix)

    // A (m x n) * B (n x q) = C (m x q)
    int m = matrixA.size();
    int n = matrixB.size();
    int q = matrixB[0].size();
    Matrix result = random_2D_vector(m, q, 0);

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
Matrix MM_simple_parallel(Matrix matrixA, Matrix matrixB, int p_max)
{
    // A (m x n) * B (n x q) = C (m x q)
    int m = matrixA.size();
    int n = matrixB.size();
    int q = matrixB[0].size();
    Matrix result = random_2D_vector(m, q, 0);

    return result;
}

/*
* You will implement the MM-1D algorithm with the following initial data layouts.
* 
* Each processor has one row of A and one column of B and is responsible for creating one row of C where the row (j) is the same for A and C
* 
* Note that each processor 𝑃𝑖 refers to a thread in the OpenMP implementation. 
*/
Matrix MM_1D_parallel(Matrix matrixA, Matrix matrixB, const int p_max)
{
    // A (m x n) * B (n x q) = C (m x q)
    int m = matrixA.size();
    int n = matrixB.size();
    int q = matrixB[0].size();
    Matrix result(m, std::vector<int>(q));

    // Check Thread Count
    int p_count = 0;
    if (p_max < m)
    {
        omp_set_num_threads(p_max);
        p_count = p_max;
    }
    else
    {
        omp_set_num_threads(m);
        p_count = m;
    }

    std::cout << "Number of Threads: " << p_count << std::endl;


    // Start Timer
    auto start = std::chrono::high_resolution_clock::now();

    // 1D Parallel Algorithm
    #pragma omp parallel
    {
        int r = omp_get_thread_num();

        for (int i = 0; i < m / p_count; i++) {
            for (int j = 0; j < q; j++) {
                int sum = 0;
                for (int k = 0; k < n; k++) {
                    sum += matrixA[i + r * (m / p_count)][k] * matrixB[k][(j + r) % q];
                }
                result[i + r * (m / p_count)][(j + r) % q] = sum;
            }
        }
    }

    if (TIMER) {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Parallel execution time: " << duration.count() << " seconds" << std::endl;
    }
    return result;
}