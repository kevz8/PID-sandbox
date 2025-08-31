#include "physics.h"
#include <cmath>
#include <raylib.h>

std::pair<float, float> BotPhysics::resolveCollision(Bot& bot, std::pair<float, float> newPos, Cell grid[GRID_H][GRID_W]) {
    int newX = (int) newPos.first / CELL_SIZE;
    int newY = (int) newPos.second / CELL_SIZE;
    int botRad = bot.radius;

    int collisionCount = 0;
    std::pair<float, float> normalForce = {0.0f, 0.0f};
    std::pair<float, float> avgNormal = {0.0f, 0.0f};

    for (int y = newY - 1; y <= newY + 1; y++) {
        for (int x = newX - 1; x <= newX + 1; x++) {
            if (!inBounds(x, y)) continue;
            if (grid[y][x].obstacle) {
                Rectangle cellRect = {(float) x * CELL_SIZE, (float) y * CELL_SIZE, (float) CELL_SIZE, (float) CELL_SIZE};
                if (CheckCollisionCircleRec({newPos.first, newPos.second}, botRad, cellRect)) {
                    collisionCount++;
                    std::pair<float, float> cellCenter = {(x + 0.5f) * CELL_SIZE, (y + 0.5f) * CELL_SIZE};
                    normalForce = {bot.pos.first - cellCenter.first, bot.pos.second - cellCenter.second};
                    float normalForceMag = std::sqrt(normalForce.first * normalForce.first + normalForce.second * normalForce.second);
                    normalForce.first = normalForce.first / normalForceMag;
                    normalForce.second = normalForce.second / normalForceMag;
                    avgNormal = {avgNormal.first + normalForce.first, avgNormal.second + normalForce.second};
                }
            }
        }
    }

    avgNormal = {avgNormal.first / (float) collisionCount, avgNormal.second / (float) collisionCount};
    float avgNormalMag = std::sqrt(avgNormal.first * avgNormal.first + avgNormal.second * avgNormal.second);
    avgNormal = {avgNormal.first / avgNormalMag, avgNormal.second / avgNormalMag};

    if (collisionCount > 0) {
        float projScale = (bot.vel.first * avgNormal.first + bot.vel.second * avgNormal.second);
        std::pair<float, float> proj = {avgNormal.first * projScale, avgNormal.second * projScale};
        // Reflection formula: v' = v - 2(v ⋅ n)n
        std::pair<float, float> reflect = {bot.vel.first - 2 * proj.first, bot.vel.second - 2 * proj.second};
        bot.vel = {reflect.first * 0.6f, reflect.second * 0.6f};
        return {bot.pos.first + avgNormal.first * botRad * 0.1f, bot.pos.second + avgNormal.second * botRad * 0.1f};
    } else {
        return newPos;
    }
}

void BotPhysics::update(Bot& bot, float dt, Cell grid[GRID_H][GRID_W]) {
    if (!currentPath.empty()) {
        std::pair<int, int> cell = currentPath[0];
        std::pair<int, int> target = {(cell.first + 0.5f) * CELL_SIZE, (cell.second + 0.5f) * CELL_SIZE};
        std::pair<float, float> error = {target.first - bot.pos.first, target.second - bot.pos.second};

        if (std::sqrt(std::sqrt(error.first * error.first + error.second * error.second)) < CELL_SIZE * 0.17f) currentPath.erase(currentPath.begin());

        integral = {integral.first + error.first, integral.second + error.second};
        std::pair<float, float> derivative = {error.first - prevError.first, error.second - prevError.second};
        prevError = error;

        std::pair<float, float> control = {
            P * error.first + I * integral.first + D * derivative.first, 
            P * error.second + I * integral.second + D * derivative.second
        };
        control = {control.first * 0.5f, control.second * 0.5f};

        int botX = (int) bot.pos.first / CELL_SIZE;
        int botY = (int) bot.pos.second / CELL_SIZE;
        std::pair<float, float> wind = {0, 0};

        if (inBounds(botX, botY)) {
            wind = grid[botY][botX].wind;
        }

        std::pair<float, float> windForce = {wind.first * 100.0f, wind.second * 100.0f};
        std::pair<float, float> drag = {bot.vel.first * (-bot.dragCoeff), bot.vel.second * (-bot.dragCoeff)};
        std::pair<float, float> totalForce = {control.first + windForce.first + drag.first, control.second + windForce.second + drag.second};
        std::pair<float, float> accel = {totalForce.first / bot.mass, totalForce.second / bot.mass};

        bot.vel = {bot.vel.first + accel.first * dt, bot.vel.second + accel.second * dt};
        float speed = std::sqrt(bot.vel.first * bot.vel.first + bot.vel.second * bot.vel.second);
        
        std::pair<float, float> newPos = {bot.pos.first + bot.vel.first * dt, bot.pos.second + bot.vel.second * dt};

        bot.pos = BotPhysics::resolveCollision(bot, newPos, grid);

        if (!currentPath.empty()) {
            std::pair<int, int> currCell = {bot.pos.first / CELL_SIZE, bot.pos.second/ CELL_SIZE};
            std::pair<int, int> nextCell = currentPath[0];
            if (grid[nextCell.second][nextCell.first].obstacle) {
                recalcPath({currCell.first, currCell.second});
            }
        }
    }
}