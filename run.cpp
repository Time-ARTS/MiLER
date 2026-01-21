#include "vision/vision.hpp"
#include "loop.hpp"
#include "discordMessaging/discordMessaging.hpp"
#include "settings.hpp"

#include <thread>
#include <chrono>
#include <atomic>

#ifdef _WIN32
#include <conio.h>
#endif

#ifdef __linux__
#include <wiringPi.h>
#endif

int main() {
    try {
        using namespace SETTINGS::CONTROL;

        Loop loop;

        std::atomic running{false};

#ifdef __linux__
        wiringPiSetupGpio();

        // Button
        pinMode(SETTINGS::CONTROL::BUTTON::START_STOP_PIN, INPUT);
        pullUpDnControl(SETTINGS::CONTROL::BUTTON::START_STOP_PIN, PUD_UP);

        // LEDs
        pinMode(SETTINGS::LED_PINS::ACTIVE_LED, OUTPUT);
        pinMode(SETTINGS::LED_PINS::ACTIVE_LED_LEFT, OUTPUT);
        pinMode(SETTINGS::LED_PINS::ACTIVE_LED_RIGHT, OUTPUT);

        // Motors
        pinMode(SETTINGS::MOTOR::PIN::EN_LEFT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN1_LEFT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN2_LEFT, OUTPUT);

        pinMode(SETTINGS::MOTOR::PIN::EN_RIGHT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN1_RIGHT, OUTPUT);
        pinMode(SETTINGS::MOTOR::PIN::IN2_RIGHT, OUTPUT);

        // Enable software PWM
        softPwmCreate(SETTINGS::MOTOR::PIN::EN_LEFT,  0, 255);
        softPwmCreate(SETTINGS::MOTOR::PIN::EN_RIGHT, 0, 255);

        pinMode(BUTTON::START_STOP_PIN, INPUT);
        pullUpDnControl(BUTTON::START_STOP_PIN, PUD_UP);

        buttonPressed = (digitalRead(BUTTON::START_STOP_PIN) == LOW);
#endif


        const auto programStartTime = std::chrono::steady_clock::now();
        auto lastActivity = programStartTime;

        DiscordMessenger::sendMessageToDiscord("System ready, waiting for start button...", MessageType::INFO);

        while (true) {
            bool buttonPressed = false;
            bool lineDetected = false;

#ifdef _WIN32
            if (_kbhit()) {
                switch (const auto key = _getch(); tolower(key)) {
                case BUTTON::KEY::START:
                    buttonPressed = true;
                    DiscordMessenger::sendMessageToDiscord(!running  ? "Started manually via keyboard." : "Stopped manually via keyboard", MessageType::DEBUG);
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

            if (buttonPressed) {
                running = !running;
                DiscordMessenger::sendMessageToDiscord(running ? "Robot started. Looking for line..." : "Robot stopped.", MessageType::SUCCESS);

                if (running && MODULE::ENABLE_VISION) {
                    if (const auto vision = loop.getVision()) {
                        lineDetected = vision->detectedLine();
                    }
                }

                lastActivity = std::chrono::steady_clock::now();

                std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::DEBOUNCE_MS));
            }

            if (running && lineDetected && MODULE::ENABLE_DRIVING) {
                lastActivity = std::chrono::steady_clock::now();

                loop.run();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::POLL_INTERVAL_MS));
            }

            const auto now = std::chrono::steady_clock::now();

            // === SOFT STOP: line not found in time ===
            if (running && !lineDetected && now - lastActivity > TIME::SLEEP_TIME) {

                running = false;
                DiscordMessenger::sendMessageToDiscord( "Line not found in time. Driving disabled (soft stop).", MessageType::WARNING );
                }

#ifdef _WIN32
            if (now - programStartTime > TIME::MAX_SYSTEM_RUNTIME) {
                DiscordMessenger::sendMessageToDiscord( "Max runtime reached. Stopping testing program.", MessageType::WARNING);
                exit(EXIT_SUCCESS);
            }
#endif

#ifdef __linux__
            // === HARD SHUTDOWN: RPi only ===
            if (now - programStartTime > TIME::MAX_SYSTEM_RUNTIME) {
                DiscordMessenger::sendMessageToDiscord( "Max runtime reached. Shutting down program.", MessageType::WARNING);
                //system("sudo shutdown -h now");
                exit(EXIT_SUCCESS);
            }
#endif
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        DiscordMessenger::sendMessageToDiscord(std::string("Fatal error: ") + e.what(), MessageType::ERROR);
        std::exit(EXIT_FAILURE);
    }
}
