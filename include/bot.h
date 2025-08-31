#ifndef BOT_H
#define BOT_H
#include <utility>

struct Bot {
    std::pair<float, float> pos;
    std::pair<float, float> vel;
    float mass = 0.3f;
    float dragCoeff = 0.2f;
    float maxSpeed = 800.0f;
    int radius = 7;
};

extern Bot bot;

#endif