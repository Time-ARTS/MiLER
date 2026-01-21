#include "PID.hpp"

float PID::compute(const float error, const float dt) {
    integral_ += error * dt;
    const float derivative = (error - prevError_) / dt;
    prevError_ = error;

    return kp_*error + ki_*integral_ + kd_*derivative;
}
