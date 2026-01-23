#ifndef MILLER_CONSTRUCTS_HPP
#define MILLER_CONSTRUCTS_HPP

#pragma once

constexpr double PI = 3.141592653589793238463;

struct Vector2
{
    float x;
    float y;

    Vector2 operator-() const { return {-x, -y}; }

    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
};

struct ControlSignals
{
    float deviation;
    float curvature;
};

struct DriveCommand
{
    float speed;
    float turn;

    [[nodiscard]] std::pair<float, float> motorCommand() const
    {
        return {speed - turn, speed + turn};
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
