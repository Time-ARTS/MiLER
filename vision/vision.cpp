#include "vision.hpp"
#include "quadrantHandler.hpp"

#include <memory>
#include <opencv2/opencv.hpp>

#include "config.hpp"
#include "discordMessaging/discordMessaging.hpp"

Vision::Vision(const std::shared_ptr<ImageProcessor>& imageProcessor)
    : imageProcessor_(imageProcessor)
{
    const auto& config = ConfigService::get();

    if (!config.modules.vision) return;

    cap_.open(CAMERA::CAMERA_INDEX, CAMERA::CAMERA_SOURCE);

    camTest();
    frameTest();

    getRawFrame();
    if (config.perspective.enabled)
    {
        initPerspective(currentFrame_.cols, currentFrame_.rows);
    }

    if (perspectiveMatrix_.empty())
        throw std::runtime_error("Perspective matrix is empty.");

    if (config.modules.debug)
    {
        std::string cameraInfo = "Camera opened successfully: index " + CAMERA::CAMERA_INFO;

        const int backend = cap_.get(cv::CAP_PROP_BACKEND);
        cameraInfo += " | backend: " + std::to_string(backend);

        DiscordMessenger::sendMessageToDiscord(cameraInfo, MessageType::INFO);
    }
}

cv::Mat Vision::getRawFrame()
{
    if (!cap_.isOpened())
        throw std::runtime_error("Camera not opened.");

    if (!cap_.read(currentFrame_))
        throw std::runtime_error("Could not read frame from camera.");

    cv::cvtColor(currentFrame_, currentFrame_, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(currentFrame_, currentFrame_, cv::Size(5, 5), 0);

    return currentFrame_;
}

cv::Mat Vision::getCurrentFrame()
{
    getRawFrame();

    const auto& config_ = ConfigService::get();

    const int cutoff = static_cast<int>((1.0f - config_.vision.color_threshold) * 255.0f);

    cv::threshold(currentFrame_, currentFrame_, cutoff, 255, cv::THRESH_BINARY_INV);

    cv::morphologyEx(currentFrame_, currentFrame_, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), 1);
    cv::morphologyEx(currentFrame_, currentFrame_, cv::MORPH_CLOSE, cv::Mat(), cv::Point(-1, -1), 2);

    if (ConfigService::get().perspective.enabled && !perspectiveMatrix_.empty())
        cv::warpPerspective(currentFrame_, currentFrame_, perspectiveMatrix_, warpSize_);

    return currentFrame_;
}

void Vision::initPerspective(const int frameWidth, const int frameHeight)
{
    const auto& cfg = ConfigService::get().perspective;

    const float W = static_cast<float>(frameWidth);
    const float H = static_cast<float>(frameHeight);

    const float vy = cfg.vanishing_y * H;
    const float topHalf = cfg.trapezoid_top_width * W * 0.5f;
    const float botHalf = cfg.trapezoid_bottom_width * W * 0.5f;
    const float midX = W * 0.5f;

    const std::vector<cv::Point2f> src = {
        {midX - topHalf, vy}, // top-left
        {midX + topHalf, vy}, // top-right
        {midX + botHalf, H}, // bottom-right
        {midX - botHalf, H}, // bottom-left
    };

    const std::vector<cv::Point2f> dst = {
        {0.f, 0.f},
        {W, 0.f},
        {W, H},
        {0.f, H},
    };

    warpSize_ = cv::Size(frameWidth, frameHeight);
    perspectiveMatrix_ = cv::getPerspectiveTransform(src, dst);
}

bool Vision::detectedLine()
{
    lineDetected_ = false;

    const auto& config_ = ConfigService::get();

    if (!config_.modules.vision) return false;

    const cv::Mat frame = getCurrentFrame();
    if (frame.empty()) return false;

    cv::Mat mask = frame.clone();

    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, {5, 5}));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, {9, 9}));

    const cv::Moments m = cv::moments(mask, true);
    return m.m00 > config_.vision.min_line_area;
}

void Vision::camTest() const
{
    if (!cap_.isOpened())
    {
        throw std::runtime_error("Could not open camera.");
    }
    if (const auto& config_ = ConfigService::get(); config_.modules.debug)
    {
        std::cout << cv::getBuildInformation() << std::endl;
    }
}

void Vision::frameTest()
{
    cv::Mat testFrame;

    if (!cap_.read(testFrame))
    {
        throw std::runtime_error("Failed to grab a frame from the camera.");
    }

    if (testFrame.empty())
    {
        throw std::runtime_error("Frame grabbed from camera is empty.");
    }
}
