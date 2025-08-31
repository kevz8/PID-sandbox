#ifndef PATHFINDER_H
#define PATHFINDER_H
#include "grid.h"
#include <vector>

extern std::pair<int, int> start, goal;
extern std::vector<std::pair<int, int>> currentPath;

void recalcPath(std::pair<int, int> from);
// Heuristic function for A* pathfinding. Resturns estimates cost from current bot position to goal.
float heuristic(std::pair<float, float> curr, std::pair<float, float> goal);

#endif