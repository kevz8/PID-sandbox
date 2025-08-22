#ifndef PATHFINDER_H
#define PATHFINDER_H
#include "grid.h"
#include <vector>

extern std::pair<int, int> start, goal;
extern std::vector<std::pair<int, int>> finalPath;

void reclacPath(std::pair<int, int> from);
float heuristic(std::pair<float, float> curr, std::pair<float, float> goal);

#endif