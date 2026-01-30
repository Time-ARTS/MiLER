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

    if (!cap_.isOpened())
    {
        throw std::runtime_error("Could not open camera.");
    }
}

cv::Mat Vision::getCurrentFrame()
{
    if (!cap_.isOpened())
        throw std::runtime_error("Camera not opened.");

    if (!cap_.read(currentFrame_))
        throw std::runtime_error("Could not read frame from camera.");

    cv::cvtColor(currentFrame_, currentFrame_, cv::COLOR_BGR2GRAY);

    cv::GaussianBlur(
        currentFrame_, currentFrame_,
        cv::Size(SETTINGS::VISION::GAUSSIAN_BLUR_KERNEL, SETTINGS::VISION::GAUSSIAN_BLUR_KERNEL),
        SETTINGS::VISION::GAUSSIAN_SIGMA);

    return currentFrame_;
}

bool Vision::detectedLine(cv::Mat* debugMask)
{
    lineDetected_ = false;

    if (!SETTINGS::CONTROL::MODULE::ENABLE_VISION) return false;

    const cv::Mat frame = getCurrentFrame();
    if (frame.empty()) return false;

    cv::Mat mask;
    cv::threshold(frame, mask, SETTINGS::VISION::THRESHOLD_MIN, SETTINGS::VISION::THRESHOLD_MAX, cv::THRESH_BINARY_INV);

    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, {5,5}));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, {9,9}));

    if (debugMask) *debugMask = mask.clone();

    const cv::Moments m = cv::moments(mask, true);
    return m.m00 > SETTINGS::VISION::MIN_LINE_AREA;
}