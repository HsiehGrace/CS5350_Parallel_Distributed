#include "variables.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include "sudoku_solver.h"
#include "board_populator.h"
#include "logging.h"
#include "validate.h"

int main(int argc, char* argv[]) {
  
  double sparse = .5;
  if (argc == 2) {
    sparse = stod(argv[1]);
  }
  
  auto start = chrono::high_resolution_clock::now();
  auto end = chrono::high_resolution_clock::now();
  bool result;
  int sixteen_board[DIMENSION][DIMENSION];
  log_headers("single_brute_force.txt");
  log_headers("parallel_brute_force.txt");
  log_headers("single_efficient.txt");
  log_headers("parallel_efficient.txt");


  // // Non-parallel brute force
  // // sparse = 0.0;
  for (int i = 0; i < 100; i++) {
    random_board(sixteen_board, sparse);
    Board b1(sixteen_board);
    start = chrono::high_resolution_clock::now();
    b1.solve_remaining();
    end = chrono::high_resolution_clock::now();
    result = isValidSudoku(b1.working_board);
    log_results("single_brute_force.txt", sparse, start, end, result);
  //   // sparse += .01;
  }
    

  
  // // Parallel brute force algorithm. 
  // sparse = .5;
  random_board(sixteen_board, sparse);
  Board b2(sixteen_board);
  start = chrono::high_resolution_clock::now();
  b2.solve_remaining_par();
  end = chrono::high_resolution_clock::now();
  result = isValidSudoku(b2.working_board);
  log_results("parallel_brute_force.txt", sparse, start, end, result);

  
  // sparse = 0.0;
  // Non-parallel efficient algorithm. 
  for (int i = 0; i < 100; i++) {
    random_board(sixteen_board, sparse);
    Board b3(sixteen_board);
    start = chrono::high_resolution_clock::now();
    b3.all_force_populate();
    b3.solve_remaining();
    end = chrono::high_resolution_clock::now();
    result = isValidSudoku(b3.working_board);
    log_results("single_efficient.txt", sparse, start, end, result);
    // sparse += .01;
  }

  // sparse += .0;
  for (int i = 0; i < 50; i++) {
    // Parallel efficient
    random_board(sixteen_board, sparse);
    Board b4(sixteen_board);
    
    start = chrono::high_resolution_clock::now();
    b4.efficient_parallel();
    end = chrono::high_resolution_clock::now();
    result = isValidSudoku(b4.working_board);
    log_results("parallel_efficient.txt", sparse, start, end, result);
    // sparse += .01;
  }


  return 0;
}