#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>

#include <time.h>
#include <chrono>
#include <omp.h> //OpenMP

#include "matrix_util.h"

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


// Matrix Multiplications
Matrix MM_sequential(Matrix matrixA, Matrix matrixB);
Matrix MM_simple_parallel(Matrix matrixA, Matrix matrixB, int p_max = MAX_THREADS);
Matrix MM_1D_parallel(Matrix matrixA, Matrix matrixB, int p_max = MAX_THREADS);