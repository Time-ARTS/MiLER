#include "PID.hpp"
#include "settings.hpp"

double PID::update(const double error, const double dt)
{
    integral_ += error * dt;
    integral_ = std::clamp(integral_,-SETTINGS::PID::INT_LIMIT, SETTINGS::PID::INT_LIMIT);

    const double derivative = (error - prevError_) / dt;
    prevError_ = error;

    return SETTINGS::PID::KP * error +
           SETTINGS::PID::KI * integral_ +
           SETTINGS::PID::KD * derivative;
}