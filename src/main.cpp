#include "config.h"
#include "grid.h"
#include "pathfinder.h"
#include "bot.h"
#include <raylib.h>
#include <cmath>
#include <vector>
#include <utility>

struct Button {
    Rectangle bounds;
    const char* label;
    Mode mode;
};

Mode editMode = OBSTACLE;

void drawButtons(std::vector<Button> buttons) {
    for (const Button b : buttons) {
        DrawRectangleRec(b.bounds, (editMode == b.mode) ? GRAY : DARKGRAY);
        DrawText(b.label, b.bounds.x + 10, b.bounds.y + 5, 14, WHITE);
    }
}

void updateGrid() {
    for (std::pair<int, int> p : finalPath) 
        DrawRectangleRec({(float) p.first*CELL_SIZE, (float) p.second*CELL_SIZE, (float)CELL_SIZE, (float)CELL_SIZE}, ColorAlpha(GREEN, 0.3f));

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            Rectangle rec = {(float) x * CELL_SIZE, (float) y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            if (grid[y][x].obstacle) {
                DrawRectangleRec(rec, RED);
            } else {
                DrawRectangleLinesEx(rec, 1, DARKGRAY);
            }
            
            std::pair<float, float> w = grid[y][x].wind;
            if (w.first != 0 || w.second != 0 ) {
                Vector2 center = {(x + 0.5f) * CELL_SIZE, (y + 0.5f) * CELL_SIZE};
                float len = std::sqrt(w.first * w.first + w.second * w.second);
                float ex = w.first / (len + 0.7f) * 15.0f + center.x;
                float ey = w.second / (len + 0.7f) * 15.0f + center.y;
                Vector2 end = {ex, ey};
                DrawLineEx(center, end, 3.0f, SKYBLUE);
            }
        }
    }
}

int main() {
    InitWindow(SCREEN_W, SCREEN_H, "PID Sandbox");
    bot.pos = {start.first * CELL_SIZE + CELL_SIZE / 2, start.second * CELL_SIZE + CELL_SIZE / 2};

    recalcPath(start);

    std::vector<Button> buttons = {
        {{10, SCREEN_H - 35, 120, 30}, "Obstacle Mode", OBSTACLE},
        {{140, SCREEN_H - 35, 120, 30}, "Wind Mode", WIND}
    };

    std::pair<int, int> windAt = {-1, -1};
    std::pair<float, float> mouseWindStart = {-1.0f, -1.0f};
    float P = 1.5f;
    float I = 0.0f;
    float D = 0.3f;

    std::pair<float, float> integral = {0.0f, 0.0f};
    std::pair<float, float> prevError = {0.0f, 0.0f};
    int botRad = 7;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();
        float mx = mouse.x / CELL_SIZE;
        float my = mouse.y / CELL_SIZE;

        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && inBounds(mx, my)) {
            grid[(int) my][(int) mx].obstacle = false;
            clearWindCoord(mx, my);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (Button b : buttons) {
                if (CheckCollisionPointRec(mouse, b.bounds)) {
                    editMode = b.mode;
                }
            }

            if (editMode == WIND && inBounds(mx, my)) {
                windAt = {mx, my};
                mouseWindStart = {mouse.x, mouse.y};
            } else if (inBounds(mx, my)) {
                grid[(int) my][(int) mx].obstacle = true;
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (windAt.first != -1) {
                float dx = mouse.x - mouseWindStart.first;
                float dy = mouse.y - mouseWindStart.second;

                if (std::sqrt(dx * dx + dy * dy)) {
                    std::pair<float, float> dir = {dx / CELL_SIZE, dy / CELL_SIZE};
                    applyWindToCoord(windAt.first, windAt.second, dir);
                }
            }

            windAt = {-1, -1};
            mouseWindStart = {-1, -1};
        }

        if (!finalPath.empty()) {
            std::pair<int, int> cell = finalPath[0];
            std::pair<int, int> target = {(cell.first + 0.5f) * CELL_SIZE, (cell.second + 0.5f) * CELL_SIZE};
            std::pair<float, float> error = {target.first - bot.pos.first, target.second - bot.pos.second};

            if (std::sqrt(std::sqrt(error.first * error.first + error.second * error.second)) < CELL_SIZE * 0.17f) finalPath.erase(finalPath.begin());

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
            int newX = (int) newPos.first / CELL_SIZE;
            int newY = (int) newPos.second / CELL_SIZE;

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
                std::pair<float, float> reflect = {bot.vel.first - 2 * proj.first, bot.vel.second - 2 * proj.second};
                bot.vel = {reflect.first * 0.6f, reflect.second * 0.6f};
                bot.pos = {bot.pos.first + avgNormal.first * botRad * 0.1f, bot.pos.second + avgNormal.second * botRad * 0.1f};
            } else {
                bot.pos = newPos;
            }

            if (!finalPath.empty()) {
                std::pair<int, int> currCell = {bot.pos.first / CELL_SIZE, bot.pos.second/ CELL_SIZE};
                std::pair<int, int> nextCell = finalPath[0];
                if (grid[nextCell.second][nextCell.first].obstacle) {
                    recalcPath({currCell.first, currCell.second});
                }
            }
        }

        BeginDrawing();
        ClearBackground(WHITE);

        updateGrid();

        DrawRectangle((start.first * CELL_SIZE), (start.second * CELL_SIZE), CELL_SIZE, CELL_SIZE, BLUE);
        DrawRectangle((goal.first * CELL_SIZE), (goal.second * CELL_SIZE), CELL_SIZE, CELL_SIZE, GREEN);

        DrawCircleV({bot.pos.first, bot.pos.second}, botRad, ORANGE);

        drawButtons(buttons);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}