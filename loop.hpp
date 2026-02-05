#ifndef MILLER_LOOP_HPP
#define MILLER_LOOP_HPP

#pragma once
#include <chrono>
#include <memory>

#include "motorControl/motorController.hpp"

class ControlExtractor;
class ImageProcessor;
class MotorController;
class PID;
class SpeedGovernor;
class Vision;
class shortKeyCommands;

class Loop
{
public:
    Loop();

    void run(const std::atomic<bool>& running);

    void motorTest() const { motorController_->selfTest(); }

    [[nodiscard]] std::shared_ptr<ControlExtractor> getControlExtractor() const { return controlExtractor_; }
    [[nodiscard]] std::shared_ptr<ImageProcessor> getImageProcessor() const { return imageProcessor_; }
    [[nodiscard]] std::shared_ptr<MotorController> getMotorController() const { return motorController_; }
    [[nodiscard]] std::shared_ptr<PID> getPID() const { return pid_; }
    [[nodiscard]] std::shared_ptr<SpeedGovernor> getSpeedGovernor() const { return speedGovernor_; }
    [[nodiscard]] std::shared_ptr<Vision> getVision() const { return vision_; }
    [[nodiscard]] std::shared_ptr<shortKeyCommands> getShortKeyCommands() const { return shortKeyCommands_; }

private:
    const std::shared_ptr<ControlExtractor> controlExtractor_;
    const std::shared_ptr<ImageProcessor> imageProcessor_;
    const std::shared_ptr<MotorController> motorController_;
    const std::shared_ptr<PID> pid_;
    const std::shared_ptr<SpeedGovernor> speedGovernor_;
    const std::shared_ptr<Vision> vision_;
    const std::shared_ptr<shortKeyCommands> shortKeyCommands_;

    bool lineDetected_;
    std::chrono::time_point<std::chrono::steady_clock> lastLineTime_ = std::chrono::steady_clock::now();

    void loop() const;
    void lineTest();
};

#endif //MILLER_LOOP_HPP
