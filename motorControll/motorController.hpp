#ifndef MOTORCONTROLLER_HPP
#define MOTORCONTROLLER_HPP

#pragma once

#include "../settings.hpp"

class MotorController {
    GPIO activeLEDRight(16);
    GPIO activeLEDLeft(26);

public:
    void drive(Speed speed);
};

#endif // MOTORCONTROLLER_HPP