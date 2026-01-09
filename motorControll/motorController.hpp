#ifndef MOTORCONTROLLER_HPP
#define MOTORCONTROLLER_HPP

#pragma once

#include "../settings.hpp"
#include "../constructs.hpp"

class MotorController {
    GPIO activeLEDRight(SETTINGS::LEDPINS::ACTIVE_LED_RIGHT);
    GPIO activeLEDLeft(SETTINGS::LEDPINS::ACTIVE_LED_LEFT);

public:
    void drive(Speed speed);
};

#endif // MOTORCONTROLLER_HPP