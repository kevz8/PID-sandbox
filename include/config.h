#ifndef CONFIG_H
#define CONFIG_H

const int GRID_W = 40;
const int GRID_H = 30;
const int CELL_SIZE = 20;
const int SCREEN_W = GRID_W * CELL_SIZE;
const int SCREEN_H = GRID_H * CELL_SIZE + 40;

enum Mode {OBSTACLE, WIND};

#endif