#include <stdio.h>
#include <iostream>
#include <vector>
#include "omp.h" //OpenMP

// Prototypes
// Utility functions
void print_2D_vector(std::vector<std::vector<int>> v);
void print_1D_vector(std::vector<int> v);
std::vector<std::vector<int>> random_2D_vector(int rows, int cols, int max_val = 5);

// Matrix Multiplications
std::vector<std::vector<int>> sequential_MM(std::vector<std::vector<int>> matrixA, std::vector<std::vector<int>> matrixB, int ditto);

int main()
{
    // Check that OpenMP works for your PC
    /*
    #pragma omp parallel num_threads(4)
        {
            printf("Hello from thread %d, nthreads %d\n", omp_get_thread_num(), omp_get_num_threads());
        }
        */


    // Create matricies
    int rows_A = 3;        // m
    int ditto_val = 2;      // n
    int cols_B = 2;         // q

    std::vector<std::vector<int>> matrixA = random_2D_vector(rows_A, ditto_val); // m x n
    std::vector<std::vector<int>> matrixB = random_2D_vector(ditto_val, cols_B); // n x q
    std::vector<std::vector<int>> matrixC;                                       // m x q

    // MM Algorithms
    matrixC = sequential_MM(matrixA, matrixB, ditto_val);
    print_2D_vector(matrixA);
    print_2D_vector(matrixB);
    print_2D_vector(matrixC);

    return 0;
}

// Print out all the contents of vector (v)
void print_2D_vector(std::vector<std::vector<int>> v)
{
    for (int i = 0; i < v.size(); i++)
    {
        for (int j = 0; j < v[i].size(); j++)
            std::cout << v[i][j] << " ";

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

// Print out all the contents of a nested/2D vector (v)
void print_1D_vector(std::vector<int> v)
{
    for (int i = 0; i < v.size(); i++)
        std::cout << v[i] << " ";

    std::cout << std::endl;
}

// Create a nested/2D vector with random integers from 0 to max_val - 1
std::vector<std::vector<int>> random_2D_vector(int rows, int cols, int max_val)
{
    std::vector<std::vector<int>> v;

    if (max_val == 0)
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

/*
* You will implement a serial MM algorithm using three nested for loops. 
* This implementation will be used to conduct the performance study. 
*/
std::vector<std::vector<int>> sequential_MM(std::vector<std::vector<int>> matrixA, std::vector<std::vector<int>> matrixB, int ditto)
{
    // Initilize Vector (matrix)

    // A (m x n) * B (n x q) = C (m x q)
    int answerRows = matrixA.size(), answerCols = matrixB[0].size();
    std::vector<std::vector<int>> answer = random_2D_vector( answerRows, answerCols, 1);
    
    for (int i = 0; i < answerRows; i++)
    {
        for (int j = 0; j < answerCols; j++)
        {
            int value_at_location = 0;
            for (int k = 0; k < ditto; k++)
            {
                value_at_location += matrixA[i][k] * matrixB[k][j];
            }
            answer[i][j] = value_at_location;
        }
    }
  
    return answer;
}
