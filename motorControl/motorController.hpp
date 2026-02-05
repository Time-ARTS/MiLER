#ifndef MOTORCONTROLLER_HPP
#define MOTORCONTROLLER_HPP

#pragma once

#include "../constructs.hpp"

struct MotorCommandEntry
{
    DriveCommand cmd{};
    std::chrono::steady_clock::time_point timestamp;
};

class MotorController
{
public:
    static bool selfTest();
    static void turnOffMotors();
    void drive(const DriveCommand& cmd);

    void backtrack(std::chrono::milliseconds duration);

private:
    static void pwmDrive(float left, float right);

    std::vector<MotorCommandEntry> commandHistory_;
    std::string motorCommandLog_;
    int steeringCount_ = 0;
};


#endif // MOTORCONTROLLER_HPP
