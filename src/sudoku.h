#pragma once

#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <unordered_map>
#include <set>
#include "util.h"

enum class State
{
    Start, Empty, Valid, Unvalid
};

class Sudoku
{
private:
    std::pair<int, int> getSubgridIndices(int i, int j) {
        int subgridRow = i / 3;
        int subgridCol = j / 3;
        return std::make_pair(subgridRow, subgridCol);
    }

    std::vector<std::pair<int, int>> getSubgridCells(int subgridRow, int subgridCol) {
        std::vector<std::pair<int, int>> subgridCells;
        for (int i = subgridRow * 3; i < (subgridRow + 1) * 3; ++i) {
            for (int j = subgridCol * 3; j < (subgridCol + 1) * 3; ++j) {
                subgridCells.push_back(std::make_pair(i, j));
            }
        }

        return subgridCells;
    }

    std::vector<std::pair<int, int>> getOtherRowsCols(int i, int j) {
        std::vector<std::pair<int, int>> result;
        // Add all cells from the same row except (i, j)
        for (int col = 0; col < 9; ++col) {
            if (col != j) {
                result.emplace_back(i, col);
            }
        }
        // Add all cells from the same column except (i, j)
        for (int row = 0; row < 9; ++row) {
            if (row != i) {
                result.emplace_back(row, j);
            }
        }
        return result;
    }

public:
    Sudoku();

    bool isValidSudoku(const std::vector<std::vector<int>>& board) {
        std::vector<std::vector<bool>> rowFlag(9, std::vector<bool>(9, false));
        std::vector<std::vector<bool>> colFlag(9, std::vector<bool>(9, false));
        std::vector<std::vector<bool>> boxFlag(9, std::vector<bool>(9, false));

        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (board[i][j] != 0) {
                    int num = board[i][j] - 1;
                    int k = i / 3 * 3 + j / 3;
                    if (rowFlag[i][num] || colFlag[j][num] || boxFlag[k][num]) {
                        return false;
                    }
                    rowFlag[i][num] = colFlag[j][num] = boxFlag[k][num] = true;
                }
                else if (board[i][j] == 0) {
                    // If there's an empty cell, it's not a complete solution yet, so return true
                    return false;
                }
            }
        }
        return true;
    }

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
    void shuffle() {
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
    void generateSudoku() {
        // Initialize grid with zeros
        grid.assign(9, std::vector<int>(9, 0));
        // Solve the Sudoku
        solveSudoku(grid);
        // Shuffle rows, columns, and numbers
        shuffle();
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

    std::vector<int> findAll()
    {
        std::vector<int> all;
        for (int i = 0; i < 9 * 9; i++)
            if (cell_numbers[active] == cell_numbers[i] && cell_numbers[i] != 0)
                all.emplace_back(i);
        return all;
    }

    std::vector<std::pair<int, int>> getAllColored()
    {
        std::set<std::pair<int, int>> colored;
        std::pair<int, int> subgridIndices = getSubgridIndices(get2DIndex(active, 9).first, get2DIndex(active, 9).second);
        auto subgridCells = getSubgridCells(subgridIndices.first, subgridIndices.second);
        auto otherRowsCols = getOtherRowsCols(get2DIndex(active, 9).first, get2DIndex(active, 9).second);

        for (int i = 0; i < subgridCells.size(); i++)
        {
            if (get2DIndex(active, 9) == subgridCells[i])
                continue;
            colored.insert(subgridCells[i]);
        }
        for (int i = 0; i < otherRowsCols.size(); i++)
        {
            colored.insert(otherRowsCols[i]);
        }
        return std::vector<std::pair<int, int>>(colored.begin(), colored.end());
    }

    void initializeCellNumbers()
    {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                cell_numbers[get1DIndex(i, j, 9)] = grid[i][j];
    }

    void initializeStates()
    {
        for (int i = 0; i < cell_numbers.size(); i++)
            cell_numbers[i] == 0 ? states[i] = State::Empty : states[i] = State::Start;
    }

    void updateGrid()
    {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                grid[i][j] = cell_numbers[get1DIndex(i, j, 9)];
    }

public:
    int active = 0;
    std::vector<int> cell_numbers;
    std::vector<std::vector<int>> grid, solved, start;
    std::unordered_map<int, State> states;
};

Sudoku::Sudoku()
{
    cell_numbers.assign(9 * 9, 0);
    grid.assign(9, std::vector<int>(9, 0));
}

bool isValidSudoku(const std::vector<std::vector<int>>& board) {
    std::vector<std::vector<bool>> rowFlag(9, std::vector<bool>(9, false));
    std::vector<std::vector<bool>> colFlag(9, std::vector<bool>(9, false));
    std::vector<std::vector<bool>> boxFlag(9, std::vector<bool>(9, false));

    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (board[i][j] != 0) {
                int num = board[i][j] - 1;
                int k = i / 3 * 3 + j / 3;
                if (rowFlag[i][num] || colFlag[j][num] || boxFlag[k][num]) {
                    return false;
                }
                rowFlag[i][num] = colFlag[j][num] = boxFlag[k][num] = true;
            }
            else if (board[i][j] == 0) {
                // If there's an empty cell, it's not a complete solution yet, so return true
                return false;
            }
        }
    }
    return true;
}

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

std::pair<int, int> getSubgridIndices(int i, int j) {
    int subgridRow = i / 3;
    int subgridCol = j / 3;
    return std::make_pair(subgridRow, subgridCol);
}

std::vector<std::pair<int, int>> getSubgridCells(int subgridRow, int subgridCol) {
    std::vector<std::pair<int, int>> subgridCells;
    for (int i = subgridRow * 3; i < (subgridRow + 1) * 3; ++i) {
        for (int j = subgridCol * 3; j < (subgridCol + 1) * 3; ++j) {
            subgridCells.push_back(std::make_pair(i, j));
        }
    }

    return subgridCells;
}

std::vector<std::pair<int, int>> getOtherRowsCols(int i, int j) {
    std::vector<std::pair<int, int>> result;
    // Add all cells from the same row except (i, j)
    for (int col = 0; col < 9; ++col) {
        if (col != j) {
            result.emplace_back(i, col);
        }
    }
    // Add all cells from the same column except (i, j)
    for (int row = 0; row < 9; ++row) {
        if (row != i) {
            result.emplace_back(row, j);
        }
    }
    return result;
}

std::vector<int> findAll(int active, std::vector<int>& cell_numbers)
{
    std::vector<int> all;
    for (int i = 0; i < 9 * 9; i++)
    {
        if (cell_numbers[active] == cell_numbers[i] && cell_numbers[i] != 0)
        {
            all.emplace_back(i);
        }
    }
    return all;
}

std::vector<std::pair<int, int>> getAllColored(int active)
{
    std::set<std::pair<int, int>> colored;
    std::pair<int, int> subgridIndices = getSubgridIndices(get2DIndex(active, 9).first, get2DIndex(active, 9).second);
    auto subgridCells = getSubgridCells(subgridIndices.first, subgridIndices.second);
    auto otherRowsCols = getOtherRowsCols(get2DIndex(active, 9).first, get2DIndex(active, 9).second);

    for (int i = 0; i < subgridCells.size(); i++)
    {
        if (get2DIndex(active, 9) == subgridCells[i])
            continue;
        colored.insert(subgridCells[i]);
    }
    for (int i = 0; i < otherRowsCols.size(); i++)
    {
        colored.insert(otherRowsCols[i]);
    }
    return std::vector<std::pair<int, int>>(colored.begin(), colored.end());
}

void initializeCellNumbers(const std::vector<std::vector<int>>& grid, std::vector<int>& cell_numbers)
{
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            cell_numbers[get1DIndex(i, j, 9)] = grid[i][j];
        }
    }
}

void initializeStates(const std::vector<int>& cell_numbers, std::unordered_map<int, State>& states)
{
    for (int i = 0; i < cell_numbers.size(); i++)
        cell_numbers[i] == 0 ? states[i] = State::Empty : states[i] = State::Start;
}

void updateGrid(const std::vector<int>& cell_numbers, std::vector<std::vector<int>>& grid)
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            grid[i][j] = cell_numbers[get1DIndex(i, j, 9)];
}
