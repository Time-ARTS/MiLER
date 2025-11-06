#include "loop.hpp"
#include "settings.hpp"
#include "vision.hpp"
#include "quadrantHandler.hpp"
#include "vectorTranslator.hpp"
#include "pid.hpp"
#include "motorController.hpp"

void runLoop() {
    startupp();
    loop();
}

void startupp() {
    ImageProcessor::quadrantize();
}

void loop() {
    auto points = Vision::processFrame();
    auto speed = VectorTranslator::translate(points);
    auto correction = PID::computeCorrection(speed.deviation);
    MotorController::drive(speed.left + correction, speed.right - correction);
}