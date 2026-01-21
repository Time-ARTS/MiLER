#include "../settings.hpp"
#include "motorController.hpp"
#include "discordMessaging/discordMessaging.hpp"

#include <thread>
#include <chrono>

#ifdef __linux__
#include <wiringPi.h>
#include <softPwm.h>
#endif

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

void MotorController::pwmDrive(const float left, const float right)
{
#ifdef __linux__
    const int pwmLeft  = std::clamp(static_cast<int>(std::abs(left)  * 255),  0, 255);
    const int pwmRight = std::clamp(static_cast<int>(std::abs(right) * 255), 0, 255);

    // LEFT motor direction
    digitalWrite(SETTINGS::MOTOR::PIN::IN1_LEFT, left  >= 0 ? HIGH : LOW);
    digitalWrite(SETTINGS::MOTOR::PIN::IN2_LEFT, left  >= 0 ? LOW  : HIGH);

    // RIGHT motor direction
    digitalWrite(SETTINGS::MOTOR::PIN::IN1_RIGHT, right >= 0 ? HIGH : LOW);
    digitalWrite(SETTINGS::MOTOR::PIN::IN2_RIGHT, right >= 0 ? LOW  : HIGH);

    // Speed
    softPwmWrite(SETTINGS::MOTOR::PIN::EN_LEFT,  pwmLeft);
    softPwmWrite(SETTINGS::MOTOR::PIN::EN_RIGHT, pwmRight);
#endif
}
