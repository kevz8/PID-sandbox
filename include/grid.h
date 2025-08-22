#ifndef GRID_He
#define GRID_He
#include "config.h"

struct Cell {
    // int x, y;
    bool obstacle = false;
    std::pair<float, float> wind = {0.0f, 0.0f};
};

extern Cell grid[GRID_H][GRID_W];

bool inBounds(int x, int y);
void applyWindToCoord(int x, int y, std::pair<float, float> windVec);
void clearWindCoord(int x, int y);

#endif