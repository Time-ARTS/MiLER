#ifndef MILLER_CONSTANTS_HPP
#define MILLER_CONSTANTS_HPP

#pragma once
#include <chrono>
#include <opencv2/opencv.hpp>

#include "constructs.hpp"

/*
Titles : // --- Title ---
SubTitles : // SUBTITLE
Descriptions : // Description of what is here
Comment : // ** Ignore **
*/

namespace SETTINGS {
    // --- CONTROL / RUNTIME SETTINGS ---
    namespace CONTROL {

        namespace TIME {
            constexpr auto SLEEP_TIME = std::chrono::seconds(60);
            constexpr auto BACKTRACK_DURATION = std::chrono::milliseconds(10);
            constexpr auto MAX_WAIT_TIME = std::chrono::seconds(10);
            constexpr auto MAX_SYSTEM_RUNTIME = std::chrono::minutes(5);
        }

        // --- BUTTON SETTINGS ---
        namespace BUTTON {
            constexpr int START_STOP_PIN = 0;        // WiringPi pin for start/stop
            constexpr int DEBOUNCE_MS = 500;         // Debounce duration for button presses
            constexpr int POLL_INTERVAL_MS = 50;     // How often to check the button
            constexpr bool ACTIVE_LOW = true;        // true if button connects to GND when pressed

            namespace KEY {
                constexpr auto START = 's';
                constexpr auto QUIT = 'q';
            }
        }

        // --- MODULE ENABLE FLAGS ---
        namespace MODULE {
            constexpr bool ENABLE_DRIVING = true;    // Enable motor/driving logic
            constexpr bool ENABLE_DISCORD = true;    // Enable Discord messaging
            constexpr bool ENABLE_VISION = true;     // Enable vision processing
            constexpr bool ENABLE_LOGGING = true;    // Enable console logging
        }

        // --- THREAD SETTINGS ---
        namespace THREAD {
            constexpr int ROBOT_LOOP_INTERVAL_MS = 30; // Delay between robot loop cycles
        }

        // --- DISCORD SETTINGS ---
        namespace DISCORD {
            constexpr char WEBHOOK_URL[] = "https://discord.com/api/webhooks/1461702296311169086/""XsXPRHTLegvDMEkyxFg5xk3tF9MWm8G-_yrDhkDkkw5oGfygZH8HfFwugaMunoYeYan2";  // Discord webhook
            constexpr int RETRY_DELAY_MS = 2000;                     // Retry delay if send fails
        }
    }

    namespace LED_PINS {
        constexpr int ACTIVE_LED = 17;
        constexpr int ACTIVE_LED_LEFT = 16;
        constexpr int ACTIVE_LED_RIGHT = 26;
    }

    // --- VISION / PROCESSING SETTINGS ---
    namespace VISION {
#ifdef __linux__
            constexpr int  CAMERA_INDEX  = 0;
            constexpr auto CAMERA_SOURCE = cv::CAP_V4L2;
#elif defined(_WIN32)
            constexpr int  CAMERA_INDEX  = 0;
            constexpr auto CAMERA_SOURCE = cv::CAP_DSHOW;
#else
            constexpr int  CAMERA_INDEX  = 0;
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

    // --- QUADRANT SETTINGS ---
    namespace QUADRANT {
        constexpr int NUM = 3;
        constexpr float OFFSET_Y = 0.0f;
        constexpr float SPACING_Y = 0.0f;
    }

    // --- LINE THRESHOLD SETTINGS ---
    namespace LINE_THRESHOLD {
        constexpr int MIN = 50;
        constexpr int MAX = 255;
        constexpr int CANNY_LOW = 50;
        constexpr int CANNY_HIGH = 150;
    }

    // --- HISTORY SETTINGS ---
    namespace HISTORY {
        constexpr double TEMP_DURATION = 2.0;
    }

    // --- PID SETTINGS ---
    namespace PID {
        constexpr double KP = 0.5;
        constexpr double KI = 0.0;
        constexpr double KD = 0.1;
    }

    // --- MOTOR SETTINGS ---
    namespace MOTOR {
        constexpr int MAX_SPEED = 255;
        constexpr int MIN_SPEED = -255;
        constexpr int DEFAULT_SPEED = 150;

        // PIN ASSIGNMENTS
        namespace PIN {
            constexpr int EN_LEFT = 22;
            constexpr int IN1_LEFT = 17;
            constexpr int IN2_LEFT = 27;

            constexpr int EN_RIGHT = 25;
            constexpr int IN1_RIGHT = 23;
            constexpr int IN2_RIGHT = 24;
        }
    }
}

#endif //MILLER_CONSTANTS_HPP
