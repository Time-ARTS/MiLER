#ifndef MILLER_CONSTRUCTS_HPP
#define MILLER_CONSTRUCTS_HPP

#pragma once
#include <functional>
#include <string>
#include <opencv2/imgproc.hpp>

constexpr double PI = 3.141592653589793238463;

struct Vector2
{
    float x;
    float y;

    Vector2 operator-() const { return {-x, -y}; }

    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
};

struct ToggleBinding {
    char key;
    bool* flag;
    const std::string name;
};

struct Trigger {
    std::function<bool()> condition;
    int key;
};

struct DriveCommand
{
    double speed;
    double correction;

    [[nodiscard]] std::pair<double, double> motorCommand() const
    {
        const auto left = std::clamp(speed + correction, 0.0, 100.0);
        const auto right = std::clamp(speed - correction, 0.0, 100.0);
        return {left, right};
    }
};

enum class MessageType
{
    INFO,
    WARNING,
    ERROR,
    DEBUG,
    SUCCESS
};
#endif //MILLER_CONSTRUCTS_HPP
