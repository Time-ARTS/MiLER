#include "../settings.hpp"
#include "motorController.hpp"
#include "discordMessaging/discordMessaging.hpp"

#include <thread>
#include <chrono>

#ifdef __linux__
#include <wiringPi.h>
#include <softPwm.h>
#endif

bool MotorController::selfTest()
{
    DiscordMessenger::sendMessageToDiscord("Motor self-test started...", MessageType::INFO);

    try
    {
        DiscordMessenger::sendMessageToDiscord("Motor Left driving: 120", MessageType::INFO);
        pwmDrive(120, 0);   // Left forward
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        DiscordMessenger::sendMessageToDiscord("Motor Right driving: 120", MessageType::INFO);
        pwmDrive(0, 120);   // Right forward
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        DiscordMessenger::sendMessageToDiscord("Motor Both driving: 120", MessageType::INFO);
        pwmDrive(120, 120); // Both forward
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        DiscordMessenger::sendMessageToDiscord("Motor Both driving: -120", MessageType::INFO);
        pwmDrive(-120, -120); // Both reverse
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        pwmDrive(0, 0); // Stop

        DiscordMessenger::sendMessageToDiscord("Motor self-test passed", MessageType::SUCCESS);
        return true;
    }
    catch (...)
    {
        DiscordMessenger::sendMessageToDiscord("Motor self-test FAILED", MessageType::ERROR);
        return false;
    }
}

void MotorController::turnOffMotors()
{
    pwmDrive(0, 0);

    #ifdef __linux__
    digitalWrite(SETTINGS::MOTOR::PIN::STANDBY, LOW);
    #endif
}

void MotorController::turnOnMotors()
{
    pwmDrive(0, 0);

#ifdef __linux__
    digitalWrite(SETTINGS::MOTOR::PIN::STANDBY, HIGH);
#endif
}

void MotorController::drive(const double& left, const double& right)
{
    commandHistory_.push_back({{left, right}, std::chrono::steady_clock::now()});

    pwmDrive(left, right);

    if (SETTINGS::CONTROL::MODULE::ENABLE_LOGGING && SETTINGS::CONTROL::MODULE::ENABLE_STEERING_COUNTER)
    {
        motorCommandLog_ += "Steering: LEFT - " + std::to_string(left) + " |.| RIGHT - " + std::to_string(right) + "\\n";

        steeringCount_ = std::clamp(steeringCount_ + 1, 0, SETTINGS::MOTOR::STEERING_COUNT);

        if (steeringCount_ == SETTINGS::MOTOR::STEERING_COUNT)
        {
            DiscordMessenger::sendMessageToDiscord(motorCommandLog_);
            motorCommandLog_.clear();
            steeringCount_ = 0;
        }
    }
}

void MotorController::backtrack(const std::chrono::milliseconds duration)
{
    const auto now = std::chrono::steady_clock::now();

    std::vector<MotorCommandEntry> toBacktrack;

    while (!commandHistory_.empty())
    {
        if (const auto& entry = commandHistory_.back(); now - entry.timestamp <= duration)
        {
            toBacktrack.push_back(entry);
            commandHistory_.pop_back();
        }
        else
        {
            break;
        }
    }

    for (auto it = toBacktrack.begin(); it != toBacktrack.end(); ++it)
    {
        auto [left, right] = it->speed;
        pwmDrive(-left, -right);

        std::this_thread::sleep_for(std::chrono::milliseconds(SETTINGS::CONTROL::THREAD::ROBOT_LOOP_INTERVAL_MS));
    }
}

void MotorController::pwmDrive(const double left, const double right)
{
#ifdef __linux__
    const int pwmLeft = static_cast<int>(std::abs(left));
    const int pwmRight = static_cast<int>(std::abs(right));

    // LEFT motor direction
    digitalWrite(SETTINGS::MOTOR::PIN::IN1_LEFT, left >= 0 ? LOW : HIGH);
    digitalWrite(SETTINGS::MOTOR::PIN::IN2_LEFT, left >= 0 ? HIGH : LOW);

    // RIGHT motor direction
    digitalWrite(SETTINGS::MOTOR::PIN::IN1_RIGHT, right >= 0 ? LOW : HIGH);
    digitalWrite(SETTINGS::MOTOR::PIN::IN2_RIGHT, right >= 0 ? HIGH : LOW);

    // Speed
    softPwmWrite(SETTINGS::MOTOR::PIN::EN_LEFT, pwmLeft);
    softPwmWrite(SETTINGS::MOTOR::PIN::EN_RIGHT, pwmRight);
#endif
}
