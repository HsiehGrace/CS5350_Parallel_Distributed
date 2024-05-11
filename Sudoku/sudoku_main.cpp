#include <iostream>
#include <time.h> 
#include <random>
#include <fstream> 
#include <string>
#include <sstream>
#include "sudoku_solver.h"

int main() {

  srand(time(NULL)); 
  
  // Parsing source.txt to create a 25x25 game board.
  std::ifstream inputFile;
  inputFile.open("source.txt");
  if (!inputFile) {
      cerr << "Unable to open file!" << endl;
      return 1; 
  }
  int another_board[25][25];
  string line;
  int row = 0;
  int col = 0;
  while (getline(inputFile, line)) {

    std::stringstream ss(line);
    string temp = "";
    for (int i = 0; i < line.length(); i++) {
      
      if (isdigit(line[i])) {
        temp += line[i];
      } 
      if (line[i] == ' ' && !temp.empty()) {
        another_board[row][col] = stoi(temp);
        col++;
        temp = "";
      }
      if (line[i] == '.') {
        another_board[row][col] = 0;
        col++;
      }
      if (i == line.length() - 1) {
        if (!temp.empty()) {
          another_board[row][col] = stoi(temp);
          temp = "";
        }
        row++;
        col = 0;
      }
    }
      
  }
  inputFile.close();

  // Blank game board.
  int test_board[DIMENSION][DIMENSION];
  for (int i = 0; i < DIMENSION; i++) {
    for (int j = 0; j < DIMENSION; j++) {
      test_board[i][j] = 0;
    }
  }
  
  int aux_board[DIMENSION][DIMENSION];

  // Backtracking algorithm will solve a blank 16x16 but not 25x25. 
  if (solveSudoku(test_board) != true){
    std::cout << "No solution exists";
  }
    

  // Populate aux_board with values from the solved test_board.
  for (int i = 0; i < DIMENSION; i ++) {
    for (int j = 0; j < DIMENSION; j++) {
      aux_board[i][j] = test_board[i][j];
      int r = rand() % DIMENSION;
      aux_board[i][r] = 0;
      // Add more aux_board[i][r] = 0 for sparser game boards. Remove for more answer-dense gameboards. Not perfect.
      r = rand() % DIMENSION;
      aux_board[i][r] = 0;
        
    }
  }


  // // Use this for 25x25
  // Board b1(another_board);
  // Use this for 16x16.
  Board b1(aux_board);
  b1.print_working_board();
  std::cout << std::endl;
  b1.all_force_populate();
  b1.print_working_board();
  std::cout << "total forced : " << b1.total_forced << "\n";
  std::cout << std::endl;
  // Hangs for sparse or large game boards. 
  b1.solve_remaining();
  b1.print_working_board();
  std::cout << std::endl;



  return 0;
}