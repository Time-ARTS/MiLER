#include "vision/vision.hpp"
#include "loop.hpp"
#include "discordMessaging/discordMessaging.hpp"
#include "settings.hpp"

#include <thread>
#include <chrono>
#include <atomic>

#ifdef __linux__
#include <wiringPi.h>
#endif

[[noreturn]] int main() {
    try {
        using namespace SETTINGS::CONTROL;

        std::atomic running{false};

#ifdef __linux__
        wiringPiSetup();
        pinMode(BUTTON::START_STOP_PIN, INPUT);
        if (BUTTON::ACTIVE_LOW) pullUpDnControl(BUTTON::START_STOP_PIN, PUD_UP);
        else pullUpDnControl(BUTTON::START_STOP_PIN, PUD_DOWN);
#endif

        auto lastActivity = std::chrono::high_resolution_clock::now();
        DiscordMessenger::sendMessageToDiscord("System ready, waiting for start button...", MessageType::INFO);

        while (true) {
            bool buttonPressed = false;
            bool lineDetected = false;

#ifdef __linux__
            buttonPressed = (digitalRead(BUTTON::START_STOP_PIN) == (BUTTON::ACTIVE_LOW ? LOW : HIGH));
#endif

            if (buttonPressed) {
                running = !running;
                DiscordMessenger::sendMessageToDiscord(running ? "Robot started. Looking for line..." : "Robot stopped.", MessageType::SUCCESS);

                if (running) {
                    Vision vision;
                    lineDetected = vision.detectedLine();
                    DiscordMessenger::sendMessageToDiscord(lineDetected ? "Line detected." : "Line not detected, please try again.",
                                                             lineDetected ? MessageType::SUCCESS : MessageType::ERROR);
                }

                // Update last activity timestamp
                lastActivity = std::chrono::high_resolution_clock::now();

                std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::DEBOUNCE_MS));
            }

            if (running && lineDetected && MODULE::ENABLE_DRIVING) {
                runLoop();
                lastActivity = std::chrono::high_resolution_clock::now(); // update activity when driving
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::POLL_INTERVAL_MS));
            }

            // Check for idle timeout
            auto now = std::chrono::high_resolution_clock::now();
            if (auto idleDuration = std::chrono::duration_cast<std::chrono::seconds>(now - lastActivity); idleDuration > TIME::SLEEP_TIME) {
                DiscordMessenger::sendMessageToDiscord("No activity detected for timeout period. Shutting down.", MessageType::ERROR);
                std::exit(EXIT_SUCCESS);
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        DiscordMessenger::sendMessageToDiscord(std::string("Fatal error: ") + e.what(), MessageType::ERROR);
        std::exit(EXIT_FAILURE);
    }
}
