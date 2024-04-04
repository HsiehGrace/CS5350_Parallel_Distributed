#include <stdio.h>
#include <iostream>
#include <vector>

#include <time.h>
#include <chrono>
#include <omp.h> //OpenMP

// Nested Integer Vectors -> Dynamic 2D Array
typedef std::vector<std::vector<int>> Matrix;

// Global Variables
// Functional Constants
const bool TIMER = true;
const bool DEBUG = false;

// A = m x n, B = n x q, C = m x q
const int m = 16;
const int n = 16;
const int q = 16;
const int MAX_THREADS = 64; // Depends on your computer
const int MAX_VAL = 3;      // For the random vectors

// Prototypes
// Utility functions
void print_2D_vector(Matrix v);
void print_1D_vector(std::vector<int> v);
void verify(Matrix v, Matrix control);
Matrix random_2D_vector(int rows, int cols, int max_val = MAX_VAL);
Matrix identity_matrix(int rows, int cols);

// Matrix Multiplications
Matrix MM_sequential(Matrix matrixA, Matrix matrixB);
Matrix MM_1D_Parallel(Matrix matrixA, Matrix matrixB, int p_max = MAX_THREADS);


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
    matrix1D = MM_1D_Parallel(matrixA, matrixB);
    print_2D_vector(matrix1D);
    verify(matrix1D, matrixSequential);
    

    return 0;
}

// Print out all the contents of the nested/2D vector (v)
void print_2D_vector(Matrix v)
{
    for (int i = 0; i < v.size(); i++)
    {
        for (int j = 0; j < v[i].size(); j++)
            std::cout << v[i][j] << " ";

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

// Print out all the contents of vector (v)
void print_1D_vector(std::vector<int> v)
{
    for (int i = 0; i < v.size(); i++)
        std::cout << v[i] << " ";

    std::cout << std::endl;
}

void verify(Matrix v, Matrix control)
{
    int m = v.size(), n = v[0].size();

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (v[i][j] != control[i][j])
            {
                std::cout << "Failed at [" << i << "][" << j << "]" << std::endl;
                std::cout << "Matrix Answer: " << v[i][j] << "; Control Answer: " << control[i][j] << std::endl;
                return;
            }
        }
    }
    std::cout << "Test complete. No errors found." << std::endl;
}

/*
* Create a nested/2D vector with random integers from 0 to max_val - 1
* Default maximum value is 5
*/
Matrix random_2D_vector(int rows, int cols, int max_val)
{
    Matrix v;

    if (max_val < 1)
        max_val = 1;

    for (int i = 0; i < rows; i++)
    {
        v.push_back({});
        for (int j = 0; j < cols; j++)
        {
            v[i].push_back(rand() % max_val);
        }
    }

    return v;
}


// Identity Matrix Creator
Matrix identity_matrix(int rows, int cols)
{
    Matrix result(rows, std::vector<int>(cols, 0));

    for (int i = 0; i < m; i++) {
        result[i][i] = 1;
    }

    return result;
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

Matrix MM_1D_Parallel(Matrix matrixA, Matrix matrixB, const int p_max)
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