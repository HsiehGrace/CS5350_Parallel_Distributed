#pragma once
#include "CS5350_Parallel_Distributed.h"

typedef std::vector<std::vector<int>> Matrix;

// Global Variables
const int MAX_MATRIX_VAL = 10;

// Prototypes
// Utility functions
void print_2D_vector(Matrix v);
void print_1D_vector(std::vector<int> v);
void verify(Matrix v, Matrix control);
Matrix random_2D_vector(int rows, int cols, int max_val = MAX_MATRIX_VAL);
Matrix identity_matrix(int rows, int cols);
