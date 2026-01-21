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

[[noreturn]] int main() {
    try {
        using namespace SETTINGS::CONTROL;

        Loop loop;

        std::atomic running{false};

#ifdef __linux__
        wiringPiSetup();
        pinMode(BUTTON::START_STOP_PIN, INPUT);
        if (BUTTON::ACTIVE_LOW) pullUpDnControl(BUTTON::START_STOP_PIN, PUD_UP);
        else pullUpDnControl(BUTTON::START_STOP_PIN, PUD_DOWN);
#endif

        const auto programStartTime = std::chrono::high_resolution_clock::now();
        auto lastActivity = programStartTime;

        DiscordMessenger::sendMessageToDiscord("System ready, waiting for start button...", MessageType::INFO);

        while (true) {
            bool buttonPressed = false;
            bool lineDetected = false;

#ifdef _WIN32
            if (_kbhit()) {
                switch (const char key = _getch(); tolower(key)) {
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

                lastActivity = std::chrono::high_resolution_clock::now();

                std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::DEBOUNCE_MS));
            }

            if (running && lineDetected && MODULE::ENABLE_DRIVING) {
                lastActivity = std::chrono::high_resolution_clock::now(); // update activity when driving
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
                DiscordMessenger::sendMessageToDiscord( "Max runtime reached. Shutting down Raspberry Pi.", MessageType::WARNING);
                system("sudo shutdown -h now");
            }
#endif
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        DiscordMessenger::sendMessageToDiscord(std::string("Fatal error: ") + e.what(), MessageType::ERROR);
        std::exit(EXIT_FAILURE);
    }
}
