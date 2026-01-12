#ifndef MOTORCONTROLLER_HPP
#define MOTORCONTROLLER_HPP

#pragma once

#include "../settings.hpp"
#include "../constructs.hpp"

GPIO activeLEDRight(SETTINGS::LEDPINS::ACTIVE_LED_RIGHT);
GPIO activeLEDLeft(SETTINGS::LEDPINS::ACTIVE_LED_LEFT);

class MotorController {
public:
    void drive(Speed speed);
};

#endif // MOTORCONTROLLER_HPP