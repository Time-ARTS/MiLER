#include "loop.hpp"
#include "shortKeyCommands/shortKeyCommands.hpp"

#include "vision/vision.hpp"
#include "motorControl/controlExtractor.hpp"
#include "motorControl/motorController.hpp"
#include "motorControl/PID.hpp"

#include <chrono>
#include <thread>
#include <memory>

#include "config.hpp"
#include "vision/debugRenderer.hpp"

Loop::Loop()
    : pid_(std::make_shared<PID>())
      , controlExtractor_(std::make_shared<ControlExtractor>(pid_))
      , imageProcessor_(std::make_shared<ImageProcessor>())
      , motorController_(std::make_shared<MotorController>())
      , vision_(std::make_shared<Vision>(imageProcessor_))
      , shortKeyCommands_(std::make_shared<shortKeyCommands>())
      , lineDetected_(true)
{
    std::vector<std::string> failed;

    if (!controlExtractor_) failed.emplace_back("ControlExtractor");
    if (!imageProcessor_) failed.emplace_back("ImageProcessor");
    if (!motorController_) failed.emplace_back("MotorController");
    if (!pid_) failed.emplace_back("PID");
    if (!vision_) failed.emplace_back("Vision");
    if (!shortKeyCommands_) failed.emplace_back("ShortKeyCommands");

    if (!failed.empty())
    {
        std::ostringstream oss;
        oss << "Loop subsystem initialization failed. Systems:";
        for (const auto& name : failed)
            oss << " " << name;

        throw std::runtime_error(oss.str());
    }

    imageProcessor_->quadrantize(vision_);
}

void Loop::run(const std::atomic<bool>& running)
{
    while (true)
    {
        lineTest();

        if (!lineDetected_ || !running)
        {
            MotorController::turnOffMotors();
            break;
        }

        shortKeyCommands_->poll();

        loop();
    }
}

void Loop::loop() const
{
    const auto& config = ConfigService::get();

    const auto points = vision_->processFrame();

    const auto line = controlExtractor_->createLine(points);
    auto [leftMotorSpeed, rightMotorSpeed] = controlExtractor_->createMotorVector(line);

    if (config.modules.mirroring)
    {
        const auto quadrants = imageProcessor_->getQuadrants();
        DebugRenderer::render(currentFrame_, quadrants, points, line, leftMotorSpeed, rightMotorSpeed);
    }

    motorController_->drive(leftMotorSpeed, rightMotorSpeed);

    std::this_thread::sleep_for(std::chrono::milliseconds(config.timing.robot_loop_interval_ms));
}

void Loop::lineTest()
{
    try
    {
        currentFrame_ = vision_->getCurrentFrame();
        auto points = vision_->processFrame();
        lineDetected_ = !points.empty();

        const auto& config = ConfigService::get();
        const auto now = std::chrono::steady_clock::now();

        if (lineDetected_)
        {
            lastLineTime_ = now;
            return;
        }

        if (std::chrono::milliseconds(config.timing.backtrack_wait_time_ms) > now - lastLineTime_)
        {
            lineDetected_ = true;
            return;
        }

        const auto maxRecoveryTime = std::chrono::seconds(config.timing.max_wait_time_s);
        const auto step = std::chrono::milliseconds(config.timing.backtrack_duration_ms);

        const auto recoveryStart = std::chrono::steady_clock::now();

        while (std::chrono::steady_clock::now() - recoveryStart < maxRecoveryTime)
        {
            motorController_->backtrack(step);

            currentFrame_ = vision_->getCurrentFrame();
            points = vision_->processFrame();
            lineDetected_ = !points.empty();

            if (lineDetected_)
            {
                lastLineTime_ = std::chrono::steady_clock::now();
                return;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
