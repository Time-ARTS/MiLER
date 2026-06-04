#include "config.hpp"
#include "shortKeyCommands/shortKeyCommands.hpp"
#include "loop.hpp"

#include "vision/vision.hpp"
#include "vision/debugRenderer.hpp"

#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

#include "vision/perspectiveTuner.hpp"

#ifdef __linux__
#include <wiringPi.h>
#include <softPwm.h>
#endif

const std::string config_path = "../config.json";

int main()
{
    try
    {
        ConfigService::init(config_path);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Initiation failed: " << e.what() << std::endl;

        waitForUserIfInteractive();
        std::exit(EXIT_FAILURE);
    }

    try
    {
        const auto& config = ConfigService::get();
        Loop loop;

        std::atomic running{false};

        bool searchWindowOpen = false;
        bool lineDetected = false;

#ifdef __linux__
        wiringPiSetupGpio();

        pinMode(config.button.start_stop, INPUT);
        pullUpDnControl(config.button.start_stop, PUD_UP);

        pinMode(config.motor_pins.stby, OUTPUT);

        pinMode(config.motor_pins.pwma_l, OUTPUT);
        pinMode(config.motor_pins.ai1, OUTPUT);
        pinMode(config.motor_pins.ai2, OUTPUT);

        pinMode(config.motor_pins.pwmb_r, OUTPUT);
        pinMode(config.motor_pins.bi1, OUTPUT);
        pinMode(config.motor_pins.bi2, OUTPUT);

        if (softPwmCreate(config.motor_pins.pwma_l, 0, 255) != 0)
            std::cerr << "Failed to create PWM on EN_LEFT\n";

        if (softPwmCreate(config.motor_pins.pwmb_r, 0, 255) != 0)
            std::cerr << "Failed to create PWM on EN_RIGHT\n";

        loop.motorTest();
#endif

        const auto programStartTime = std::chrono::steady_clock::now();
        auto lastActivity = programStartTime;

        const auto keys = loop.getShortKeyCommands();

        keys->bind(config.keys.start, [&running, &lastActivity, config]
        {
            running = !running;

            lastActivity = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(config.timing.button_debounce_ms));
        });

        keys->bind(config.keys.perspective, [&loop]
        {
            PerspectiveTuner::open(*loop.getVision());
        });

        keys->bind(config.keys.error_test, []
        {
            throw std::runtime_error("Error catch test");
        });

        keys->bind(config.keys.quit, []
        {
            MotorController::turnOffMotors();

            std::exit(EXIT_SUCCESS);
        });

        for (const auto& [key_, flag_, name_] : config.modules.toggleTable)
        {
            keys->bind(key_, [flag = flag_]
            {
                *flag = !*flag;
            });
        }

#ifdef __linux__
        keys->addTrigger(config.keys.start, [&]
        {
            return (digitalRead(config.button.start_stop) == (config.button.active_low ? LOW : HIGH));
        });
#endif
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

                if (config.modules.debug)
                    DebugRenderer::closeWindow();
            }

            if (now - programStartTime > std::chrono::minutes(config.timing.max_system_runtime_min))
            {
                exit(EXIT_SUCCESS);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;

        waitForUserIfInteractive();
        std::exit(EXIT_FAILURE);
    }
}
