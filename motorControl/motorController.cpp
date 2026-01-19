#include "../settings.hpp"
#include "motorController.hpp"
#include "discordMessaging/discordMessaging.hpp"
#include <algorithm>

void MotorController::drive(const Speed speed) {
    const int leftSpeed = std::clamp(static_cast<int>(speed.left), SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);
    const int rightSpeed = std::clamp(static_cast<int>(speed.right), SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DISCORD) {
        DiscordMessenger::sendMessageToDiscord(
            "Driving - Left Speed: " + std::to_string(leftSpeed) +
            ", Right Speed: " + std::to_string(rightSpeed),
            MessageType::DEBUG
        );
    }
}
