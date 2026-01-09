#include <sstream>
#include <string>
#include <gpiod.h>
#include <unistd.h> // write()

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

void activateMotors() {
    activeLEDLeft = GPIO(SETTINGS::LEDPINS::ACTIVE_LED_LEFT);
    activeLEDRight = GPIO(SETTINGS::LEDPINS::ACTIVE_LED_RIGHT);
}
