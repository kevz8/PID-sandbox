#include "pathfinder.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_map>

std::pair<int, int> start = {1, 1};
std::pair<int, int> goal = {GRID_W - 2, GRID_H - 2};
std::vector<std::pair<int, int>> finalPath;

struct Node {
    std::pair<int, int> pos;
    float cost;
    bool operator>(const Node& other) const {
        return cost > other.cost;
    }
};

float heuristic(std::pair<int, int> curr, std::pair<float, float> goal) {
    return std::abs(curr.first - goal.first) + std::abs(curr.second - goal.second);
}

void recalcPath(std::pair<int, int> from) {
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    std::pair<int, int> path[GRID_H][GRID_W];
    float costSoFar[GRID_H][GRID_W];

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            costSoFar[y][x] = std::numeric_limits<int>::max();
        }
    }

    pq.push({from, 0});
    costSoFar[from.second][from.first] = 0.0f;
    path[from.second][from.first] = from;

    static const std::pair<float, float> dirs[4] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}
    };

    while (!pq.empty()) {
        Node top = pq.top();
        pq.pop();
        int cx = top.pos.first;
        int cy = top.pos.second;

        if (cx == goal.first && cy == goal.second) {
            break;
        }       

        for (int i = 0; i < 4; i++) {
            int nx = cx + dirs[i].first;
            int ny = cy + dirs[i].second;

            if (!inBounds(nx, ny) || grid[ny][nx].obstacle) continue;

            float newCost = costSoFar[cy][cx] + 1;

            if (newCost < costSoFar[ny][nx]) {
                costSoFar[ny][nx] = newCost;
                float priority = newCost + heuristic(std::make_pair(nx, ny), goal);

                pq.push({{nx, ny}, priority});

                path[ny][nx] = top.pos;
            }
        }
    }

    finalPath.clear();
    std::pair<int, int> curr = goal;
    while (curr.first != from.first || curr.second != from.second) {
        finalPath.push_back(curr);
        if (curr.first < 0 || curr.second < 0) {
            finalPath.clear();
            return;
        }

        curr = path[curr.second][curr.first];
    }

    std::reverse(finalPath.begin(), finalPath.end());
}
