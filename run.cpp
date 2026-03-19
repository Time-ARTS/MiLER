#include "settings.hpp"
#include "shortKeyCommands/shortKeyCommands.hpp"
#include "loop.hpp"

#include "vision/vision.hpp"
#include "vision/debugRenderer.hpp"

#include "discordMessaging/discordMessaging.hpp"

#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

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

        bool searchWindowOpen = false;
        bool lineDetected = false;

#ifdef __linux__
        wiringPiSetupGpio();

        pinMode(BUTTON::START_STOP_PIN, INPUT);
        pullUpDnControl(BUTTON::START_STOP_PIN, PUD_UP);

        pinMode(SETTINGS::MOTOR::PIN::STANDBY, OUTPUT);

        pinMode(SETTINGS::MOTOR::PIN::EN_LEFT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN1_LEFT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN2_LEFT, OUTPUT);

        pinMode(SETTINGS::MOTOR::PIN::EN_RIGHT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN1_RIGHT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN2_RIGHT, OUTPUT);

        if (softPwmCreate(SETTINGS::MOTOR::PIN::EN_LEFT, 0, 255) != 0)
            DiscordMessenger::sendMessageToDiscord("Failed to create PWM on EN_LEFT", MessageType::ERROR);
        if (softPwmCreate(SETTINGS::MOTOR::PIN::EN_RIGHT, 0, 255) != 0)
            DiscordMessenger::sendMessageToDiscord("Failed to create PWM on EN_RIGHT", MessageType::ERROR);

        digitalWrite(SETTINGS::MOTOR::PIN::STANDBY, HIGH);

        loop.motorTest();
#endif

        const auto programStartTime = std::chrono::steady_clock::now();
        auto lastActivity = programStartTime;

        const auto keys = loop.getShortKeyCommands();

        keys->bind(BUTTON::KEY::START, [&running, &lastActivity]
        {
            running = !running;

            DiscordMessenger::sendMessageToDiscord(
                running ? "Robot started. Searching for line..." : "Robot stopped.",
                MessageType::SUCCESS
            );

            lastActivity = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::DEBOUNCE_MS));
        }, "Start");

        keys->bind(BUTTON::KEY::ERROR_TEST, []
        {
            throw std::runtime_error("Error catch test");
        });

        keys->bind(BUTTON::KEY::QUIT, []
        {
            DiscordMessenger::sendMessageToDiscord("Forced shutdown.", MessageType::WARNING);
            std::exit(EXIT_SUCCESS);
        }, "Quit");

        for (const auto& [key_, flag_, name_] : BUTTON::KEY::TOGGLES::TOGGLE_TABLE)
        {
            keys->bind(key_, [flag = flag_, name = name_]
            {
                *flag = !*flag;

                DiscordMessenger::sendMessageToDiscord(std::string(name) + ": " + (*flag ? "ON" : "OFF"));
            }, name_);
        }

#ifdef __linux__
        keys->addTrigger(BUTTON::KEY::START, [&]
        {
            return (digitalRead(BUTTON::START_STOP_PIN) == (BUTTON::ACTIVE_LOW ? LOW : HIGH));
        });
#endif

        keys->flushBindingReport();

        DiscordMessenger::sendMessageToDiscord("System ready. Waiting for start.", MessageType::INFO);

        while (true)
        {
            keys->poll();

            if (running && MODULE::ENABLE_VISION)
            {
                if (const auto vision = loop.getVision())
                {
                    lineDetected = vision->detectedLine();

                    if (MODULE::ENABLE_DEBUG)
                    {
                        if (!MODULE::ENABLE_DRIVING || !lineDetected)
                        {
                            if (!searchWindowOpen)
                            {
                                DebugRenderer::openWindow();
                                searchWindowOpen = true;
                            }
                        }
                        else
                        {
                            if (searchWindowOpen)
                            {
                                DebugRenderer::closeWindow();
                                searchWindowOpen = false;
                            }
                        }
                    }
                }
            }

            if (running && lineDetected && MODULE::ENABLE_DRIVING)
            {
                lastActivity = std::chrono::steady_clock::now();

                DiscordMessenger::sendMessageToDiscord("Line detected. Starting loop...", MessageType::SUCCESS);
                loop.run(running);
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::POLL_INTERVAL_MS));
            }

            const auto now = std::chrono::steady_clock::now();

            if (running && !lineDetected && now - lastActivity > TIME::SLEEP_TIME)
            {
                running = false;
                searchWindowOpen = false;

                DiscordMessenger::sendMessageToDiscord(
                    "Line not found in time. Soft stop.",
                    MessageType::WARNING
                );

                if (MODULE::ENABLE_DEBUG)
                    DebugRenderer::closeWindow();
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
                exit(EXIT_SUCCESS);
            }
#endif
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        DiscordMessenger::sendMessageToDiscord(std::string("Fatal error: ") + e.what(), MessageType::ERROR);

        waitForUserIfInteractive();

        std::exit(EXIT_FAILURE);
    }

    DiscordMessenger::shutdown();
}
