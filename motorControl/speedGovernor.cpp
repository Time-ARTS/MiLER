#include "speedGovernor.hpp"
#include "settings.hpp"

double SpeedGovernor::compute(const double curvature)
{
    return SETTINGS::MOTOR::DEFAULT_SPEED / (1.0f + std::abs(curvature) * SETTINGS::SPEED_GOVERNOR::CURVATURE_GAIN);
}

double SpeedGovernor::compute(const std::vector<double>& line)
{
    const int y = line.size() - 1;
    const double dx1 = line[y] - line[y-1];
    const double dx2 = line[y-1] - line[y-2];
    const double curvature = std::abs(dx1 - dx2);
    const double speedFactor = std::max(0.1, 1.0 - curvature/50.0);
    return compute(speedFactor);
}