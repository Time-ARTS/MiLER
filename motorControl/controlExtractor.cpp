#include "controlExtractor.hpp"

#include <cmath>
#include <vector>

ControlSignals ControlExtractor::extract(const std::vector<Vector2>& points) {
    const Vector2 v = computeMeanVector(points);
    return {
        computeDeviation(v),
        computeCurvature(points)
    };
}

Vector2 ControlExtractor::computeMeanVector(const std::vector<Vector2>& points) {
    Vector2 sum{0,0};
    int valid = 0;

    for (const auto& [x, y] : points) {
        if (x >= 0 && y >= 0) {
            sum.x += x;
            sum.y += y;
            valid++;
        }
    }
    return valid ? Vector2(sum.x/valid, sum.y/valid) : Vector2(0,0);
}

float ControlExtractor::computeDeviation(const Vector2& vector) {
    return std::atan2(vector.x, vector.y); // steering error
}

float ControlExtractor::computeCurvature(const std::vector<Vector2>& points) {
    if (points.size() < 3) return 0;

    float total = 0;
    int n = 0;
    for (size_t i = 1; i < points.size()-1; ++i) {
        const auto [x, y] = points[i] - points[i-1];
        const auto [z, w] = points[i+1] - points[i];
        total += std::fabs(std::atan2(w,z) - std::atan2(y,x));
        n++;
    }
    return n ? total/n : 0;
}

