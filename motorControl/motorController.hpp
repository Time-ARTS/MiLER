#ifndef MOTORCONTROLLER_HPP
#define MOTORCONTROLLER_HPP

#pragma once

#include "../constructs.hpp"

class MotorController
{
public:
    static bool selfTest();
    static void turnOffMotors();
    void drive(const double& left, const double& right);

    void backtrack(std::chrono::milliseconds duration);

private:
    static void pwmDrive(double left, double right);

    std::vector<MotorCommandEntry> commandHistory_;
    std::string motorCommandLog_;
    int steeringCount_ = 0;
};


#endif // MOTORCONTROLLER_HPP
