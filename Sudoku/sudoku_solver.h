#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include "variables.h"
#include <iostream>
#include <omp.h>
#include <vector>
#include "sudoku_backtrack.h"

typedef std::vector<int> v_list;

const int DEBUG = true;

struct Cell {

  // location[0] = r value; location[1] = s value;
  int location[2];
  std::vector<int> cell_mark_up;
  int unforced = cell_mark_up.size();

  void print_location() {
    std::cout << "\n(";
    for (int i = 0; i < 2; i++) {
      std::cout << location[i];
      if (i == 0) {
        std::cout << ", ";    
      }
    }
    std::cout << ")";
  }

  void print_cell_mark_up() {
    if (cell_mark_up.empty()) {
      print_location();
      std::cout << "mark up v_list is empty.";
      return;
    }
    print_location();
    std::cout << "[";
    for (int i = 0; i < cell_mark_up.size(); i++) {
      std::cout << cell_mark_up[i];
      if (i < cell_mark_up.size()-1) {
        std::cout << ",";
      }
    }
    std::cout << "]";
  }
};


struct MarkUpBoard {
  Cell grid_mark_up[DIMENSION][DIMENSION];
  int board_unforced = 0;

  void show_all_mark_ups() {
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        grid_mark_up[i][j].print_cell_mark_up();
      }
    }
  }
  
  // Not used/needed?
  int unforced_remaining() {
    int total = 0;
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        total += grid_mark_up[i][j].cell_mark_up.size();
      }
    }
    board_unforced = total;
    return board_unforced;
  }


  MarkUpBoard() {
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        grid_mark_up[i][j].location[0] = i;
        grid_mark_up[i][j].location[1] = j;
      }
    }
  }
  

  // Doesn't work. 
  void populate_row_singleton(int game_board[DIMENSION][DIMENSION], int target_row) {
    std::vector<std::vector<Cell>> row_singletons = find_row_singletons(target_row);
    
    for (int i = 0; i < row_singletons.size(); i++) {
      // std::cout << "row_singletons[i].size()" << row_singletons[i].size() << "\n";
      // for (int i = 0; i < row_singletons[i].size(); i++) {
        // std::cout << row_singletons[i].size() << "\n";
        if (row_singletons[i].size() == 1) {
          int y = row_singletons[i][0].location[0];
          int x = row_singletons[i][0].location[1];
          game_board[y][x] = i;
          // std::cout << " game_board[" << y << "][" << x << "] = " << i << "\n";
        // }
      }
    }
  }

  // Doesn't work. 
  // Returns vector of vectors. If cell_v_list[i].size() == 1 then cell_v_list[i].location can be marked with 'i'.
  std::vector<std::vector<Cell>> find_row_singletons(int target_row) {
    std::vector<std::vector<Cell>> cell_v_list(DIMENSION);
    for (int i = 0; i < DIMENSION; i++) {
      Cell c = grid_mark_up[target_row][i];
      for (int j = 0; j < c.cell_mark_up.size(); j++) {
        cell_v_list[c.cell_mark_up[j]].push_back(c);
      }
    }
    
    return cell_v_list;
  }


  // Populates each cell's mark up v_list with the relevant values that could go in that cell. 
  void populate_mark_up_values(int game_board[DIMENSION][DIMENSION]) {
    
    // #pragma omp parallel for
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        if (game_board[i][j] == 0) {
          int rectified_row = (i / ROOT_DIM) * ROOT_DIM;
          int rectified_column = (j / ROOT_DIM) * ROOT_DIM;
          v_list box_vals = box_search(game_board, rectified_row, rectified_column);
          v_list row_vals = row_search(game_board, i);
          v_list column_vals = column_search(game_board, j);
          grid_mark_up[i][j].cell_mark_up = generate_cell_mark_up(row_vals, column_vals, box_vals);
        }
      }
    }
  }

    // Populates each cell's mark up v_list with the relevant values that could go in that cell. 
  void populate_mark_up_values_par(int game_board[DIMENSION][DIMENSION]) {
    
    // #pragma omp parallel for
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        if (game_board[i][j] == 0) {
          int rectified_row = (i / ROOT_DIM) * ROOT_DIM;
          int rectified_column = (j / ROOT_DIM) * ROOT_DIM;

          v_list box_vals;
          v_list row_vals;
          v_list column_vals;

          // #pragma omp parallel sections
          // {
            // #pragma omp section
            // {
              box_vals = box_search(game_board, rectified_row, rectified_column);
            // }

            // #pragma omp section
            // {
              row_vals = row_search(game_board, i);
            // }
            
            // #pragma omp section
            // {
              column_vals = column_search(game_board, j);
            // }
            
          // }

          grid_mark_up[i][j].cell_mark_up = generate_cell_mark_up(row_vals, column_vals, box_vals);
        }
      }
    }
  }




  // One-hot row search. If results[i] == 1 then i is in row.
  v_list row_search(int game_board[][DIMENSION], int row) {
    v_list results(DIMENSION + 1, 0);
    
    // #pragma omp parallel for
    for (int i = 0; i < DIMENSION; i++) {
      results[game_board[row][i]] += 1;
      if (results[game_board[row][i]] > 1) {
        if (game_board[row][i] != 0) {
          if (DEBUG) {
            std::cout << "Sudoku violated at row " << row << " : " << game_board[row][i] << ".\n";
          }
        }
      }
    }
    return results;
  }

  // One-hot column search. If results[i] == 1 then i is in column.
  std::vector<int> column_search(int game_board[][DIMENSION], int column) {
    std::vector<int> results(DIMENSION + 1, 0);
    for (int i = 0; i < DIMENSION; i++) {
      results[game_board[i][column]] += 1;

      if (results[game_board[i][column]] > 1) {
        if (game_board[i][column] != 0) {
          if (DEBUG) {
            std::cout << "Sudoku violated at column " << column << " : " << game_board[i][column] << ".\n";
          }
        }
      }
    }
    
    return results;
  }

  // One-hot box search. Be careful with start_row and start_column values. 
  v_list box_search(int game_board[DIMENSION][DIMENSION], int start_row, int start_column) {
    v_list results(DIMENSION + 1, 0);
    
    int idx = 0;
    // #pragma omp parallel for
    for (int i = start_row; i < start_row + ROOT_DIM; i++) {
      for (int j = start_column; j < start_column + ROOT_DIM; j++) {
        results[game_board[i][j]] += 1;
        if (results[game_board[i][j]] > 1) {
          if (game_board[i][j] != 0) {
            if (DEBUG) {
              std::cout << "Sudoku violated at box " << i << ", " << j << " : " << game_board[i][j] << ".\n";
            }
          }
        }
      }
    }
    
    return results;
  }

  // For i = 1 to DIMENSION; if row/colum/box[i] == 0 than i is a potential value for the cell the v_lists were generated for. 
  v_list generate_cell_mark_up(v_list row_vals, v_list column_vals, v_list box_vals) {
    v_list cell_mark_up;
    // #pragma omp parallel for
    for (int i = 1; i <= DIMENSION; i++) {
      if (row_vals[i] == 0 && column_vals[i] == 0 && box_vals[i] == 0) {
        cell_mark_up.push_back(i);
      }
    }
    return cell_mark_up;
  }

  void print_mark_up() {
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        grid_mark_up[i][j].print_location();
        std::cout << " ";
        grid_mark_up[i][j].print_cell_mark_up();
        std::cout << std::endl;
      }
    }
  }
};

struct Board {

  int original_board[DIMENSION][DIMENSION];
  int working_board[DIMENSION][DIMENSION];
  MarkUpBoard mark_up_board;
  int total_unforced = DIMENSION * DIMENSION;
  bool dry = false;
  bool complete = false;
  int total_forced = 0;
  

  Board() {
    create_blank_board(); 
  }

  Board(int game_board[DIMENSION][DIMENSION]) {
    init_working_board(game_board);
    populate_mark_up_values();
  }

  void init_working_board(int game_board[DIMENSION][DIMENSION]) {
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        working_board[i][j] = game_board[i][j];
      }
    }
  }

  void pop_singletone() {
    for (int i = 0; i < DIMENSION; i ++) {
      mark_up_board.populate_row_singleton(working_board, i);
    } 
  }

  void create_blank_board() {
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        working_board[i][j] = 0;
      }
    }
  }

  // If there are values that can be forced, this function will populate all of them. 
  void all_force_populate() {
    while (!dry) {
      populate_forced_values();
    }
  }

  void efficient_parallel() {

    while (!dry) {
      populate_forced_values_par();
    }
    solve_remaining();
  }

  // Populates forced values for (1) iteration. 
  void populate_forced_values_par() {

    populate_mark_up_values_par();
    // populate_mark_up_values();
    int total = 0;
    dry = true;
    complete = true;

    // std::cout << "here\n";
    #pragma omp parallel for
    for (int i = 0; i < DIMENSION; i++) {
      #pragma omp parallel for
      for (int j = 0; j < DIMENSION; j++) {
        if (working_board[i][j] == 0) {
          int v_list_size = mark_up_board.grid_mark_up[i][j].cell_mark_up.size();
          if (v_list_size == 1) {
            int value = mark_up_board.grid_mark_up[i][j].cell_mark_up[0];
            
            #pragma omp critical 
            {
              mark_up_board.grid_mark_up[i][j].cell_mark_up.pop_back();
              working_board[i][j] = value;
              dry = false;
              total_forced++;
            }
          }
        }

        #pragma omp critical 
        {
          complete = working_board[i][j] == 0 ? false : complete;
        }

      }  
    }

  }

  // If the board has not been solved this will run the backtracking algorithm. Trouble with 25x25.
  void solve_remaining() {
    if (!complete) {
      solveSudoku(working_board);          
    }
  }

  // If the board has not been solved this will run the backtracking algorithm. Trouble with 25x25.
  void solve_remaining_par() {

    if (!complete) {
      
      omp_set_num_threads(THREAD_COUNT);

      #pragma omp parallel 
      {
        solve(working_board, 0, 0);
      } 
    }
  }
  
  // Populates forced values for (1) iteration. 
  void populate_forced_values() {

    populate_mark_up_values();
    int total = 0;
    dry = true;
    complete = true;
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        if (working_board[i][j] == 0) {
          int v_list_size = mark_up_board.grid_mark_up[i][j].cell_mark_up.size();
          if (v_list_size == 1) {
            int value = mark_up_board.grid_mark_up[i][j].cell_mark_up[0];
            mark_up_board.grid_mark_up[i][j].cell_mark_up.pop_back();
            working_board[i][j] = value;
            dry = false;
            total_forced++;
          }
        }
        complete = working_board[i][j] == 0 ? false : complete;
      }  
    }
  }

  void populate_mark_up_values() {
    mark_up_board.populate_mark_up_values(working_board);
  }

  void populate_mark_up_values_par() {
    mark_up_board.populate_mark_up_values_par(working_board);
  }

  // WIP
  void populate_row_singleton(int target_row) {
    mark_up_board.populate_row_singleton(working_board, target_row);
  }

  void print_mark_up_values() {
    mark_up_board.print_mark_up();
  }

  void print_working_board() {
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        std::cout << working_board[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }

  void print_original_board() {
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        std::cout << original_board[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }

};

#endif