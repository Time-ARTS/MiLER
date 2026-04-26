#include "config.hpp"
#include "settings.hpp"
#include "shortKeyCommands/shortKeyCommands.hpp"
#include "loop.hpp"

#include "vision/vision.hpp"
#include "vision/debugRenderer.hpp"

#include "discordMessaging/discordMessaging.hpp"
#include "discordMessaging/discordConfig.hpp"

#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

#include "vision/perspectiveTuner.hpp"

#ifdef __linux__
#include <wiringPi.h>
#include <softPwm.h>
#endif

int main()
{
    ConfigService::init(SETTINGS::CONFIG_PATH);
    DiscordMessenger::init(DiscordConfig::loadDiscordConfig(SETTINGS::CONFIG_PATH));

    try
    {
        const auto& config = ConfigService::get();
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

        loop.motorTest();
#endif

        const auto programStartTime = std::chrono::steady_clock::now();
        auto lastActivity = programStartTime;

        const auto keys = loop.getShortKeyCommands();

        keys->bind(SETTINGS::KEY::START, [&running, &lastActivity, config]
        {
            running = !running;

            DiscordMessenger::sendMessageToDiscord(
                running ? "Robot started. Searching for line..." : "Robot stopped.",
                MessageType::SUCCESS
            );

            lastActivity = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(config.timing.button_debounce_ms));
        }, "Start");

        keys->bind(SETTINGS::KEY::PERSPECTIVE, [&loop]
        {
            PerspectiveTuner::open(*loop.getVision());
        }, "Perspective tuner");

        keys->bind(SETTINGS::KEY::ERROR_TEST, []
        {
            throw std::runtime_error("Error catch test");
        }, "Error catch test");

        keys->bind(SETTINGS::KEY::QUIT, []
        {
            DiscordMessenger::sendMessageToDiscord("Forced shutdown.", MessageType::WARNING);

            MotorController::turnOffMotors();

            std::exit(EXIT_SUCCESS);
        }, "Quit");

        for (const auto& [key_, flag_, name_] : config.modules.toggleTable)
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

            if (running && config.modules.vision)
            {
                if (const auto vision = loop.getVision())
                {
                    lineDetected = vision->detectedLine();

                    if (config.modules.debug)
                    {
                        if (!config.modules.driving || !lineDetected)
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

            if (running && lineDetected && config.modules.driving)
            {
                lastActivity = std::chrono::steady_clock::now();

                DiscordMessenger::sendMessageToDiscord("Line detected. Starting loop...", MessageType::SUCCESS);
                MotorController::turnOnMotors();
                loop.run(running);
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.timing.button_poll_interval_ms));
            }

            const auto now = std::chrono::steady_clock::now();

            if (running && !lineDetected && now - lastActivity > std::chrono::seconds(config.timing.sleep_time_s))
            {
                running = false;
                searchWindowOpen = false;

                DiscordMessenger::sendMessageToDiscord(
                    "Line not found in time. Soft stop.",
                    MessageType::WARNING
                );

                if (config.modules.debug)
                    DebugRenderer::closeWindow();
            }

            if (now - programStartTime > std::chrono::minutes(config.timing.max_system_runtime_min))
            {
                DiscordMessenger::sendMessageToDiscord("Max runtime reached. Shutting down.", MessageType::WARNING);
                exit(EXIT_SUCCESS);
            }
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
