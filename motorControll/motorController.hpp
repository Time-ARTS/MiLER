#ifndef MOTORCONTROLLER_HPP
#define MOTORCONTROLLER_HPP

#pragma once

#include "../settings.hpp"

class MotorController {
    GPIO activeLEDRight;
    GPIO activeLEDLeft;

public:
    void drive(Speed speed);
    void activateMotors();
};

#endif // MOTORCONTROLLER_HPP