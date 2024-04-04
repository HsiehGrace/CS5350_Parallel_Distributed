#include "matrix_util.h"

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

                std::cout << "Answer" << std::endl;
                print_2D_vector(v);

                std::cout << "Control" << std::endl;
                print_2D_vector(control);

                std::cout << "Matrix Answer: " << v[i][j] << "; Control Answer: " << control[i][j] << std::endl << std::endl;
                return;
            }
        }
    }
    std::cout << "Test complete. No errors found." << std::endl << std:: endl;
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

    for (int i = 0; i < rows; i++) {
        result[i][i] = 1;
    }

    return result;
}