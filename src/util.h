#pragma once

#include <utility>

inline int get1DIndex(int i, int j, int num_columns) {
    return i * num_columns + j;
}

std::pair<int, int> get2DIndex(int index, int size) {
    int i = index / size;
    int j = index % size;
    return { i, j };
}