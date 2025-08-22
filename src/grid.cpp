#include "grid.h"
#include <utility>

Cell grid[GRID_H][GRID_W];

bool inBounds(int x, int y) {
    return x >= 0 && x < GRID_W && y >= 0 && y < GRID_H;
}

void applyWindToCoord(int x, int y, std::pair<float, float> windVec) {
    if (inBounds(x, y)) {
        grid[y][x].wind = windVec;
    }
}

void clearWindCoord(int x, int y) {
    if (inBounds(x, y)) {
        grid[y][x].wind = {0.0f, 0.0f};
    }
}