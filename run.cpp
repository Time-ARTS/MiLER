#include "vision/vision.hpp"
#include "loop.hpp"
#include "discordMessaging/discordMessaging.hpp"
#include "settings.hpp"

#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#endif

#ifdef __linux__
#include <wiringPi.h>
#include <softPwm.h>
#endif

int main()
{
    DiscordMessenger::init();

    try
    {
        using namespace SETTINGS::CONTROL;

        Loop loop;

        std::atomic running{false};

        bool buttonPressed = false;
        bool lineDetected = false;

#ifdef __linux__
        wiringPiSetupGpio();

        pinMode(BUTTON::START_STOP_PIN, INPUT);
        pullUpDnControl(BUTTON::START_STOP_PIN, PUD_UP);

        pinMode(SETTINGS::LED_PINS::ACTIVE_LED, OUTPUT);
        pinMode(SETTINGS::LED_PINS::ACTIVE_LED_LEFT, OUTPUT);
        pinMode(SETTINGS::LED_PINS::ACTIVE_LED_RIGHT, OUTPUT);

        pinMode(SETTINGS::MOTOR::PIN::EN_LEFT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN1_LEFT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN2_LEFT, OUTPUT);

        pinMode(SETTINGS::MOTOR::PIN::EN_RIGHT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN1_RIGHT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN2_RIGHT, OUTPUT);

        softPwmCreate(SETTINGS::MOTOR::PIN::EN_LEFT, 0, 255);
        softPwmCreate(SETTINGS::MOTOR::PIN::EN_RIGHT, 0, 255);
#endif

        const auto programStartTime = std::chrono::steady_clock::now();
        auto lastActivity = programStartTime;

        DiscordMessenger::sendMessageToDiscord("System ready. Waiting for start.", MessageType::INFO);

        while (true)
        {
#ifdef _WIN32
            buttonPressed = false;
            if (_kbhit())
            {
                switch (static_cast<char>(tolower(_getch())))
                {
                case BUTTON::KEY::START:
                    buttonPressed = true;
                    break;

                case BUTTON::KEY::QUIT:
                    DiscordMessenger::sendMessageToDiscord("Forced shutdown.", MessageType::WARNING);
                    std::exit(EXIT_SUCCESS);
                default: ;
                }
            }
#endif

#ifdef __linux__
            buttonPressed = (digitalRead(BUTTON::START_STOP_PIN) == (BUTTON::ACTIVE_LOW ? LOW : HIGH));
#endif

            if (buttonPressed)
            {
                running = !running;

#ifdef __linux__
                digitalWrite(SETTINGS::LED_PINS::ACTIVE_LED, running ? HIGH : LOW);
#endif

                DiscordMessenger::sendMessageToDiscord(
                    running ? "Robot started. Searching for line..." : "Robot stopped.",
                    MessageType::SUCCESS
                );

                lastActivity = std::chrono::steady_clock::now();
                std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::DEBOUNCE_MS));
            }

            if (running && MODULE::ENABLE_VISION)
            {
                if (const auto vision = loop.getVision())
                {
                    lineDetected = vision->detectedLine();
                }
            }

            if (running && lineDetected && MODULE::ENABLE_DRIVING)
            {
                lastActivity = std::chrono::steady_clock::now();
                loop.run();
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::POLL_INTERVAL_MS));
            }

            const auto now = std::chrono::steady_clock::now();

            if (running && !lineDetected && now - lastActivity > TIME::SLEEP_TIME)
            {
                running = false;

#ifdef __linux__
                digitalWrite(SETTINGS::LED_PINS::ACTIVE_LED, LOW);
#endif

                DiscordMessenger::sendMessageToDiscord(
                    "Line not found in time. Soft stop.",
                    MessageType::WARNING
                );
            }

#ifdef _WIN32
            if (now - programStartTime > TIME::MAX_SYSTEM_RUNTIME)
            {
                DiscordMessenger::sendMessageToDiscord("Max runtime reached. Exiting.", MessageType::WARNING);
                exit(EXIT_SUCCESS);
            }
#endif

#ifdef __linux__
            if (now - programStartTime > TIME::MAX_SYSTEM_RUNTIME)
            {
                DiscordMessenger::sendMessageToDiscord("Max runtime reached. Shutting down.", MessageType::WARNING);

                digitalWrite(SETTINGS::LED_PINS::ACTIVE_LED, LOW);
                digitalWrite(SETTINGS::LED_PINS::ACTIVE_LED_LEFT, LOW);
                digitalWrite(SETTINGS::LED_PINS::ACTIVE_LED_RIGHT, LOW);

                exit(EXIT_SUCCESS);
            }
#endif
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        DiscordMessenger::sendMessageToDiscord(std::string("Fatal error: ") + e.what(), MessageType::ERROR);
        std::exit(EXIT_FAILURE);
    }

    DiscordMessenger::shutdown();
}
