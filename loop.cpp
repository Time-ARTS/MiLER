#include "loop.hpp"
#include "settings.hpp"
#include "shortKeyCommands/shortKeyCommands.hpp"
#include "discordMessaging/discordMessaging.hpp"

#include "vision/vision.hpp"
#include "motorControl/controlExtractor.hpp"
#include "motorControl/motorController.hpp"
#include "motorControl/PID.hpp"
#include "motorControl/speedGovernor.hpp"

#include <chrono>
#include <thread>
#include <memory>

#include "vision/debugRenderer.hpp"

Loop::Loop()
    : controlExtractor_(std::make_shared<ControlExtractor>())
      , imageProcessor_(std::make_shared<ImageProcessor>())
      , motorController_(std::make_shared<MotorController>())
      , pid_(std::make_shared<PID>(SETTINGS::PID::KD, SETTINGS::PID::KI, SETTINGS::PID::KP))
      , speedGovernor_(std::make_shared<SpeedGovernor>())
      , vision_(std::make_shared<Vision>(imageProcessor_))
      , shortKeyCommands_(std::make_shared<shortKeyCommands>())
      , lineDetected_(true)
{
    std::vector<std::string> failed;

    if (!controlExtractor_) failed.emplace_back("ControlExtractor");
    if (!imageProcessor_) failed.emplace_back("ImageProcessor");
    if (!motorController_) failed.emplace_back("MotorController");
    if (!pid_) failed.emplace_back("PID");
    if (!speedGovernor_) failed.emplace_back("SpeedGovernor");
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
    DiscordMessenger::sendMessageToDiscord("Loop started.", MessageType::INFO);

    while (true)
    {
        if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
            DiscordMessenger::sendMessageToDiscord("LineTest activated", MessageType::INFO);

        lineTest();

        if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
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
            motorController_->turnOffMotors();
            break;
        }

        if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
            DiscordMessenger::sendMessageToDiscord("Checking for activated keys", MessageType::INFO);

        shortKeyCommands_->poll();

        loop();
    }
}

void Loop::loop() const
{
    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord("Loop itteration started.", MessageType::INFO);

    const auto points = vision_->processFrame();

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord("Points processed successfully, continuing to line estimation",
                                               MessageType::SUCCESS);

    const auto line = controlExtractor_->createLine(points);
    auto [leftMotorSpeed, rightMotorSpeed] = controlExtractor_->createMotorVector(line);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord(
            "Line estimated successfully, continuing to correction and speed estimation", MessageType::SUCCESS);

    leftMotorSpeed = pid_->compute(leftMotorSpeed);
    rightMotorSpeed = pid_->compute(rightMotorSpeed);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord("Correction completed successfully, waiting for speed estimation",
                                               MessageType::SUCCESS);

    leftMotorSpeed = speedGovernor_->compute(leftMotorSpeed);
    rightMotorSpeed = speedGovernor_->compute(rightMotorSpeed);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord(
            "Speed estimation completed successfully, sending drive commands to motors", MessageType::SUCCESS);
    DiscordMessenger::sendMessageToDiscord(
        "Speed -> Left: " + std::to_string(leftMotorSpeed) + "   - Right: " + std::to_string(rightMotorSpeed),
        MessageType::DEBUG);

    motorController_->drive(leftMotorSpeed, rightMotorSpeed);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
    {
        const cv::Mat frame = vision_->getCurrentFrame();
        const auto quadrants = imageProcessor_->getQuadrants();
        DebugRenderer::render(frame, quadrants, points, line, leftMotorSpeed, rightMotorSpeed);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(SETTINGS::CONTROL::THREAD::ROBOT_LOOP_INTERVAL_MS));
}

void Loop::lineTest()
{
    try
    {
        auto points = vision_->processFrame();
        lineDetected_ = !points.empty();
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

        constexpr auto maxRecoveryTime = SETTINGS::CONTROL::TIME::MAX_WAIT_TIME;
        constexpr auto step = std::chrono::milliseconds(SETTINGS::CONTROL::TIME::BACKTRACK_DURATION);

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
