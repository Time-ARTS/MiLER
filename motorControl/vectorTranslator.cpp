#include "vectorTranslator.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

Speed VectorTranslator::translate(const std::vector<Vector2>& points, const float preferredSpeed) {
    const Vector2 vector = computeVectors(points);
    const float deviation = computeAngularDeviation(vector);
    const float curvature = computeCurvature(points);
    return computeWheelSpeeds(preferredSpeed, deviation, curvature);
}

Vector2 VectorTranslator::computeVectors(const std::vector<Vector2>& points) {
    Vector2 sum{0.0f, 0.0f};
    for (const auto& [x, y] : points) {
        sum.x += x;
        sum.y += y;
    }
    const auto count = static_cast<float>(points.size());
    return {sum.x / count, sum.y / count};
}

float VectorTranslator::computeAngularDeviation(const Vector2& vector) {
    const float mag = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (mag < 1e-5f) return 0.0f;

    const float nx = vector.x / mag;
    const float ny = vector.y / mag;

    return std::atan2(nx, ny);
}

float VectorTranslator::computeCurvature(const std::vector<Vector2>& points) {
    if (points.size() < 3) return 0.0f;

    float totalAngle = 0.0f;
    int count = 0;

    for (size_t i = 1; i < points.size() - 1; ++i) {
        const Vector2 a = {points[i].x - points[i - 1].x, points[i].y - points[i - 1].y};
        const Vector2 b = {points[i + 1].x - points[i].x, points[i + 1].y - points[i].y};

        const float angleA = std::atan2(a.y, a.x);
        const float angleB = std::atan2(b.y, b.x);
        const float delta = angleB - angleA;

        totalAngle += std::fabs(delta);
        count++;
    }

    return count > 0 ? totalAngle / static_cast<float>(count) : 0.0f;
}

Speed VectorTranslator::computeWheelSpeeds(const float baseSpeed, const float deviation, const float curvature) {
    constexpr float deviationGain = 0.6f; // how strongly it corrects angle
    constexpr float curvatureGain = 0.4f; // how strongly it slows for curves
    constexpr float minSpeed = 0.2f;

    // Slow down proportionally to curvature
    const float speedAdjustment = 1.0f / (1.0f + curvature * curvatureGain);
    const float adjustedSpeed = baseSpeed * speedAdjustment;

    // Differential steering
    const float turnAdjustment = deviation * deviationGain;

    float leftSpeed = adjustedSpeed - turnAdjustment;
    float rightSpeed = adjustedSpeed + turnAdjustment;

    // Clamp values between minSpeed and maxSpeed (1.0)
    leftSpeed = std::clamp(leftSpeed, minSpeed, 1.0f);
    rightSpeed = std::clamp(rightSpeed, minSpeed, 1.0f);

    speed.left = leftSpeed;
    speed.right = rightSpeed;
    speed.deviation = deviation;

    return speed;
}
