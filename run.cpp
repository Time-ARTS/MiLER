#include "vision/vision.hpp"
#include "loop.hpp"
#include "components.cpp"
#include <thread>
#include <chrono>

int main() {
    while (true) {
        if (Vision vision; vision.detectedLine()) {
            runLoop();
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
}