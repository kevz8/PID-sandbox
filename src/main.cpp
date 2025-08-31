#include "config.h"
#include "grid.h"
#include "pathfinder.h"
#include "bot.h"
#include "physics.h"
#include <raylib.h>
#include <cmath>
#include <vector>
#include <utility>

BotPhysics botPhysics;
Bot bot;

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
    for (std::pair<int, int> p : currentPath) 
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
                // Wind length scaling
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

    while (!WindowShouldClose()) {
        // dt: Delta time since last frame
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
                    // Wind relative to cell size
                    std::pair<float, float> dir = {dx / CELL_SIZE, dy / CELL_SIZE};
                    applyWindToCoord(windAt.first, windAt.second, dir);
                }
            }

            windAt = {-1, -1};
            mouseWindStart = {-1, -1};
        }

        botPhysics.update(bot, dt, grid);

        BeginDrawing();
        ClearBackground(WHITE);

        updateGrid();

        DrawRectangle((start.first * CELL_SIZE), (start.second * CELL_SIZE), CELL_SIZE, CELL_SIZE, BLUE);
        DrawRectangle((goal.first * CELL_SIZE), (goal.second * CELL_SIZE), CELL_SIZE, CELL_SIZE, GREEN);
        DrawCircleV({bot.pos.first, bot.pos.second}, bot.radius, ORANGE);
        drawButtons(buttons);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}