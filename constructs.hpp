#ifndef MILLER_CONSTRUCTS_HPP
#define MILLER_CONSTRUCTS_HPP

#pragma once
#include <chrono>
#include <functional>
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

struct ToggleBinding
{
    char key;
    bool* flag;
    const char* name;
};

struct Config
{
    struct Modules
    {
        std::vector<ToggleBinding> toggleTable;

        bool driving,
             vision,
             discord,
             logging,
             debug,
             mirroring,
             steering_counter;
    } modules;

    struct Motor
    {
        double default_speed,
               min_speed,
               max_speed;

        int steering_count;
    } motor;

    struct PID
    {
        double kp, ki, kd, int_limit;
    } pid;

    struct Vision
    {
        int threshold_min,
            threshold_max,
            max_test_attempts;

        double color_threshold,
               min_line_area;
    } vision;

    struct LineControl
    {
        int steps_count;
        double weight_center,
               weight_sigma,
               offset_gain,
               angle_gain,
               curvature_gain;
    } line_control;

    struct Quadrant
    {
        int num;
        float offset_y,
              spacing_y;
    } quadrant;

    struct Perspective
    {
        bool enabled;
        double vanishing_y;
        double trapezoid_top_width;
        double trapezoid_bottom_width;
    } perspective;

    struct History
    {
        float temp_duration;
    } history;

    struct Timing
    {
        int robot_loop_interval_ms,
            button_debounce_ms,
            button_poll_interval_ms,
            backtrack_duration_ms,
            sleep_time_s,
            max_wait_time_s,
            max_system_runtime_min;
    } timing;
};

struct Trigger
{
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
