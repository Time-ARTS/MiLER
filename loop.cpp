#include "loop.hpp"
#include "settings.hpp"
#include "vision/vision.hpp"
#include "vision/quadrantHandler.hpp"
#include "motorControll/vectorTranslator.hpp"
#include "motorControll/PID.hpp"
#include "motorControll/motorController.hpp"

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