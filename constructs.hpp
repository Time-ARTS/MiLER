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
    float x = 0.f;
    float y = 0.f;

    bool operator==(const Vector2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vector2& o) const { return !(*this == o); }

    Vector2 operator+(const Vector2& o) const { return {x + o.x, y + o.y}; }
    Vector2 operator-(const Vector2& o) const { return {x - o.x, y - o.y}; }
    Vector2 operator-()                 const { return {-x, -y}; }
};

static constexpr Vector2 INVALID_POINT = { -1.f, -1.f };

struct ToggleBinding
{
    std::string key;
    bool* flag;
    const char* name;
};

struct Config
{
    // --- Button ---
    struct Button
    {
        int start_stop;
        bool active_low;
    } button{};

    // --- History ---
    struct History
    {
        float temp_duration;
    } history{};

    // --- Keys ---
    struct keys
    {
        std::string error_test,
             perspective,
             quit,
             start;
    } keys;

    // --- Line control ---
    struct LineControl
    {
        int steps_count;

        double angle_gain,
               curvature_gain,
               offset_gain,
               weight_center,
               weight_sigma;
    } line_control{};

    // --- Modules ---
    struct Modules
    {
        std::vector<ToggleBinding> toggleTable;

        bool debug,
             driving,
             logging,
             mirroring,
             vision;
    } modules;

    // --- Motor ---
    struct Motor
    {
        double default_speed,
               max_speed,
               min_speed;

        int steering_count;
    } motor{};

    // --- Motor pins ---
    struct MotorPins
    {
        int ai1,
            ai2,
            bi1,
            bi2,
            pwma_l,
            pwmb_r,
            stby;
    } motor_pins{};

    // --- Perspective ---
    struct Perspective
    {
        bool enabled;
        double trapezoid_bottom_width;
        double trapezoid_top_width;
        double vanishing_y;
    } perspective{};

    // --- PID ---
    struct PID
    {
        double kp,
               ki,
               kd,
               int_limit;
    } pid{};

    // --- Quadrant ---
    struct Quadrant
    {
        int num;
        float offset_y,
              spacing_y;
    } quadrant{};

    // --- Screen ---
    struct Screen
    {
        int width,
            height,
            center_x,
            center_y;
    } screen{};

    // --- Timing ---
    struct Timing
    {
        int backtrack_duration_ms,
            backtrack_wait_time_ms,
            button_debounce_ms,
            button_poll_interval_ms,
            max_system_runtime_min,
            max_wait_time_s,
            robot_loop_interval_ms,
            sleep_time_s;
    } timing{};

    // --- Vision ---
    struct Vision
    {
        int threshold_min,
            threshold_max,
            max_test_attempts;

        double color_threshold,
               min_line_area;
    } vision{};
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

inline void waitForUserIfInteractive()
{
    if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO))
    {
        std::cerr << "\nPress ENTER to exit...";
        std::cin.get();
    }
}

#endif //MILLER_CONSTRUCTS_HPP
