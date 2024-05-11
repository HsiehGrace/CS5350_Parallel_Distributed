#include <iostream>
#include <cmath>
#include <omp.h>

const int DIMENSION = 16;
using namespace std;

const int ROOT_DIMENSION = sqrt(DIMENSION);

// 
bool isPresentInCol(int grid[DIMENSION][DIMENSION], int col, int num){ //check whether num is present in col or not
  // bool present = false;
  // #pragma omp parallel for
  for (int row = 0; row < DIMENSION; row++) {
    if (grid[row][col] == num) {
      // #pragma omp critical // Ensure atomic access to present variable
      // {
      //   present = true; // Set present to true if num is found
      // }
      return true;
    }
  }    
      
  return false;
  // return present;
}

// 
bool isPresentInRow(int grid[DIMENSION][DIMENSION], int row, int num){ //check whether num is present in row or not
  // bool present = false;
  // #pragma omp parallel for
  for (int col = 0; col < DIMENSION; col++) {
    if (grid[row][col] == num) {
      // #pragma omp critical 
      // {
      //   present = true;
      // }
      return true;
    }
  }
  return false;
  // return present;
}

//
bool isPresentInBox(int grid[DIMENSION][DIMENSION], int boxStartRow, int boxStartCol, int num){
//check whether num is present in 3x3 box or not
  // bool present = false;
  // #pragma omp parallel for
  for (int row = 0; row < ROOT_DIMENSION; row++) {
    for (int col = 0; col < ROOT_DIMENSION; col++) {
      if (grid[row+boxStartRow][col+boxStartCol] == num) {
        // #pragma omp critical 
        // {
        //   present = true;
        // }
        return true;
      }   
    }
  }

  return false;
  // return present;
}

void sudokuGrid(int grid[DIMENSION][DIMENSION]){ //print the sudoku grid after solve
  
  for (int row = 0; row < DIMENSION; row++){
    for (int col = 0; col < DIMENSION; col++){
      cout << grid[row][col] <<" ";
    }
    cout << endl;
  }
}

// 
bool findEmptyPlace(int grid[DIMENSION][DIMENSION], int &row, int &col){ //get empty location and update row and column
  
  // bool empty = false;
  // #pragma omp parallel for
  for (row = 0; row < DIMENSION; row++) {
    for (col = 0; col < DIMENSION; col++) {
      //marked with 0 is empty
      if (grid[row][col] == 0) {
        // #pragma omp critical 
        // {
          // empty = true;
        // }
        return true;
      }   
    }
  }

  // return empty;
  return false;
}


bool isValidPlace(int grid[DIMENSION][DIMENSION], int row, int col, int num){
  //when item not found in col, row and current 3x3 box
  return !isPresentInRow(grid, row, num) && !isPresentInCol(grid, col, num) && !isPresentInBox(grid, row - row%ROOT_DIMENSION ,
  col - col%ROOT_DIMENSION, num);
}

// 
bool solveSudoku(int grid[DIMENSION][DIMENSION]){
  int row, col;
  if (!findEmptyPlace(grid, row, col))
    return true; //when all places are filled
  // #pragma omp parallel for
  for (int num = 1; num <= DIMENSION; num++){ //valid numbers are 1 - 9
    if (isValidPlace(grid, row, col, num)){ //check validation, if yes, put the number in the grid
      grid[row][col] = num;
      if (solveSudoku(grid)) //recursively go for other rooms in the grid
        return true;
      grid[row][col] = 0; //turn to unassigned space when conditions are not satisfied
    }
  }
  return false;
}

