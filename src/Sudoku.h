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
    std::pair<int, int> getSubgridIndices(int i, int j);
    std::vector<std::pair<int, int>> getSubgridCells(int subgridRow, int subgridCol);
    std::vector<std::pair<int, int>> getOtherRowsCols(int i, int j);
    int difficulty;
public:
    Sudoku();

    const bool isValidSudoku(const std::vector<std::vector<int>>& board);
    const bool isValid(const std::vector<std::vector<int>>& board, int row, int col, int num);
    // Function to solve Sudoku recursively
    bool solveSudoku(std::vector<std::vector<int>>& board);

    // Function to shuffle rows and columns
    void shuffle();
    // Function to generate a random Sudoku puzzle
    void generateSudoku();
    void setDifficulty(int difficulty_level);

    std::vector<int> findAll();
    std::vector<std::pair<int, int>> getAllColored();

    void initializeCellNumbers();
    void initializeStates();

    void updateGrid();
public:
    int active = 0;
    std::vector<int> cell_numbers;
    std::vector<std::vector<int>> grid, solved, start;
    std::unordered_map<int, State> states;
};

std::pair<int, int> Sudoku::getSubgridIndices(int i, int j) {
    int subgridRow = i / 3;
    int subgridCol = j / 3;
    return std::make_pair(subgridRow, subgridCol);
}

std::vector<std::pair<int, int>> Sudoku::getSubgridCells(int subgridRow, int subgridCol) {
    std::vector<std::pair<int, int>> subgridCells;
    for (int i = subgridRow * 3; i < (subgridRow + 1) * 3; ++i) {
        for (int j = subgridCol * 3; j < (subgridCol + 1) * 3; ++j) {
            subgridCells.push_back(std::make_pair(i, j));
        }
    }

    return subgridCells;
}

std::vector<std::pair<int, int>> Sudoku::getOtherRowsCols(int i, int j) {
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

Sudoku::Sudoku()
{
    setDifficulty(1);
    cell_numbers.assign(9 * 9, 0);
    grid.assign(9, std::vector<int>(9, 0));
}

const bool Sudoku::isValidSudoku(const std::vector<std::vector<int>>& board) {
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

const bool Sudoku::isValid(const std::vector<std::vector<int>>& board, int row, int col, int num) {
    // Check row
    for (int i = 0; i < 9; ++i) {
        if (board[row][i] == num) return false;
    }
    // Check column
    for (int i = 0; i < 9; ++i) {
        if (board[i][col] == num) return false;
    }
    // Check subgrid
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i + startRow][j + startCol] == num) return false;
        }
    }
    return true;
}

// Function to solve Sudoku recursively
bool Sudoku::solveSudoku(std::vector<std::vector<int>>& board) {
    for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
            if (board[row][col] == 0) {
                for (int num = 1; num <= 9; ++num) {
                    if (isValid(board, row, col, num)) {
                        board[row][col] = num;
                        if (solveSudoku(board)) {
                            return true;
                        }
                        board[row][col] = 0; // Backtrack
                    }
                }
                return false;
            }
        }
    }
    return true;
}

// Function to shuffle rows and columns
void Sudoku::shuffle() {
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
void Sudoku::generateSudoku() {
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
    for (int i = 0; i < difficulty; ++i) {
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

void Sudoku::setDifficulty(int difficulty_level)
{
    std::random_device seed;
    std::mt19937 gen{ seed() }; // seed the generator

    // Easy
    int min = 35, max = 39;
    switch (difficulty_level)
    {
    case 1:
        // Medium
        min = 31; max = 34;
        break;
    case 2:
        // Hard
        min = 26; max = 29;
        break;
    case 3:
        // Evil
        min = 17; max = 24;
        break;
    }
    std::uniform_int_distribution<> dist{ min, max }; // set min and max
    difficulty = 81 - dist(gen);
    SDL_Log("%d", 81 - difficulty);
}

std::vector<int> Sudoku::findAll()
{
    std::vector<int> all;
    for (int i = 0; i < 9 * 9; i++)
        if (cell_numbers[active] == cell_numbers[i] && cell_numbers[i] != 0)
            all.emplace_back(i);
    return all;
}

std::vector<std::pair<int, int>> Sudoku::getAllColored()
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

void Sudoku::initializeCellNumbers()
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            cell_numbers[get1DIndex(i, j, 9)] = grid[i][j];
}

void Sudoku::initializeStates()
{
    for (int i = 0; i < cell_numbers.size(); i++)
        cell_numbers[i] == 0 ? states[i] = State::Empty : states[i] = State::Start;
}

void Sudoku::updateGrid()
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            grid[i][j] = cell_numbers[get1DIndex(i, j, 9)];
}