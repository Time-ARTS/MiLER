#include "quadrantHandler.hpp"
#include <opencv2/opencv.hpp>

using namespace SETTINGS;

Vector2 Quadrant::processSelf(cv::Mat frame) const{
    if (roi.empty()) return Vector2(-1, -1);

    cv::Mat quadrant = frame(roi);

    cv::Mat gray, edges;
    cv::cvtColor(quadrant, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(VISION::GAUSSIAN_BLUR_KERNEL, VISION::GAUSSIAN_BLUR_KERNEL), VISION::GAUSSIAN_SIGMA);
    cv::Canny(gray, edges, LINE_THRESHOLD::CANNY_LOW, LINE_THRESHOLD::CANNY_HIGH);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, VISION::HOUGH_RHO, VISION::HOUGH_THETA, VISION::HOUGH_THRESHOLD, VISION::HOUGH_MIN_LINE, VISION::HOUGH_MAX_GAP);

    // ** Split to several line midpoint later? **
    if (!lines.empty()) {
        float avgX = 0, avgY = 0;
        for (auto& l : lines) {
            avgX += (l[0] + l[2]) / 2.0f;
            avgY += (l[1] + l[3]) / 2.0f;
        }
        avgX /= lines.size();
        avgY /= lines.size();
        return Vector2(avgX, avgY + QUADRANT::OFFSET_Y);
    }

    return Vector2(-1, -1);
}

void Camera::quadrantize(const cv::Mat& frame) {
    quadrants_.clear();

    float totalHeight = static_cast<float>(frame.rows);
    float availableHeight = totalHeight - (QuadrantSettings::SPACING_Y * (QUADRANT::NUM - 1));
    float quadrantHeight = availableHeight / QUADRANT::NUM;

    for (int i = 0; i < QUADRANT::NUM; ++i) {
        int startY = static_cast<int>(Settings::QUADRANT::OFFSET_Y + i * (quadrantHeight + QuadrantSettings::SPACING_Y));
        int height = static_cast<int>(std::max(0.0f, std::min(quadrantHeight, totalHeight - startY)));

        Quadrant q;
        q.setROI(0, startY, frame.cols, height);
        quadrants_.push_back(q);
    }
}


std::vector<Vector2> Camera::processQuadrants(const cv::Mat& frame) {
    lineMidpoints_.clear();

    for (auto& q : quadrants_) {
        Vector2 midpoint = q.processSelf(frame);
        lineMidpoints_.push_back(midpoint);
    }

    double timestamp = std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    fullHistory_.emplace_back(timestamp, lineMidpoints_);
    tempHistory_.emplace_back(timestamp, lineMidpoints_);

    while (!tempHistory_.empty() && (timestamp - tempHistory_.front().first) > tempHistoryDuration_) {
        tempHistory_.pop_front();
    }

    return lineMidpoints_;
}