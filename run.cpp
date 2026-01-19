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

int main() {
    using namespace SETTINGS::CONTROL;

    std::atomic<bool> running{false};
    DiscordMessenger messenger;

#ifdef __linux__
    wiringPiSetup();
    pinMode(BUTTON::START_STOP_PIN, INPUT);
    if (BUTTON::ACTIVE_LOW) pullUpDnControl(BUTTON::START_STOP_PIN, PUD_UP);
    else pullUpDnControl(BUTTON::START_STOP_PIN, PUD_DOWN);
#endif

    messenger.sendMessageToDiscord("System ready, waiting for start button...", MessageType::INFO);

    while (true) {
        bool buttonPressed = false;

#ifdef __linux__
        buttonPressed = (digitalRead(BUTTON::START_STOP_PIN) == (BUTTON::ACTIVE_LOW ? LOW : HIGH));
#endif

        if (buttonPressed) {
            running = !running; 
            messenger.sendMessageToDiscord(running ? "Robot started." : "Robot stopped.",
                                           running ? MessageType::SUCCESS : MessageType::WARNING);
            std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::DEBOUNCE_MS));
        }

        if (running && MODULE::ENABLE_DRIVING) {
            runLoop();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(BUTTON::POLL_INTERVAL_MS));
        }
    }
}
