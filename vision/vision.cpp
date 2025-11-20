#include "vision.hpp"
#include "../settings.hpp"
#include "quadrantHandler.hpp"
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

Vision::Vision() {
    cap_.open(0, cv::CAP_V4L2);  

    if (!cap_.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
    }
}

cv::Mat Vision::getCurrentFrame() {
    if (!cap_.isOpened()) return cv::Mat();

    cap_ >> currentFrame_;
    return currentFrame_;
}

bool Vision::detectedLine() {
    cv::Mat frame = getCurrentFrame();
    if (frame.empty()) return false;

    cv::Mat gray, edges;
    cv::cvtColor(quadrant, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(VISION::GAUSSIAN_BLUR_KERNEL, VISION::GAUSSIAN_BLUR_KERNEL), VISION::GAUSSIAN_SIGMA);
    cv::Canny(gray, edges, LINE_THRESHOLD::CANNY_LOW, LINE_THRESHOLD::CANNY_HIGH);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, VISION::HOUGH_RHO, VISION::HOUGH_THETA, VISION::HOUGH_THRESHOLD, VISION::HOUGH_MIN_LINE, VISION::HOUGH_MAX_GAP);

    return !lines.empty();
}

std::vector<Vector2> Vision::processFrame(const std::shared_ptr<ImageProcessor>& ip) { 
    cv::Mat frame = getCurrentFrame();
    return ip->processQuadrants(frame);
}