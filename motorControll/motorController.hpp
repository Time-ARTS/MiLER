#ifndef MOTORCONTROLLER_HPP
#define MOTORCONTROLLER_HPP

#pragma once

#include "../settings.hpp"
#include "../constructs.hpp"

class MotorController {
public:
    void drive(Speed speed);
};

#endif // MOTORCONTROLLER_HPP