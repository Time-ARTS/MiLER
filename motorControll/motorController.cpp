#include <sstream>
#include <string>
#include <algorithm>
#include <gpiod.h>
#include <unistd.h>

#include "motorController.hpp"
#include "../errorMessaging/error_msg.cpp"

void MotorController::drive(Speed speed) {
    int leftSpeed = std::clamp(static_cast<int>(speed.left), SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);
    int rightSpeed = std::clamp(static_cast<int>(speed.right), SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);

    //activeLEDLeft.adaptiveSet(leftSpeed);
    //activeLEDRight.adaptiveSet(rightSpeed);

    sendMessageToDiscord("Driving - Left Speed: " + std::to_string(leftSpeed) + ", Right Speed: " + std::to_string(rightSpeed));

    /*
    // Build a compact protocol message, terminated with newline
    std::ostringstream msg;
    msg << "L:" << leftSpeed << ";R:" << rightSpeed << "\n";
    std::string out = msg.str();

    // Send over serial
    write(serialFd, out.c_str(), out.size());
    */
}
