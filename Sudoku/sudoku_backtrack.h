#ifndef SUDOKU_BACKTRACK_H
#define SUDOKU_BACKTRACK_H

#include "variables.h"
#include <iostream>
#include <omp.h>

using namespace std;

bool SHOW_GUESSES = false;

//check whether num is present in col or not
bool isPresentInCol(int grid[DIMENSION][DIMENSION], int col, int num){ 

  for (int row = 0; row < DIMENSION; row++) {
    if (grid[row][col] == num) {
      return true;
    }
  }    
  return false;
}

// check whether num is present in row or not
bool isPresentInRow(int grid[DIMENSION][DIMENSION], int row, int num){ 

  for (int col = 0; col < DIMENSION; col++) {
    if (grid[row][col] == num) {
      return true;
    }
  }
  return false;
}

//check whether num is present in DIMENSIONxDIMENSION box or not
bool isPresentInBox(int grid[DIMENSION][DIMENSION], int boxStartRow, int boxStartCol, int num){

  for (int row = 0; row < ROOT_DIM; row++) {
    for (int col = 0; col < ROOT_DIM; col++) {
      if (grid[row+boxStartRow][col+boxStartCol] == num) {
        return true;
      }   
    }
  }

  return false;
}

// Non-parallel. get empty location and update row and column
bool findEmptyPlace(int grid[DIMENSION][DIMENSION], int &row, int &col){ 
  
  for (row = 0; row < DIMENSION; row++) {
    for (col = 0; col < DIMENSION; col++) {
      if (grid[row][col] == 0) {
        return true;
      }   
    }
  }
  
  return false;
}

// Parallelized. get empty location and update row and column
bool findEmptyPlace_par(int grid[DIMENSION][DIMENSION], int &row, int &col){
  
  int r = omp_get_thread_num();
  int p = omp_get_num_threads();
  int start_row = r * (DIMENSION / p);
  int end_row = (r + 1) * (DIMENSION / p);

  for (row = start_row; row < end_row; row++) {
    for (col = 0; col < DIMENSION; col++) {
      if (grid[row][col] == 0) {
        return true;
      }   
    }
  }
  
  return false;
}

//when item not found in col, row and current DIMENSIONxDIMENSION box
bool isValidPlace(int grid[DIMENSION][DIMENSION], int row, int col, int num){
  return !isPresentInRow(grid, row, num) && !isPresentInCol(grid, col, num) && !isPresentInBox(grid, row - row%ROOT_DIM,
  col - col%ROOT_DIM, num);
}

bool solveSudoku(int grid[DIMENSION][DIMENSION]){
  int row, col;

  if (!findEmptyPlace(grid, row, col)) {
    //when all places are filled
    return true;
  }
 
  //valid numbers are 1 - DIMENSION
  for (int num = 1; num <= DIMENSION; num++){ 
    
    //check validation, if yes, put the number in the grid
    if (isValidPlace(grid, row, col, num)) { 
      grid[row][col] = num;
      
      //recursively go for other cells in the grid
      if (solveSudoku(grid)) {
        return true;
      }
  
      //turn to unassigned space when conditions are not satisfied
      grid[row][col] = 0; 
    }


  }
  return false;
}

// 
// bool solveSudoku_par(int grid[DIMENSION][DIMENSION]){
//   int row = 0; 
//   int col = 0;

//   // if (!findEmptyPlace_par(grid, row, col)) {
//   if (!findEmptyPlace(grid, row, col)) {
//     // when all places are filled
//     return true; 
//   }
    
//   // int r = omp_get_thread_num();
//   //valid numbers are 1 - DIMENSION
//   for (int num = 1; num <= DIMENSION; num++) {

//     //check validation, if yes, put the number in the grid 
//     if (isValidPlace(grid, row, col, num)) {
      
//       #pragma omp_critical
//       {
//         grid[row][col] = num;

//         if (SHOW_GUESSES) {
//           std::cout << "Thread #" << omp_get_thread_num() << " ";
//           std::cout << "[" << row << "][" << col << "] = " << num << "\n";
//         }
//       }

//       //recursively go for other cells in the grid
//       if (solveSudoku_par(grid)) {
//         return true;
//       }

//       //turn to unassigned space when conditions are not satisfied
//       #pragma omp_critical 
//       {
//         grid[row][col] = 0; 
//       }
      
//     }
//   }
//   return false;
// }



// // 
// bool solveSudoku_par(int grid[DIMENSION][DIMENSION]) {
//   int row = 0; 
//   int col = 0;

//   if (!findEmptyPlace_par(grid, row, col)) {
//     // when all places are filled
//     return true; 
//   }
    
//   if (findEmptyPlace_par(grid, row, col)) {

//     for (int num = 1; num <= DIMENSION; num++) {

//       //check validation, if yes, put the number in the grid 
//       if (isValidPlace(grid, row, col, num)) {
        
//         #pragma omp_critical
//         {
//           grid[row][col] = num;

//           if (SHOW_GUESSES) {
//             std::cout << "Thread #" << omp_get_thread_num() << " ";
//             std::cout << "[" << row << "][" << col << "] = " << num << "\n";
//           }
//         }

//         //recursively go for other cells in the grid
//         if (solveSudoku_par(grid)) {
//           return true;
//         }

//         //turn to unassigned space when conditions are not satisfied
//         #pragma omp_critical 
//         {
//           grid[row][col] = 0; 
//         }
        
//       }
//     }
//   }

//   return false;
// }



bool is_possible(int grid[DIMENSION][DIMENSION], int row,int col, int val)
{
    // check if the column is valid
    for (int i=0;i<DIMENSION;i++)
    {
        if (grid[i][col] == val)
        {
            return false;
        }
    }
    // check if the row is valid
    for (int i=0;i<DIMENSION;i++)
    {
        if (grid[row][i] == val)
        {
            return false;
        }
    }
    // check if the 3x3 square is valid
    int startRow = ROOT_DIM*(row/ROOT_DIM); //get if it is the 0,1 or 2 3x3 square then *3
    int startCol = ROOT_DIM*(col/ROOT_DIM);

    /* alternative way
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    */
   
    for (int i = startRow; i < startRow + ROOT_DIM; i++)
    {
        for (int j = startCol; j < startCol + ROOT_DIM; j++)
        {
            if (grid[i][j] == val)
            {
                return false;
            }
        }
    }
    // if every test passed
    return true;
}



bool solve(int grid[DIMENSION][DIMENSION], int row,int col)
{

  if (col == DIMENSION)
  {
    if (row == DIMENSION-1)
    {
      return true; // we have filled the last square 
                  // and add 1 to col (sudoku solved)
    }
    // we are at the end of a column
      col = 0;
      row ++;
   }

    if (grid[row][col] != 0) // Already filled, solve for next square
    {
        return solve(grid,row,col+1);
    }

    for (int x = 1; x <= DIMENSION; x++) //try every value between 1 and 9
    { 
      if (is_possible(grid, row, col, x))
      {
        #pragma omp critical
        {
          grid[row][col] = x;
          if (SHOW_GUESSES) {
            std::cout << "Thread #" << omp_get_thread_num() << " ";
            std::cout << "[" << row << "][" << col << "] = " << x << "\n";
          }
        }
        
        if (solve(grid, row, col + 1)) // try x and check if it works
        {
          return true;
        }
           
       }
       #pragma omp critical
       {
        grid[row][col] = 0; // previous x value didn't work
                             // reset and try another x
       }

    }
  
    return false; // We are in a dead end
                  // can't solve the sudoku with current values
}


void sudokuGrid(int grid[DIMENSION][DIMENSION]){ //print the sudoku grid after solve
  
  for (int row = 0; row < DIMENSION; row++){
    for (int col = 0; col < DIMENSION; col++){
      cout << grid[row][col] <<" ";
    }
    cout << endl;
  }
}


#endif