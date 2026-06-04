#include "quadrantHandler.hpp"

#include <chrono>

#include "vision.hpp"
#include <opencv2/opencv.hpp>

#include "config.hpp"

Vector2 Quadrant::self(const cv::Mat& frame) const
{
    if (roi.empty()) return Vector2(-1, -1);

    const cv::Mat qFrame = frame(roi);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(qFrame, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) return Vector2(-1, -1);

    const auto largest = std::ranges::max_element(
        contours, [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b)
        {
            return cv::contourArea(a) < cv::contourArea(b);
        }
    );

    const cv::Moments m = cv::moments(*largest);
    if (m.m00 == 0) return Vector2(-1, -1);

    const auto cx = static_cast<float>(m.m10 / m.m00);
    const auto cy = static_cast<float>(m.m01 / m.m00);

    // Offset by ROI to get full-frame coordinates
    return Vector2(roi.x + cx, roi.y + cy);
}


void ImageProcessor::quadrantize(const std::shared_ptr<Vision>& v)
{
    quadrants_.clear();

    const cv::Mat frame = v->getCurrentFrame();

    const auto& config = ConfigService::get();

    const auto totalHeight = static_cast<float>(frame.rows);
    const float availableHeight = totalHeight - config.quadrant.spacing_y * (config.quadrant.num - 1);
    const float quadrantHeight = availableHeight / config.quadrant.num;

    for (int i = 0; i < config.quadrant.num; ++i)
    {
        const int startY = static_cast<int>(config.quadrant.offset_y + static_cast<float>(i) * (quadrantHeight +
            config.quadrant.spacing_y));
        const int height = static_cast<int>(std::max(
            0.0f, std::min(quadrantHeight, totalHeight - static_cast<float>(startY))));

        Quadrant q;
        q.setROI(0, startY, frame.cols, height);
        quadrants_.push_back(q);
    }
}

std::vector<Vector2> ImageProcessor::processQuadrants(const cv::Mat& frame)
{
    lineMidpoints_.clear();

    for (auto& q : quadrants_)
    {
        if (Vector2 midpoint = q.self(frame); midpoint != falsePoint_)
            lineMidpoints_.push_back(midpoint);
    }

    double timestamp = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();

    fullHistory_.emplace_back(timestamp, lineMidpoints_);
    tempHistory_.emplace_back(timestamp, lineMidpoints_);

    const auto& config = ConfigService::get();

    while (!tempHistory_.empty() && timestamp - tempHistory_.front().first > config.history.temp_duration)
    {
        tempHistory_.pop_front();
    }

    return lineMidpoints_;
}
