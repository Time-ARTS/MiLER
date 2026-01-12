#include <sstream>
#include <string>
#include <algorithm>
#include <gpiod.h>
#include <unistd.h>

#include "motorController.hpp"

void MotorController::drive(Speed speed) {
    const float leftspeed = speed.left;
    const float rightspeed = speed.right;
    float leftSpeed = std::clamp(leftspeed, SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);
    float rightSpeed = std::clamp(rightspeed, SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);

    activeLEDLeft.adaptiveSet(static_cast<int>(leftSpeed));
    activeLEDRight.adaptiveSet(static_cast<int>(rightSpeed));

    /*
    // Build a compact protocol message, terminated with newline
    std::ostringstream msg;
    msg << "L:" << leftSpeed << ";R:" << rightSpeed << "\n";
    std::string out = msg.str();

    // Send over serial
    write(serialFd, out.c_str(), out.size());
    */
}
