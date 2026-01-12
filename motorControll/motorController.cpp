#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <gpiod.h>
#include <unistd.h>

#include "motorController.hpp"

void MotorController::drive(Speed speed) {
    int leftSpeed = std::clamp(speed.left, SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);
    int rightSpeed = std::clamp(speed.right, SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);
    
    activeLEDLeft.adaptiveSet(leftSpeed);
    activeLEDRight.adaptiveSet(rightSpeed);

    /*
    // Build a compact protocol message, terminated with newline
    std::ostringstream msg;
    msg << "L:" << leftSpeed << ";R:" << rightSpeed << "\n";
    std::string out = msg.str();

    // Send over serial
    write(serialFd, out.c_str(), out.size());
    */
}
