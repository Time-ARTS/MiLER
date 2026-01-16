#ifndef MOTORCONTROLLER_HPP
#define MOTORCONTROLLER_HPP

#pragma once

#include "../settings.hpp"
#include "../constructs.hpp"

extern GPIO activeLEDRight;
extern GPIO activeLEDLeft;

class MotorController {
public:
    void drive(Speed speed);
};

#endif // MOTORCONTROLLER_HPP