#include <iostream>
#include <omp.h>
#include <time.h> 
#include <tuple>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>

typedef std::vector<int> list;

const int DIMENSION = 9;
const int DEBUG = true;

bool row_check (int board[DIMENSION][DIMENSION]) {
  bool is_valid = true;
  for (int i = 0; i < DIMENSION; i++) {
    std::map<int, int> row_checker;
    for (int j = 0; j < DIMENSION; j++) {
      if (auto itr = row_checker.find(board[i][j]) != row_checker.end()) {
        row_checker[board[i][j]]++;
        if (row_checker[board[i][j]] > 1) {
          return false;
        }
      }
      else {
        row_checker[board[i][j]] = 1;
      }
    }
  }
  return true;
}

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

  void populate_mark_up_values(int game_board[DIMENSION][DIMENSION]) {

    int root_dim = sqrt(DIMENSION);
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

  // One-hot box search. Becareful with start_row and start_column values. 
  list box_search(int game_board[DIMENSION][DIMENSION], int start_row, int start_column) {
    list results(DIMENSION + 1, 0);
    // int flattened_box[1][DIMENSION];
    int root_dim = sqrt(DIMENSION);

    int idx = 0;
    for (int i = start_row; i < start_row + root_dim; i++) {
      for (int j = start_column; j < start_column + root_dim; j++) {
        // flattened_box[0][idx++] = game_board[i][j];
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

    // return row_search(flattened_box, 0);
    return results;
  }

  // Map of values to frequency in row. 
  std::map<int, int> row_map(int game_board[][DIMENSION], int row) {
    std::map<int, int> results;
    for (int i = 0; i < DIMENSION; i++) {
      int value = game_board[row][i];
      if (results.find(value) == results.end()) {
        results[value] = 1;
      } else {
        results[value]++;
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

  // Is this value in this box. Linear search.
  bool search_box(int value, int row, int column, int game_board[DIMENSION][DIMENSION]) {
    int root_dim = sqrt(DIMENSION);
    for (int i = row; i < row + root_dim; i++) {
      for (int j = column; j < column + root_dim; j++) {
        if (value == game_board[i][j]) {
          return true;
        }
      }
    }
    return false;
  }

  // Is this value in this row. Linear search.
  bool search_row(int value, int row, int game_board[DIMENSION][DIMENSION]) {
    for (int i = 0; i < DIMENSION; i++) {
      if (value == game_board[row][i]) {
        return true;
      }
    }
    return false;
  }

  // Is this value in this column. Linear search.
  bool search_column(int value, int column, int game_board[DIMENSION][DIMENSION]) {
    for (int i = 0; i < DIMENSION; i++) {
      if (value == game_board[i][column]) {
        return true;
      }
    }
    return false;
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


  void all_force_populate() {
    while (total_unforced > 0) {
      populate_forced_values();
    }
  }
  
  void populate_forced_values() {
    
    populate_mark_up_values();
    int total = 0;
    for (int i = 0; i < DIMENSION; i++) {
      for (int j = 0; j < DIMENSION; j++) {
        if (working_board[i][j] == 0) {
          int list_size = mark_up_board.grid_mark_up[i][j].cell_mark_up.size();
          if (list_size == 1) {
            int value = mark_up_board.grid_mark_up[i][j].cell_mark_up[0];
            mark_up_board.grid_mark_up[i][j].cell_mark_up.pop_back();
            working_board[i][j] = value;
          }
        }
        total += mark_up_board.grid_mark_up[i][j].cell_mark_up.size();
      }
    }
    total_unforced = total;
  }

  void populate_mark_up_values() {
    mark_up_board.populate_mark_up_values(working_board);
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

int main() {

  int original_board[DIMENSION][DIMENSION] = 
                        {
                          {0,0,0,2,6,0,7,0,1},
                          {6,8,0,0,7,0,0,9,0},
                          {1,9,0,0,0,4,5,0,0},
                          {8,2,0,1,0,0,0,4,0},
                          {0,0,4,6,0,2,9,0,0},
                          {0,5,0,0,0,3,0,2,8},
                          {0,0,9,3,0,0,0,7,4},
                          {0,4,0,0,5,0,0,3,6},
                          {7,0,3,0,1,8,0,0,0}
                      };

  int test_board[DIMENSION][DIMENSION] = 
                        {
                          {0,0,0,2,6,0,7,0,1},
                          {6,8,0,0,7,0,0,9,0},
                          {1,9,0,0,0,4,5,0,0},
                          {8,2,0,1,0,0,0,4,0},
                          {0,0,4,6,0,2,9,0,0},
                          {0,5,0,0,0,3,0,2,8},
                          {0,0,9,3,0,0,0,7,4},
                          {0,4,0,0,5,0,0,3,6},
                          {7,0,3,0,1,8,0,0,0}
                      };

  MarkUpBoard m1;

  Board b1(test_board);
  b1.print_working_board();
  std::cout << std::endl;
  // std::cout << std::endl;
  // b1.print_mark_up_values();
  // std::cout << b1.mark_up_board.unforced_remaining() << std::endl;
  // b1.populate_forced_values();
  // b1.print_working_board();
  // std::cout << std::endl;
  // std::cout << b1.mark_up_board.unforced_remaining() << std::endl;
  // std::cout << std::endl;
  // b1.populate_forced_values();
  // b1.print_working_board();
  // std::cout << std::endl;
  // std::cout << b1.mark_up_board.unforced_remaining() << std::endl;
  b1.all_force_populate();
  b1.print_working_board();
  std::cout << std::endl;


  // m1.populate_mark_up_values(test_board);
  // m1.print_mark_up();
  
  // // Populating forced values. 
  // for (int i = 0; i < DIMENSION; i++) {
  //   for (int j = 0; j < DIMENSION; j++) {
  //     if (test_board[i][j] == 0) {
  //       if (m1.grid_mark_up[i][j].cell_mark_up.size() == 1) {
  //         int value = m1.grid_mark_up[i][j].cell_mark_up[0];
  //         test_board[i][j] = value;
  //       }
  //     }
  //   }
  // }



  // for (int i = 0; i < 5; i++) {
  //   m1.populate_mark_up_values(test_board); 
  //   for (int i = 0; i < DIMENSION; i++) {
  //     for (int j = 0; j < DIMENSION; j++) {
  //       if (test_board[i][j] == 0) {
  //         if (m1.grid_mark_up[i][j].cell_mark_up.size() == 1) {
  //           int value = m1.grid_mark_up[i][j].cell_mark_up[0];
  //           m1.grid_mark_up[i][j].cell_mark_up.pop_back();
  //           test_board[i][j] = value;
  //         }
  //       }
  //     }
  //   }
  // }


  // m1.print_mark_up();

  // for (int i = 0; i < DIMENSION; i++) {
  //   for (int j = 0; j < DIMENSION; j++) {
  //     std::cout << test_board[i][j] << " ";
  //   }
  //   std::cout << std::endl;
  // }






  // int cell_row = 2;
  // int cell_column = 1;
  // int rectified_row = (cell_row / 3) * 3;
  // int rectified_column = (cell_column / 3) * 3;

  // list box_vals;
  // list row_vals;
  // list column_vals;

  // box_vals = m1.box_search(test_baord, rectified_row, rectified_column);
  // row_vals = m1.row_search(test_baord, cell_row);
  // column_vals = m1.column_search(test_baord, cell_column);

  // for each cell - for i from 1 to 9; if result[i] is 0 in row, column, box, than i can go in that cell.

  // list MarkUpBoard_list = m1.generate_cell_mark_up(row_vals, column_vals, box_vals);
  
  // for (auto i : MarkUpBoard_list) {
  //   std::cout << i << " ";
  // }

  // for (int i = 1; i <= 9; i++) {
  //   if (row_vals[i] == 0 && column_vals[i] == 0 && box_vals[i] == 0) {
  //     std::cout << i << " ";
  //   }
  // }

  // // General structure for linear search of rows.
  // for (int i = 0; i < 9; i++) {
  //   for (int j = 1; j <= 9; j ++) {
  //     bool value = m1.search_row(j, i, test_baord);
  //     if (value == false) {
  //       std::cout << j << " ";
  //     }
  //   }
  //   std::cout << std::endl;
  // }

  // General structure for searching every box for MarkUpBoard values. 
  // for (int i = 0; i < 9; i += 3) {
  //   for (int j = 0; j < 9; j += 3) {
  //     for (int n = 1; n <= 9; n++) {
  //       bool value = m1.search_box(n, i, j, test_baord);
  //       if (value == false) {
  //         std::cout << n << " "; 
  //       }
  //     }
  //     std::cout << std::endl;
  //   }
  //   std::cout << std::endl;
  // }
  

  return 0;
}
