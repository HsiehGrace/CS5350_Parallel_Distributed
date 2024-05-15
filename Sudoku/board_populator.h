#ifndef BOARD_POPULATOR_H
#define BOARD_POPULATOR_H

#include <iostream>
#include <fstream> 
#include <string>
#include <sstream>
#include <time.h> 
#include "sudoku_backtrack.h"
#include "variables.h"
  
// Creates a valid board of size DIMENSION. 
void random_board(int random_board[DIMENSION][DIMENSION], double sparseness) {

  srand(time(NULL)); 

  for (int i = 0; i < DIMENSION; i++) {
    for (int j = 0; j < DIMENSION; j++) {
      random_board[i][j] = 0;
    }
  }

  if (solveSudoku(random_board) != true){
    std::cout << "No solution exists";
  }

  // Populates an element with 0 according to sparsness. 
  for (int i = 0; i < DIMENSION; i++) {
    for (int j = 0; j < (int)DIMENSION * sparseness; j++) {
      
      int r = rand() % DIMENSION;
      int s = rand() % DIMENSION;
      
      while (random_board[i][r] == 0) {
        r = rand() % DIMENSION;
        s = rand() % DIMENSION;
      }

      random_board[i][r] = 0;
    }

  }
}


// Make sure global var DIMENSION matches dimension of source.txt board. 
void non_standard_board_populate(std::string file_name, int non_standard_board[DIMENSION][DIMENSION]) {
  
  std::ifstream inputFile;
  inputFile.open(file_name);
  if (!inputFile) {
    std::cerr << "Unable to open file!" << std::endl;
    return;
  }
  
  std::string line;
  int row = 0;
  int col = 0;
  while (getline(inputFile, line)) {

    std::stringstream ss(line);
    std::string temp = "";
    for (int i = 0; i < line.length(); i++) {
      
      if (isdigit(line[i])) {
        temp += line[i];
      } 
      if (line[i] == ' ' && !temp.empty()) {
        non_standard_board[row][col] = stoi(temp);
        col++;
        temp = "";
      }
      if (line[i] == '.') {
        non_standard_board[row][col] = 0;
        col++;
      }
      if (i == line.length() - 1) {
        if (!temp.empty()) {
          non_standard_board[row][col] = stoi(temp);
          temp = "";
        }
        row++;
        col = 0;
      }
    }
  }
  inputFile.close();
}


int** blank_board() {
  int** blank_board = new int*[DIMENSION];
  for (int i = 0; i < DIMENSION; i++) {
    blank_board[i] = new int[DIMENSION];
    for(int j = 0; j < DIMENSION; j++) {
      blank_board[i][j] = 0;
    }
  }
  return blank_board;
}

#endif