#include <cmath>
#include <iostream>

// using namespace std;

struct vec2D {
    double x;
    double y;

    vec2D() {
        x = 0;
        y = 0;
    }

    vec2D(double x_val, double y_val) {
        x = x_val;
        y = y_val;
    }

    void add(vec2D &v) {
        x += v.x;
        y += v.y;
    }

    vec2D subtract(vec2D &v) {
        return vec2D(x - v.x, y - v.y);
    }

    vec2D multiply(double num) {
        return vec2D(x * num, y * num);
    }

    vec2D divide(double num) {
        return vec2D(x / num, y / num);
    }

    double length() {
        return std::sqrt(x*x + y*y);
    }
};