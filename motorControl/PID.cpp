#include "PID.hpp"
#include "settings.hpp"

#include <vector>

float PID::compute(const float error)
{
    constexpr auto dt = SETTINGS::CONTROL::THREAD::ROBOT_LOOP_INTERVAL_MS / 1000.0f;
    integral_ += error * dt;
    const float derivative = (error - prevError_) / dt;
    prevError_ = error;

    return kp_ * error + ki_ * integral_ + kd_ * derivative;
}

double PID::compute(const std::vector<double>& error)
{
    const int lookaheadY = std::min<int>(error.size()-1, 20);
    const float errorY= error[lookaheadY] - SETTINGS::SCREEN::CENTER_X;
    return compute(errorY);
}