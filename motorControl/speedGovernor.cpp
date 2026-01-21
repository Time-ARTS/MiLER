#include "speedGovernor.hpp"

float SpeedGovernor::compute(const float curvature, const float baseSpeed) {
    return baseSpeed / (1.0f + curvature * curvatureGain);
}
