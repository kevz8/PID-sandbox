#ifndef MY_HEADER_H
#define MY_HEADER_H

struct Cell {
    bool obstacle = false;
    float windX = 0.0f;
    float windY = 0.0f;
};

struct Grid {
public:
    static const int WIDTH = 40;
    static const int HEIGHT = 30;

    Grid();
    bool inBounds(int, int);
    bool isObstacle(int, int);
    void setObstacle(int, int);

    Cell cells[HEIGHT][WIDTH];

    private:
};


#endif