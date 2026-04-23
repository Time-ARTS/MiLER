#include "PID.hpp"

#include <algorithm>

#include "config.hpp"

double PID::update(const double error, const double dt)
{
    const auto& config_ = ConfigService::get();

    integral_ += error * dt;
    integral_ = std::clamp(integral_,-config_.pid.int_limit, config_.pid.int_limit);

    const double derivative = (error - prevError_) / dt;
    prevError_ = error;

    return config_.pid.kd * error +
           config_.pid.ki * integral_ +
           config_.pid.kp * derivative;
}
