#include "motorController.hpp"

#include <algorithm>

#include <thread>
#include <chrono>

#include "config.hpp"

#ifdef __linux__
#include <wiringPi.h>
#include <softPwm.h>
#endif

bool MotorController::selfTest()
{
    try
    {
        std::cout << "Testing motors:" << std::endl;
        turnOnMotors();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::cout << "Motor Left driving forward" << std::endl;
        pwmDrive(10, 0); // Left forward
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::cout << "Motor Right driving forward" << std::endl;
        pwmDrive(0, 10); // Right forward
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::cout << "Both driving forward" << std::endl;
        pwmDrive(10, 10); // Both forward
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::cout << "Both driving backward" << std::endl;
        pwmDrive(-10, -10); // Both reverse
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        pwmDrive(0, 0); // Stop

        std::cout << "Truning motors off" << std::endl;
        turnOffMotors();

        std::cout << "Motor self-test passed" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

void MotorController::turnOffMotors()
{
    pwmDrive(0, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

#ifdef __linux__
    digitalWrite(ConfigService::get().motor_pins.stby, LOW);
#endif

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void MotorController::turnOnMotors()
{
#ifdef __linux__
    digitalWrite(ConfigService::get().motor_pins.stby, HIGH);
#endif

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    pwmDrive(0, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void MotorController::drive(const double& left, const double& right)
{
    commandHistory_.push_back({{left, right}, std::chrono::steady_clock::now()});

    pwmDrive(left, right);

    if (const auto& config_ = ConfigService::get(); config_.modules.logging)
    {
        motorCommandLog_ += "Steering: LEFT - " + std::to_string(left) + " |.| RIGHT - " + std::to_string(right) +
            "\\n";

        steeringCount_ = std::clamp(steeringCount_ + 1, 0, config_.motor.steering_count);

        if (steeringCount_ == config_.motor.steering_count)
        {
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

        std::this_thread::sleep_for(std::chrono::milliseconds(ConfigService::get().timing.robot_loop_interval_ms));
    }
}

void MotorController::pwmDrive(const double left, const double right)
{
#ifdef __linux__
    const int pwmLeft = static_cast<int>(std::abs(left));
    const int pwmRight = static_cast<int>(std::abs(right));

    const auto& config_= ConfigService::get().motor_pins;

    // LEFT motor direction
    digitalWrite(config_.ai1, left >= 0 ? LOW : HIGH);
    digitalWrite(config_.ai2, left >= 0 ? HIGH : LOW);

    // RIGHT motor direction
    digitalWrite(config_.bi1, right >= 0 ? LOW : HIGH);
    digitalWrite(config_.bi2, right >= 0 ? HIGH : LOW);

    // Speed
    softPwmWrite(config_.pwma_l, pwmLeft);
    softPwmWrite(config_.pwmb_r, pwmRight);
#endif
}
