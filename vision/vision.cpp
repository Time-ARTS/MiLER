#include "vision.hpp"
#include "../settings.hpp"
#include "quadrantHandler.hpp"
#include "discordMessaging/discordMessaging.hpp"

#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

Vision::Vision() {
    cap_.open(0, cv::CAP_V4L2);  

    if (!cap_.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        DiscordMessenger::sendMessageToDiscord("Could not open camera.", MessageType::ERROR);
    }
}

cv::Mat Vision::getCurrentFrame() {
    if (!cap_.isOpened()) return cv::Mat();

    if (!cap_.read(currentFrame_)) {
        std::cerr << "Error: Could not read frame from camera." << std::endl;
        DiscordMessenger::sendMessageToDiscord("Could not read frame from camera.", MessageType::ERROR);
        return cv::Mat();
    }
    return currentFrame_;
}

bool Vision::detectedLine() {
    cv::Mat frame = getCurrentFrame();
    if (frame.empty()) return false;

    cv::Mat gray, edges;
    cv::cvtColor(currentFrame_, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(SETTINGS::VISION::GAUSSIAN_BLUR_KERNEL, SETTINGS::VISION::GAUSSIAN_BLUR_KERNEL), SETTINGS::VISION::GAUSSIAN_SIGMA);
    cv::Canny(gray, edges, SETTINGS::LINE_THRESHOLD::CANNY_LOW, SETTINGS::LINE_THRESHOLD::CANNY_HIGH);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, SETTINGS::VISION::HOUGH_RHO, SETTINGS::VISION::HOUGH_THETA, SETTINGS::VISION::HOUGH_THRESHOLD, SETTINGS::VISION::HOUGH_MIN_LINE, SETTINGS::VISION::HOUGH_MAX_GAP);

    return !lines.empty();
}

std::vector<Vector2> Vision::processFrame(const std::shared_ptr<ImageProcessor>& ip) { 
    cv::Mat frame = getCurrentFrame();
    return ip->processQuadrants(frame);
}