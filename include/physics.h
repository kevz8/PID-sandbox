#ifndef PHYSICS_H
#define PHYSICS_H
#include "grid.h"
#include "bot.h"
#include "pathfinder.h"
#include <vector>

struct BotPhysics {
    float P = 1.5f;
    float I = 0.0f;
    float D = 0.3f;

    std::pair<float, float> integral = {0.0f, 0.0f};
    std::pair<float, float> prevError = {0.0f, 0.0f};

    void update(Bot& bot, float dt, Cell grid[GRID_H][GRID_W]);

private:
    std::pair<float, float> resolveCollision(Bot& bot, std::pair<float, float> newPos, Cell grid[GRID_H][GRID_W]);
};

#endif