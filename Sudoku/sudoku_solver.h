#include <iostream>
#include <omp.h>
#include <vector>
#include <cmath>
#include "sudoku_recursive.h"

typedef std::vector<int> list;

const int DEBUG = true;

struct Cell {

  // location[0] = r value; location[1] = s value;
  int location[2];
  std::vector<int> cell_mark_up;
  int unforced = cell_mark_up.size();

  void print_location() {
    std::cout << "(";
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
      std::cout << "mark up list is empty.";
      return;
    }
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
    for (int i = 0; i < DIMENSION; i++) {
      if (row_singletons[i].size() == 1) {
        int y = row_singletons[i][0].location[0];
        int x = row_singletons[i][0].location[1];
        game_board[y][x] = i;
      }
    }
  }

  // Doesn't work. 
  // Returns vector of vectors. If cell_list[i].size() == 1 then cell_list[i].location can be marked with 'i'.
  std::vector<std::vector<Cell>> find_row_singletons(int target_row) {
    std::vector<std::vector<Cell>> cell_list(DIMENSION);
    for (int i = 0; i < DIMENSION; i++) {
      Cell c = grid_mark_up[target_row][i];
      for (int j = 0; j < c.cell_mark_up.size(); j++) {
        cell_list[c.cell_mark_up[j]].push_back(c);
        
      }
    }
    return cell_list;
  }


  // Populates each cell's mark up list with the relevant values that could go in that cell. 
  void populate_mark_up_values(int game_board[DIMENSION][DIMENSION]) {

    int root_dim = sqrt(DIMENSION);
    
    // #pragma omp parallel for
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        if (game_board[i][j] == 0) {
          int rectified_row = (i / root_dim) * root_dim;
          int rectified_column = (j / root_dim) * root_dim;
          list box_vals = box_search(game_board, rectified_row, rectified_column);
          list row_vals = row_search(game_board, i);
          list column_vals = column_search(game_board, j);
          grid_mark_up[i][j].cell_mark_up = generate_cell_mark_up(row_vals, column_vals, box_vals);
        }
      }
    }
  
  }

  // One-hot row search. If results[i] == 1 then i is in row.
  list row_search(int game_board[][DIMENSION], int row) {
    list results(DIMENSION + 1, 0);
    
    // #pragma omp parallel for
    for (int i = 0; i < DIMENSION; i++) {
      results[game_board[row][i]] += 1;
      if (results[game_board[row][i]] > 1) {
        if (game_board[row][i] != 0) {
          if (DEBUG) {
            std::cout << "Sudoku has been violated.\n";
          }
        }
      }
    }
    return results;
  }

  // One-hot column search. If results[i] == 1 then i is in column.
  std::vector<int> column_search(int game_board[][DIMENSION], int column) {
    std::vector<int> results(DIMENSION + 1, 0);

    // #pragma omp parallel for
    for (int i = 0; i < DIMENSION; i++) {
      results[game_board[i][column]] += 1;

      if (results[game_board[i][column]] > 1) {
        if (game_board[i][column] != 0) {
          if (DEBUG) {
            std::cout << "Sudoku has been violated.\n";
          }
        }
      }
    }
    
    return results;
  }

  // One-hot box search. Be careful with start_row and start_column values. 
  list box_search(int game_board[DIMENSION][DIMENSION], int start_row, int start_column) {
    list results(DIMENSION + 1, 0);
    int root_dim = sqrt(DIMENSION);

    int idx = 0;
    // #pragma omp parallel for
    for (int i = start_row; i < start_row + root_dim; i++) {
      for (int j = start_column; j < start_column + root_dim; j++) {
        results[game_board[i][j]] += 1;
        if (results[game_board[i][j]] > 1) {
          if (game_board[i][j] != 0) {
            if (DEBUG) {
              std::cout << "Sudoku has been violated.\n";
            }
          }
        }
      }
    }
    
    return results;
  }

  // For i = 1 to DIMENSION; if row/colum/box[i] == 0 than i is a potential value for the cell the lists were generated for. 
  list generate_cell_mark_up(list row_vals, list column_vals, list box_vals) {
    list cell_mark_up;
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

  void create_blank_board() {
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        working_board[i][j] = 0;
      }
    }
  }

  // If there values that can be forced, this function will populate all of them. 
  void all_force_populate() {
    while (!dry) {
      populate_forced_values();
    }
  }

  // If the board has not been solved this will run the backtracking algorithm. Only works if board is sufficiently populated. 
  void solve_remaining() {
  if (!complete) {
    // #pragma omp parallel 
    // {
      solveSudoku(working_board);
    // }
      
    }
  }
  
  // Populates forced values for (1) iteration. 
  void populate_forced_values() {
  
  // #pragma omp parallel
  // {
    populate_mark_up_values();
    int total = 0;
    dry = true;
    complete = true;
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        if (working_board[i][j] == 0) {
          int list_size = mark_up_board.grid_mark_up[i][j].cell_mark_up.size();
          if (list_size == 1) {
            int value = mark_up_board.grid_mark_up[i][j].cell_mark_up[0];
            mark_up_board.grid_mark_up[i][j].cell_mark_up.pop_back();
            working_board[i][j] = value;
            dry = false;
            total_forced++;
          }
          complete = working_board[i][j] == 0 ? false : complete;
        }
      }  
    }
      
  }

  void populate_mark_up_values() {
    mark_up_board.populate_mark_up_values(working_board);
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

