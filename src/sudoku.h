#pragma once

#include <vector>
#include <algorithm>
#include <numeric>
#include <random>

enum class State
{
    Start, Empty, Valid, Unvalid
};

bool isValid(std::vector<std::vector<int>>& grid, int row, int col, int num) {
    // Check row
    for (int i = 0; i < 9; ++i) {
        if (grid[row][i] == num) return false;
    }
    // Check column
    for (int i = 0; i < 9; ++i) {
        if (grid[i][col] == num) return false;
    }
    // Check subgrid
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (grid[i + startRow][j + startCol] == num) return false;
        }
    }
    return true;
}
// Function to solve Sudoku recursively
bool solveSudoku(std::vector<std::vector<int>>& grid) {
    for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
            if (grid[row][col] == 0) {
                for (int num = 1; num <= 9; ++num) {
                    if (isValid(grid, row, col, num)) {
                        grid[row][col] = num;
                        if (solveSudoku(grid)) {
                            return true;
                        }
                        grid[row][col] = 0; // Backtrack
                    }
                }
                return false;
            }
        }
    }
    return true;
}

// Function to shuffle rows and columns
void shuffle(std::vector<std::vector<int>>& grid) {
    std::random_device rd;
    std::mt19937 g(rd());
    // Shuffle rows within each block
    for (int i = 0; i < 9; i += 3) {
        std::shuffle(grid.begin() + i, grid.begin() + i + 3, g);
    }
    // Transpose the grid
    for (int i = 0; i < 9; ++i) {
        for (int j = i + 1; j < 9; ++j) {
            std::swap(grid[i][j], grid[j][i]);
        }
    }
    // Shuffle columns within each block
    for (int i = 0; i < 9; i += 3) {
        std::shuffle(grid.begin() + i, grid.begin() + i + 3, g);
    }
}

// Function to generate a random Sudoku puzzle
void generateSudoku(std::vector<std::vector<int>>& grid) {
    // Initialize grid with zeros
    grid.assign(9, std::vector<int>(9, 0));
    // Solve the Sudoku
    solveSudoku(grid);
    // Shuffle rows, columns, and numbers
    shuffle(grid);
    // Remove numbers to create a puzzle
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<int> indices(81);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), gen);
    for (int i = 0; i < 54; ++i) {
        int row = indices[i] / 9;
        int col = indices[i] % 9;
        int temp = grid[row][col];
        grid[row][col] = 0;
        // Check uniqueness
        std::vector<std::vector<int>> tempGrid = grid;
        if (!solveSudoku(tempGrid)) {
            grid[row][col] = temp; // If removing the number makes the puzzle unsolvable, revert the change
            continue;
        }

    }
}
