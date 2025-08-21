#include "grid.h"

Cell grid[GRID_H][GRID_W];

bool inBounds(int x, int y) {
    return x >= 0 && x < 40 && y >= 0 && y < 30;
}

void applyWindToCoord(int x, int y, float wx, float wy) {
    if (inBounds(x, y)) {
        grid[y][x].windX = wx;
        grid[y][x].windY = wy;
    }
}

void clearWindCoord(int x, int y) {
    if (inBounds(x, y)) {
        grid[y][x].windX = 0;
        grid[y][x].windY = 0;
    }
}