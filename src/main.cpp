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

// TODO
void updateGrid() {

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
            } else if (inBounds(mx, my)) {
                grid[(int) my][(int) mx].obstacle = true;
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            std::pair<float, float> dir = {mx - windAt.first, my - windAt.second};
            if (std::abs(dir.first + dir.second) > 0) {
                applyWindToCoord(windAt.first, windAt.second, dir);
            }
            windAt = {-1, -1};
        }

        if (!finalPath.empty()) {
            std::pair<int, int> cell = finalPath[0];
            std::pair<int, int> target = {(cell.first + 0.5f) * CELL_SIZE, (cell.second + 0.5f) * CELL_SIZE};
            std::pair<float, float> error = {target.first - bot.pos.first, target.second - bot.pos.second};

            if (std::sqrt(std::sqrt(error.first * error.first + error.second * error.second)) < CELL_SIZE * 0.2f) finalPath.erase(finalPath.begin());

            integral = {integral.first + error.first, integral.second + error.second};
            std::pair<float, float> derivative = {error.first - prevError.first, error.second - prevError.second};
            prevError = error;

            std::pair<float, float> control = {
                P * error.first + I * integral.first + D * derivative.first, 
                P * error.second + I * integral.second + D * derivative.second
            };

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
            int newX = (int) newPos.first;
            int newY = (int) newPos.second;

            bool collision = false;
            std::pair<float, float> normalForce = {0.0f, 0.0f};

            for (int y = newY - 1; y <= newY + 1; y++) {
                for (int x = newX - 1; x <= newX + 1; x++) {
                    if (!inBounds(x, y)) continue;
                    if (grid[y][x].obstacle) {
                        Rectangle cellRect = {(float) x * CELL_SIZE, (float) y * CELL_SIZE, (float) CELL_SIZE, (float) CELL_SIZE};
                        if (CheckCollisionCircleRec({newPos.first, newPos.second}, botRad, cellRect)) {
                            collision = true;
                            break;
                            // std::pair<float, float> center = {(x + 0.5f) * CELL_SIZE, (y + 0.5f) * CELL_SIZE};
                        }
                    }
                }
                if (collision) break;
            }

            if (collision) {
                    newPos = bot.pos;
                    bot.vel.first *= -0.5f;
                    bot.vel.second *= -0.5f;
            } else {
                bot.pos = newPos;
            }

            if (!finalPath.empty()) {
                std::pair<int, int> currCell = {bot.pos.first / CELL_SIZE, bot.pos.second/ CELL_SIZE};
                std::pair<int, int> nextCell = finalPath[0];
                if (grid[nextCell.second][nextCell.first].obstacle) {
                    recalcPath({currCell.first, currCell.second});
                    integral = {0.0f, 0.0f};
                }
            }
        }

        BeginDrawing();
        ClearBackground(WHITE);

        for (std::pair<int, int> p : finalPath) 
            DrawRectangleRec({(float) p.first*CELL_SIZE, (float) p.second*CELL_SIZE, (float)CELL_SIZE, (float)CELL_SIZE}, ColorAlpha(GREEN, 0.3f));

        DrawRectangle((start.first * CELL_SIZE), (start.second * CELL_SIZE), CELL_SIZE, CELL_SIZE, BLUE);
        DrawRectangle((goal.first * CELL_SIZE), (goal.second * CELL_SIZE), CELL_SIZE, CELL_SIZE, GREEN);

        DrawCircleV({bot.pos.first, bot.pos.second}, botRad, ORANGE);

        drawButtons(buttons);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}