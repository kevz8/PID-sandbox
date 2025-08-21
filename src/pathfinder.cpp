#include "pathfinder.h"
#include <queue>
#include <algorithm>
#include <cmath>
#include <unordered_map>

std::pair<int, int> start = {1, 1};
std::pair<int, int> goal = {GRID_W - 2, GRID_H - 2};

struct ComparePairs {
    bool operator()(const std::pair<float, std::pair<float, float>>& a, const std::pair<float, std::pair<float, float>>& b) {
        return a.first > b.first;
    }
};

float heuristic(std::pair<float, float> curr, std::pair<float, float> goal) {
    return std::abs(curr.first - goal.first) + std::abs(curr.second - goal.second);
}

void recalcPath(std::pair<float, float> from) {
    std::priority_queue<std::pair<float, std::pair<float, float>>, std::vector<std::pair<float, std::pair<float, float>>>, ComparePairs> pq;


    std::unordered_map<int, std::pair<float, float>> path;
    std::unordered_map<int, float> cost;
    auto hash = [](int x, int y) {return y * GRID_W + x;};

    pq.push({0.0f, from});
    cost[hash(from.first, from.second)] = 0.0f;

    static const std::pair<float, float> dirs[4] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}
    };

    while (!pq.empty()) {
        std::pair<float, std::pair<float, float>> top = pq.top();
        pq.pop();
        float currCost = top.first;
        std::pair<float, float> curr = top.second;
        float cx = top.second.first;
        float cy = top.second.second;

        if (cx == goal.first && cy == goal.second) {
            break;
        }       

        for (int i = 0; i < 4; i++) {
            int nx = cx + dirs[i].first;
            int ny = cy + dirs[i].second;

            if (!inBounds(nx, ny)) continue;
            if (grid[ny][nx].obstacle) continue;

            int currHash = hash(cx, cy);
            float newCost = cost[currHash] + 1.0f;
            int nextHash = hash(nx, ny);

            if (cost.count(nextHash) == 0 || newCost < cost[nextHash]) {
                cost[nextHash] = newCost;

                std::pair<int, int> nextPos = {nx, ny};
                float price = newCost + heuristic(nextPos, goal);

                pq.push(std::make_pair(price, nextPos));

                path[nextHash] = curr;
            }
        }
    }

    path.clear();
    std::pair<float, float> curr = goal;
    while (!(curr.first == from.first && curr.second == from.second)) {
        path.emplace(curr);
        int h = hash(curr.first, curr.second);
        if (!path.count(h)) {
            path.clear();
            return;
        }

        curr = path[h];
    }

    std::reverse(path.begin(), path.end());
}
