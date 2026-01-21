#include "vision.hpp"
#include "../settings.hpp"
#include "quadrantHandler.hpp"

#include <memory>
#include <opencv2/opencv.hpp>

#include "discordMessaging/discordMessaging.hpp"

Vision::Vision(const std::shared_ptr<ImageProcessor>& imageProcessor) : imageProcessor_(imageProcessor) {
    if constexpr (!SETTINGS::CONTROL::MODULE::ENABLE_VISION) return;
    cap_.open(SETTINGS::VISION::CAMERA_INDEX, SETTINGS::VISION::CAMERA_SOURCE);

    if (!cap_.isOpened()) {
        throw std::runtime_error("Could not open camera.");
    }
}

cv::Mat Vision::getCurrentFrame() {
    if (!cap_.isOpened())
        throw std::runtime_error("Camera not opened.");

    if (!cap_.read(currentFrame_)) {
        throw std::runtime_error("Could not read frame from camera.");
    }

    return currentFrame_;
}

bool Vision::detectedLine() {
    lineDetected_ = false;
    if constexpr (!SETTINGS::CONTROL::MODULE::ENABLE_VISION) return false;
    if (const cv::Mat frame = getCurrentFrame(); frame.empty()) return false;

    cv::Mat gray, edges;
    cv::cvtColor(currentFrame_, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(SETTINGS::VISION::GAUSSIAN_BLUR_KERNEL, SETTINGS::VISION::GAUSSIAN_BLUR_KERNEL), SETTINGS::VISION::GAUSSIAN_SIGMA);
    cv::Canny(gray, edges, SETTINGS::LINE_THRESHOLD::CANNY_LOW, SETTINGS::LINE_THRESHOLD::CANNY_HIGH);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, SETTINGS::VISION::HOUGH_RHO, SETTINGS::VISION::HOUGH_THETA, SETTINGS::VISION::HOUGH_THRESHOLD, SETTINGS::VISION::HOUGH_MIN_LINE, SETTINGS::VISION::HOUGH_MAX_GAP);

    if (!lines.empty())
    {
        const auto points = this->processFrame();
        lineDetected_ = !points.empty() ? true : false;
    }
    DiscordMessenger::sendMessageToDiscord(lineDetected_ ? "Line detected." : "Line not detected, please try again.",
                                              lineDetected_ ? MessageType::SUCCESS : MessageType::WARNING);
    return lineDetected_;
}

std::vector<Vector2> Vision::processFrame() {
    const cv::Mat frame = getCurrentFrame();
    return imageProcessor_->processQuadrants(frame);
}