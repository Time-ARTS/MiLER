#ifndef MILLER_CONSTANTS_HPP
#define MILLER_CONSTANTS_HPP

#pragma once

#include <chrono>
#include <opencv2/opencv.hpp>
#include "constructs.hpp"

namespace SETTINGS
{
    // --- CONTROL / RUNTIME ---
    namespace CONTROL
    {
        // --- TIMING ---
        namespace TIME
        {
            constexpr auto SLEEP_TIME = std::chrono::seconds(60);
            constexpr auto BACKTRACK_DURATION = std::chrono::milliseconds(10);
            constexpr auto MAX_WAIT_TIME = std::chrono::seconds(10);
            constexpr auto MAX_SYSTEM_RUNTIME = std::chrono::minutes(5);
        }

        // --- BUTTON ---
        namespace BUTTON
        {
            constexpr int START_STOP_PIN = 12;
            constexpr int DEBOUNCE_MS = 500;
            constexpr int POLL_INTERVAL_MS = 50;
            constexpr bool ACTIVE_LOW = true;

            namespace KEY
            {
                constexpr char START = 'r';
                constexpr char QUIT = 'q';
            }
        }

        // --- MODULE FLAGS ---
        namespace MODULE
        {
            constexpr bool ENABLE_DRIVING = true;
            constexpr bool ENABLE_VISION = true;
            constexpr bool ENABLE_DISCORD = false;
            constexpr bool ENABLE_LOGGING = true;
        }

        // --- DEBUGGING ---
        namespace DEBUG
        {
            constexpr bool ENABLE_DEBUG_VIEW = true;
        }

        // --- THREAD ---
        namespace THREAD
        {
            constexpr int ROBOT_LOOP_INTERVAL_MS = 30;
        }

        // --- DISCORD ---
        namespace DISCORD
        {
            constexpr char WEBHOOK_URL[] = "https://discord.com/api/webhooks/1461702296311169086/"
                "XsXPRHTLegvDMEkyxFg5xk3tF9MWm8G-_yrDhkDkkw5oGfygZH8HfFwugaMunoYeYan2";
            constexpr int RETRY_DELAY_MS = 2000;
        }
    }

    // --- GPIO / HARDWARE ---
    namespace LED_PINS
    {
        constexpr int ACTIVE_LED = 17;
        constexpr int ACTIVE_LED_LEFT = 16;
        constexpr int ACTIVE_LED_RIGHT = 26;
    }

    namespace MOTOR
    {
        constexpr int MAX_SPEED = 255;
        constexpr int MIN_SPEED = -255;
        constexpr int DEFAULT_SPEED = 150;

        namespace PIN
        {
            constexpr int EN_LEFT = 22;
            constexpr int IN1_LEFT = 17;
            constexpr int IN2_LEFT = 27;

            constexpr int EN_RIGHT = 25;
            constexpr int IN1_RIGHT = 23;
            constexpr int IN2_RIGHT = 24;
        }
    }

    // --- VISION / CV ---
    namespace VISION
    {
#ifdef __linux__
        constexpr int CAMERA_INDEX = 0;
        constexpr auto CAMERA_SOURCE = cv::CAP_V4L2;
#elif defined(_WIN32)
        constexpr int CAMERA_INDEX = 0;
        constexpr auto CAMERA_SOURCE = cv::CAP_DSHOW;
#else
        constexpr int CAMERA_INDEX = 0;
        constexpr auto CAMERA_SOURCE = cv::CAP_ANY;
#endif

        constexpr int GAUSSIAN_BLUR_KERNEL = 5;
        constexpr double GAUSSIAN_SIGMA = 0.0;

        constexpr double HOUGH_RHO = 1.0;
        constexpr double HOUGH_THETA = PI / 180.0;
        constexpr int HOUGH_THRESHOLD = 50;
        constexpr int HOUGH_MIN_LINE = 50;
        constexpr int HOUGH_MAX_GAP = 10;
    }

    // --- IMAGE PROCESSING ---
    namespace QUADRANT
    {
        constexpr int NUM = 12;
        constexpr float OFFSET_Y = 0.0f;
        constexpr float SPACING_Y = 25.0f;
    }

    namespace LINE_THRESHOLD
    {
        constexpr int MIN = 50;
        constexpr int MAX = 255;
        constexpr int CANNY_LOW = 50;
        constexpr int CANNY_HIGH = 150;
    }

    namespace HISTORY
    {
        constexpr double TEMP_DURATION = 2.0;
    }

    // --- CONTROL / PID ---
    namespace PID
    {
        constexpr double KP = 0.5;
        constexpr double KI = 0.0;
        constexpr double KD = 0.1;
    }
}

#endif // MILLER_CONSTANTS_HPP
