#ifndef MILLER_CONSTANTS_HPP
#define MILLER_CONSTANTS_HPP

#pragma once

#include <chrono>
#include <opencv2/opencv.hpp>
#include "constructs.hpp"

namespace SETTINGS
{
    namespace PHYSICAL
    {
        constexpr float WEIGHT = 0.5f; // KG
        constexpr float WHEEL_RADIUS = 1.f; // cm
        constexpr float WHEELBASE = 25.f; // cm
    }

    namespace SCREEN
    {
        constexpr int CENTER_X = 320;
        constexpr int CENTER_Y = 480;
        constexpr float THETA = 0.0f;

        const cv::Point CENTER_TOP = { CENTER_X, 0};
        const cv::Point CENTER_BOTTOM = { CENTER_X, CENTER_Y};
    }

    // --- CONTROL / RUNTIME ---
    namespace CONTROL
    {
        // --- MODULE FLAGS ---
        namespace MODULE
        {
            inline bool ENABLE_DRIVING = true;
            inline bool ENABLE_VISION = true;
            inline bool ENABLE_DISCORD = false;
            inline bool ENABLE_LOGGING = true;
            inline bool ENABLE_DEBUG = false;
            inline bool ENABLE_MIRRORING = true;
            inline bool ENABLE_STEERING_COUNTER = false;
        }

        // --- TIMING ---
        namespace TIME
        {
            constexpr auto SLEEP_TIME = std::chrono::seconds(60);
            constexpr auto BACKTRACK_DURATION = std::chrono::milliseconds(10);
            constexpr auto MAX_WAIT_TIME = std::chrono::seconds(10);
            constexpr auto MAX_SYSTEM_RUNTIME = std::chrono::minutes(5);
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
                constexpr char ERROR_TEST = 'e';

                namespace TOGGLES
                {
                    constexpr char DRIVING = '0';
                    constexpr char VISION = '1';
                    constexpr char DISCORD = '2';
                    constexpr char LOGGING = '3';
                    constexpr char DEBUG = '4';
                    constexpr char MIRRORING = '5';
                    constexpr char STEERING_COUNTER = '6';

                    static ToggleBinding TOGGLE_TABLE[] = {
                        {DRIVING, &MODULE::ENABLE_DRIVING, "Toggle Driving"},
                        {VISION, &MODULE::ENABLE_VISION, "Toggle Vision"},
                        {DISCORD, &MODULE::ENABLE_DISCORD, "Toggle Discord"},
                        {LOGGING, &MODULE::ENABLE_LOGGING, "Toggle Logging"},
                        {DEBUG, &MODULE::ENABLE_DEBUG, "Toggle Debug View"},
                        {MIRRORING, &MODULE::ENABLE_MIRRORING, "Toggle Camera Mirroring"},
                        {STEERING_COUNTER, &MODULE::ENABLE_STEERING_COUNTER, "Toggle Steering Counter"}
                    };
                }
            }
        }
    }

    // --- GPIO / HARDWARE ---
    namespace MOTOR
    {
        constexpr double MAX_SPEED = 1.0f;
        constexpr double MIN_SPEED = -1.0f;
        constexpr double DEFAULT_SPEED = 1.0f;
        constexpr int STEERING_COUNT = 50;

        namespace PIN
        {
            constexpr int EN_LEFT = 22;
            constexpr int IN1_LEFT = 17;
            constexpr int IN2_LEFT = 27;

            constexpr int EN_RIGHT = 25;
            constexpr int IN1_RIGHT = 23;
            constexpr int IN2_RIGHT = 24;

            constexpr int STANDBY = 26;
        }
    }

    // --- VISION / CV ---
    namespace VISION
    {
        constexpr int MAX_VIDEO_DEVICES = 40;

#ifdef __linux__
        constexpr auto CAMERA_INDEX = "libcamerasrc ! "
        "video/x-raw,width=640,height=480,framerate=30/1 ! "
        "videoconvert ! "
        "appsink";

        constexpr auto CAMERA_SOURCE = cv::CAP_GSTREAMER;
#elif defined(_WIN32)
        constexpr int CAMERA_INDEX = 0;
        constexpr auto CAMERA_SOURCE = cv::CAP_DSHOW;
#else
        constexpr int CAMERA_INDEX = 0;
        constexpr auto CAMERA_SOURCE = cv::CAP_ANY;
#endif

        constexpr int MAX_TEST_ATTEMPTS = 10;

        constexpr float COLOR_THRESHOLD = 0.80f;

        constexpr int THRESHOLD_MIN = 100;
        constexpr int THRESHOLD_MAX = 255;
        constexpr float MIN_LINE_AREA = 0.0f;
    }

    // --- IMAGE PROCESSING ---
    namespace QUADRANT
    {
        constexpr int NUM = 24;
        constexpr float OFFSET_Y = 0.0f;
        constexpr float SPACING_Y = 15.0f;

        constexpr float CIRCLE_RADIUS = 50.0f;

        // BGR not RGB (Blue, Green, Red)
        inline cv::Scalar RENDER_QUADRANT{184, 169, 245};
        inline cv::Scalar RENDER_POINT{250, 206, 91};
        inline cv::Scalar RENDER_CENTERLINE{91, 206, 250};
        inline cv::Scalar RENDER_LINE{0, 255, 0};
        inline cv::Scalar RENDER_MOTOR_LINE{0, 0, 255};
    }

    namespace HISTORY
    {
        constexpr double TEMP_DURATION = 2.0;
    }

    namespace LINE_CONTROL
    {
        inline const std::vector<double> DEFAULT_CENTERLINE(480, 320);
        constexpr int STEPS_COUNT = 200;

        constexpr double CURVATURE_GAIN = 1;
    }

    namespace PID
    {
        constexpr double KP = 0.5;
        constexpr double KI = 0.0;
        constexpr double KD = 0.1;
    }

    namespace SPEED_GOVERNOR
    {
        constexpr float CURVATURE_GAIN = 0.5f;
    }
}

#endif // MILLER_CONSTANTS_HPP
