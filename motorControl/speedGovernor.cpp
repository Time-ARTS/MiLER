#include "speedGovernor.hpp"
#include "settings.hpp"

double SpeedGovernor::compute(const double curvature)
{
    return SETTINGS::MOTOR::DEFAULT_SPEED / (1.0f + std::abs(curvature) * SETTINGS::SPEED_GOVERNOR::CURVATURE_GAIN);
}