#include "vision/vision.hpp"
#include "loop.hpp"
#include <thread>
#include <chrono>

int main() {
    while (true) {
        if (Vision.detectedLine()) {
            runLoop();
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
}