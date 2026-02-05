#include "constructs.hpp"
#include "settings.hpp"

std::pair<double, double> DriveCommand::motorCommand() const
{
    constexpr auto min = SETTINGS::MOTOR::MIN_SPEED;
    constexpr auto max = SETTINGS::MOTOR::MAX_SPEED;

    const auto left = std::clamp(speed + correction, min, max);
    const auto right = std::clamp(speed - correction, min, max);
    return {left, right};
}