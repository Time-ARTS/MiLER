#include "vision/vision.hpp"
#include "loop.hpp"
#include "components.cpp"
#include "errorMessaging/error_msg.cpp"
#include <thread>
#include <chrono>

int main() {
    while (true) {
        sendMessageToDiscord("System started, checking for line...");
        if (Vision vision; vision.detectedLine()) {
            sendMessageToDiscord("Line detected, starting main loop.");
            runLoop();
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
}