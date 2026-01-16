#include "vision/vision.hpp"
#include "loop.hpp"
#include "components.cpp"
#include <thread>
#include <chrono>

int main() {
    while (true) {
        std::cout << "Checking for line..." << std::endl;
        if (Vision vision; vision.detectedLine()) {
            std::cout << "Line detected! Starting main loop..." << std::endl;
            runLoop();
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
}