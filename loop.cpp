#include "loop.hpp"
#include "settings.hpp"
#include "shortKeyCommands/shortKeyCommands.hpp"
#include "discordMessaging/discordMessaging.hpp"

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
    const auto& config = ConfigService::get();

    DiscordMessenger::sendMessageToDiscord("Loop started.", MessageType::INFO);

    while (true)
    {
        if (config.modules.debug)
            DiscordMessenger::sendMessageToDiscord("LineTest activated", MessageType::INFO);

        lineTest();

        if (config.modules.debug)
        {
            DiscordMessenger::sendMessageToDiscord(
                lineDetected_
                    ? "LineTest computed successfully, line detected. **CONTINUING**"
                    : "LineTest computed successfully, line not found. **EXITING**"
                , lineDetected_ ? MessageType::SUCCESS : MessageType::WARNING);
        }

        if (!lineDetected_ || !running)
        {
            DiscordMessenger::sendMessageToDiscord("Loop exiting to main control.", MessageType::WARNING);
            MotorController::turnOffMotors();
            break;
        }

        if (config.modules.debug)
            DiscordMessenger::sendMessageToDiscord("Checking for activated keys", MessageType::INFO);

        shortKeyCommands_->poll();

        loop();
    }
}

void Loop::loop() const
{
    const auto& config = ConfigService::get();

    if (config.modules.debug)
        DiscordMessenger::sendMessageToDiscord("Loop itteration started.", MessageType::INFO);

    const auto points = vision_->processFrame();

    if (config.modules.debug)
        DiscordMessenger::sendMessageToDiscord("Points processed, continuing...", MessageType::SUCCESS);

    const auto line = controlExtractor_->createLine(points);
    auto [leftMotorSpeed, rightMotorSpeed] = controlExtractor_->createMotorVector(line);

    if (config.modules.debug)
        DiscordMessenger::sendMessageToDiscord("Motor-controlls accuired, continuing...", MessageType::SUCCESS);

    if (config.modules.mirroring)
    {
        const cv::Mat frame = vision_->getCurrentFrame();
        const auto quadrants = imageProcessor_->getQuadrants();
        DebugRenderer::render(frame, quadrants, points, line, leftMotorSpeed, rightMotorSpeed);
    }

    motorController_->drive(leftMotorSpeed, rightMotorSpeed);

    DiscordMessenger::sendMessageToDiscord(
        "Speed -> Left: " + std::to_string(leftMotorSpeed) + "   - Right: " + std::to_string(rightMotorSpeed),
        MessageType::DEBUG);

    std::this_thread::sleep_for(std::chrono::milliseconds(config.timing.robot_loop_interval_ms));
}

void Loop::lineTest()
{
    try
    {
        auto points = vision_->processFrame();
        lineDetected_ = std::ranges::any_of(points, [](const Vector2& p) { return p.x >= 0 && p.y >= 0; });
        const auto now = std::chrono::steady_clock::now();

        if (lineDetected_)
        {
            lastLineTime_ = now;
            return;
        }

        DiscordMessenger::sendMessageToDiscord(
            "Line lost. Attempting incremental backtrack recovery...",
            MessageType::WARNING
        );

        const auto& config = ConfigService::get();

        const auto maxRecoveryTime = std::chrono::seconds(config.timing.max_wait_time_s);
        const auto step = std::chrono::milliseconds(config.timing.backtrack_duration_ms);

        const auto recoveryStart = std::chrono::steady_clock::now();

        while (std::chrono::steady_clock::now() - recoveryStart < maxRecoveryTime)
        {
            motorController_->backtrack(step);

            points = vision_->processFrame();
            lineDetected_ = !points.empty();

            if (lineDetected_)
            {
                DiscordMessenger::sendMessageToDiscord("Line re-acquired during backtrack recovery.",
                                                       MessageType::SUCCESS);
                lastLineTime_ = std::chrono::steady_clock::now();
                return;
            }
        }

        DiscordMessenger::sendMessageToDiscord("Line recovery failed. Soft stopping robot.", MessageType::ERROR);

        lineDetected_ = false;
    }
    catch (const std::exception& e)
    {
        DiscordMessenger::sendMessageToDiscord(std::string("Fatal error in loop: ") + e.what(), MessageType::ERROR);
        std::exit(EXIT_FAILURE);
    }
}
