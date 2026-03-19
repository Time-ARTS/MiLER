#ifndef MILLER_CONSTRUCTS_HPP
#define MILLER_CONSTRUCTS_HPP

#pragma once
#include <chrono>
#include <functional>
#include <string>
#include <iostream>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define STDIN_FILENO _fileno(stdin)
#define STDOUT_FILENO _fileno(stdout)
#else
#include <unistd.h>
#endif

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

struct MotorCommandEntry
{
    std::pair<double, double> speed;
    std::chrono::steady_clock::time_point timestamp;
};

enum class MessageType
{
    INFO,
    WARNING,
    ERROR,
    DEBUG,
    SUCCESS
};

inline void waitForUserIfInteractive()
{
    if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO))
    {
        std::cerr << "\nPress ENTER to exit...";
        std::cin.get();
    }
}

#endif //MILLER_CONSTRUCTS_HPP
