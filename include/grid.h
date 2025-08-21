#ifndef GRID_H
#define GRID_H
#include "config.h"

struct Cell {
    // int x, y;
    bool obstacle = false;
    float windX = 0.0f;
    float windY = 0.0f;
};

extern Cell grid[GRID_H][GRID_W];

bool inBounds(int x, int y);
void applyWindToCoord(int x, int y, float wx, float wy);
void clearWindCoord(int x, int y);

#endif