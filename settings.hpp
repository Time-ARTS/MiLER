#ifndef MILLER_CONSTANTS_HPP
#define MILLER_CONSTANTS_HPP

#pragma once

#include <chrono>

namespace SETTINGS
{
    inline std::string CONFIG_PATH = "..\\config.json";

    namespace PHYSICAL
    {
        constexpr float WEIGHT = 0.5f;
        constexpr float WHEEL_RADIUS = 1.f;
        constexpr float WHEELBASE = 25.f;
    }

    namespace MOTOR::PIN
    {
        constexpr int EN_LEFT = 22;
        constexpr int IN1_LEFT = 17;
        constexpr int IN2_LEFT = 27;

        constexpr int EN_RIGHT = 25;
        constexpr int IN1_RIGHT = 23;
        constexpr int IN2_RIGHT = 24;

        constexpr int STANDBY = 26;
    }

    namespace BUTTON
    {
        constexpr int START_STOP_PIN = 12;
        constexpr bool ACTIVE_LOW = true;
    }

    namespace KEY
    {
        constexpr char START = 'r';
        constexpr char QUIT = 'q';
        constexpr char ERROR_TEST = 'e';
        constexpr char PERSPECTIVE = 'p';
    }

    namespace SCREEN
    {
        constexpr int WIDTH = 640;
        constexpr int HEIGHT = 480;

        constexpr int CENTER_X = WIDTH / 2;
        constexpr int CENTER_Y = HEIGHT;
    }
}

#endif // MILLER_CONSTANTS_HPP
