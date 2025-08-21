#ifndef PATHFINDER_H
#define PATHFINDER_H
#include "grid.h"
#include <vector>

extern std::pair<int, int> start, goal;

void reclacPath(float curr_x, float curr_y);
int hash(int x, int y);

#endif