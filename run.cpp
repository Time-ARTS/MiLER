#include "vision/vision.hpp"
#include "loop.hpp"
#include <thread>
#include <chrono>

int main() {
    Vision vision;
    vision.mainQuadrant.setROI(0, 0, 640, 480); // Example ROI, adjust as needed
    
    while (true) {
        if (vision.detectedLine()) {
            runLoop();
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
}