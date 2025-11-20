#include "vectorTranslator.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

Speed VectorTranslator::translate(const std::vector<Vector2>& points, float preferredSpeed) {
    Vector2 vector = computeVectors(points);
    float deviation = computeAngularDeviation(vector);
    float curvature = computeCurvature(points);
    return computeWheelSpeeds(preferredSpeed, deviation, curvature);
}

Vector2 VectorTranslator::computeVectors(const std::vector<Vector2>& points) {
    Vector2 sum{0.0f, 0.0f};
    for (const auto& p : points) {
        sum.x += p.x;
        sum.y += p.y;
    }
    float count = static_cast<float>(points.size());
    return {sum.x / count, sum.y / count};
}

float VectorTranslator::computeAngularDeviation(const Vector2& v) {
    float mag = std::sqrt(v.x * v.x + v.y * v.y);
    if (mag < 1e-5f) return 0.0f;

    float nx = v.x / mag;
    float ny = v.y / mag;

    return std::atan2(nx, ny);
}

float VectorTranslator::computeCurvature(const std::vector<Vector2>& points) {
    if (points.size() < 3) return 0.0f;

    float totalAngle = 0.0f;
    int count = 0;

    for (size_t i = 1; i < points.size() - 1; ++i) {
        Vector2 a = {points[i].x - points[i - 1].x, points[i].y - points[i - 1].y};
        Vector2 b = {points[i + 1].x - points[i].x, points[i + 1].y - points[i].y};

        float angleA = std::atan2(a.y, a.x);
        float angleB = std::atan2(b.y, b.x);
        float delta = angleB - angleA;

        totalAngle += std::fabs(delta);
        count++;
    }

    return (count > 0) ? totalAngle / count : 0.0f;
}

Speed VectorTranslator::computeWheelSpeeds(float baseSpeed, float deviation, float curvature) {
    const float deviationGain = 0.6f; // how strongly it corrects angle
    const float curvatureGain = 0.4f; // how strongly it slows for curves
    const float minSpeed = 0.2f;

    // Slow down proportionally to curvature
    float speedAdjustment = 1.0f / (1.0f + curvature * curvatureGain);
    float adjustedSpeed = baseSpeed * speedAdjustment;

    // Differential steering
    float turnAdjustment = deviation * deviationGain;

    float leftSpeed = adjustedSpeed - turnAdjustment;
    float rightSpeed = adjustedSpeed + turnAdjustment;

    // Clamp values between minSpeed and maxSpeed (1.0)
    leftSpeed = std::clamp(leftSpeed, minSpeed, 1.0f);
    rightSpeed = std::clamp(rightSpeed, minSpeed, 1.0f);

    return {leftSpeed, rightSpeed};
}
