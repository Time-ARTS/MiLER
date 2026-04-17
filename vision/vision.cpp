#include "vision.hpp"
#include "../settings.hpp"
#include "quadrantHandler.hpp"

#include <memory>
#include <opencv2/opencv.hpp>
#include "discordMessaging/discordMessaging.hpp"

Vision::Vision(const std::shared_ptr<ImageProcessor>& imageProcessor)
    : imageProcessor_(imageProcessor)
{
    if (!SETTINGS::CONTROL::MODULE::ENABLE_VISION) return;

    cap_.open(SETTINGS::VISION::CAMERA_INDEX, SETTINGS::VISION::CAMERA_SOURCE);

    camTest();
    frameTest();

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
    {
        std::string cameraInfo = "Camera opened successfully: index "
            + std::to_string(SETTINGS::VISION::CAMERA_INDEX);

        const int backend = cap_.get(cv::CAP_PROP_BACKEND);
        cameraInfo += " | backend: " + std::to_string(backend);

        DiscordMessenger::sendMessageToDiscord(cameraInfo, MessageType::INFO);}
}

void Vision::camTest() const
{
    if (!cap_.isOpened())
    {
        throw std::runtime_error("Could not open camera.");
    }

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
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

cv::Mat Vision::getCurrentFrame()
{
    if (!cap_.isOpened())
        throw std::runtime_error("Camera not opened.");

    if (!cap_.read(currentFrame_))
        throw std::runtime_error("Could not read frame from camera.");

    cv::cvtColor(currentFrame_, currentFrame_, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(currentFrame_, currentFrame_, cv::Size(5,5), 0);

    constexpr int cutoff = static_cast<int>((1.0f - SETTINGS::VISION::COLOR_THRESHOLD) * 255.0f);

    cv::threshold(currentFrame_, currentFrame_, cutoff, 255, cv::THRESH_BINARY_INV);

    cv::morphologyEx(currentFrame_, currentFrame_, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1,-1), 1);
    cv::morphologyEx(currentFrame_, currentFrame_, cv::MORPH_CLOSE, cv::Mat(), cv::Point(-1,-1), 2);

    return currentFrame_;
}

bool Vision::detectedLine()
{
    lineDetected_ = false;

    if (!SETTINGS::CONTROL::MODULE::ENABLE_VISION) return false;

    const cv::Mat frame = getCurrentFrame();
    if (frame.empty()) return false;

    cv::Mat mask = frame.clone();

    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, {5, 5}));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, {9, 9}));

    const cv::Moments m = cv::moments(mask, true);
    return m.m00 > SETTINGS::VISION::MIN_LINE_AREA;
}