#include "loop.hpp"


void runLoop() {
    startupp();
    loop();
}

void startupp() {
    camera::quadrantize();
}

void loop() {
    vision::processFrame();
    auto speed = vectorTranslator::translate(vision::getLinePoints());
    auto correction = PID::computeCorrection(speed.deviation);
    MotorController::drive(speed.left + correction, speed.right - correction);
}