#include "loop.hpp"
#include "settings.hpp"
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
#ifdef _WIN32
      , shortKeyCommands_(std::make_shared<shortKeyCommands>())
#endif
      , lineDetected_(true)
{
    std::vector<std::string> failed;

    if (!controlExtractor_) failed.emplace_back("ControlExtractor");
    if (!imageProcessor_) failed.emplace_back("ImageProcessor");
    if (!motorController_) failed.emplace_back("MotorController");
    if (!pid_) failed.emplace_back("PID");
    if (!speedGovernor_) failed.emplace_back("SpeedGovernor");
    if (!vision_) failed.emplace_back("Vision");

#ifdef _WIN32
    if (!shortKeyCommands_) failed.emplace_back("ShortKeyCommands");
#endif

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

void Loop::run()
{
    DiscordMessenger::sendMessageToDiscord("Loop started.", MessageType::INFO);

    while (true)
    {
        lineTest();

        if (!lineDetected_)
        {
            DiscordMessenger::sendMessageToDiscord("Loop exiting to main control.", MessageType::WARNING);
            break;
        }

#ifdef _WIN32
        shortKeyCommands_->poll();
#endif
        loop();
    }
}

void Loop::loop() const
{
    const auto points = vision_->processFrame();

    const auto [deviation, curvature] = controlExtractor_->extract(points);

    constexpr float dt = SETTINGS::CONTROL::THREAD::ROBOT_LOOP_INTERVAL_MS / 1000.0f;
    const float turnCorrection = pid_->compute(deviation, dt);

    const float speed = speedGovernor_->compute(curvature, SETTINGS::MOTOR::DEFAULT_SPEED);

    const DriveCommand cmd{speed, turnCorrection};
    motorController_->drive(cmd);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
    {
        cv::Mat frame = vision_->getCurrentFrame();
        const auto quadrants = imageProcessor_->getQuadrants();
        DebugRenderer::render(frame, quadrants, points, deviation, curvature, turnCorrection, speed);
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
