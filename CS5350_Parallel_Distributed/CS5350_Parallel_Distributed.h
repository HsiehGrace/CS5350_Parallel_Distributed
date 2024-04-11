#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>

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


// Matrix Multiplications
Matrix MM_sequential(const Matrix matrixA, const Matrix matrixB);
Matrix MM_simple_parallel(const Matrix matrixA, const Matrix matrixB, int p_count);
Matrix MM_1D_parallel(const Matrix matrixA, const Matrix matrixB, int p_count);
