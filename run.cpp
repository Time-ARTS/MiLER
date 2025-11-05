#include "vision.hpp"
#include "loop.hpp"
#include <thread>
#include <chrono>

int main() {
    while (true) {
        if (vision::detectedLine()) {
            runLoop();
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
}