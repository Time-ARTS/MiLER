#include "../settings.hpp"
#include "motorController.hpp"
#include "discordMessaging/discordMessaging.hpp"

void MotorController::drive(const DriveCommand& cmd) {
    auto [left, right] = cmd.motorCommand();

    commandHistory_.push_back({cmd, std::chrono::steady_clock::now()});

    this->pwmDrive(left, right);

    if (SETTINGS::CONTROL::MODULE::ENABLE_LOGGING) DiscordMessenger::sendMessageToDiscord("Steering: LEFT - " + std::to_string(left) + " || RIGHT - " + std::to_string(right));
}

void MotorController::backtrack(const std::chrono::milliseconds duration)
{
    const auto now = std::chrono::steady_clock::now();

    std::vector<MotorCommandEntry> toBacktrack;

    while (!commandHistory_.empty()) {
        if (const auto& entry = commandHistory_.back(); now - entry.timestamp <= duration) {
            toBacktrack.push_back(entry);
            commandHistory_.pop_back();
        } else {
            break;
        }
    }

    for (auto it = toBacktrack.begin(); it != toBacktrack.end(); ++it) {
        auto [left, right] = it->cmd.motorCommand();
        pwmDrive(-left, -right);

        std::this_thread::sleep_for(std::chrono::milliseconds(SETTINGS::CONTROL::THREAD::ROBOT_LOOP_INTERVAL_MS));
    }
}

void MotorController::pwmDrive(const int left, const int right)
{
    const int pwmLeft  = static_cast<int>(left * 255);
    const int pwmRight = static_cast<int>(right * 255);

#ifdef __linux__
    // Not necessarily correct
    pwmWrite(LEFT_MOTOR_PIN, pwmLeft);
    pwmWrite(RIGHT_MOTOR_PIN, pwmRight);
#endif
}