#ifndef VALIDATE_H
#define VALIDATE_H

#include <iostream>
#include <unordered_set>
#include "variables.h"

// using namespace std;

bool isValidUnit(int unit[DIMENSION]) {
    unordered_set<int> seen;
    for (int i = 0; i < DIMENSION; ++i) {
        if (unit[i] != 0) {
            if (seen.find(unit[i]) != seen.end()) {
                return false;
            }
            seen.insert(unit[i]);
        }
        else {
          return false;
        }
    }
    return true;
}

bool isValidSudoku(int board[DIMENSION][DIMENSION]) {
    // Check rows
    for (int i = 0; i < DIMENSION; ++i) {
        if (!isValidUnit(board[i])) {
            return false;
        }
    }

    // Check columns
    for (int j = 0; j < DIMENSION; ++j) {
        int column[DIMENSION];
        for (int i = 0; i < DIMENSION; ++i) {
            column[i] = board[i][j];
        }
        if (!isValidUnit(column)) {
            return false;
        }
    }

    // Check 3x3 subgrids
    for (int i = 0; i < DIMENSION; i += ROOT_DIM) {
        for (int j = 0; j < DIMENSION; j += ROOT_DIM) {
            int square[DIMENSION];
            int index = 0;
            for (int x = i; x < i + ROOT_DIM; ++x) {
                for (int y = j; y < j + ROOT_DIM; ++y) {
                    square[index++] = board[x][y];
                }
            }
            if (!isValidUnit(square)) {
                return false;
            }
        }
    }

    return true;
}

#endif