#include "quadrantHandler.hpp"
#include "vision.hpp"
#include "../settings.hpp"
#include <opencv2/opencv.hpp>

using namespace SETTINGS;

Vector2 Quadrant::self(cv::Mat frame) const{
    if (roi.empty()) return Vector2(-1, -1);

    cv::Mat qFrame = frame(roi);
    cv::Mat gray;

    if (qFrame.channels() == 3)
        cv::cvtColor(qFrame, gray, cv::COLOR_BGR2GRAY);
    else
        gray = qFrame;

    cv::Mat binary;
    cv::threshold(gray, binary, LINE_THRESHOLD::MIN, LINE_THRESHOLD::MAX, cv::THRESH_BINARY);

    int width = binary.cols;
    int height = binary.rows;

    float sumX = 0.0f;
    float count = 0.0f;

    for (int y = 0; y < height; ++y) {
        uchar* row = binary.ptr<uchar>(y);
        for (int x = 0; x < width; ++x) {
            if (row[x] > 0) {
                sumX += x;
                count += 1.0f;
            }
        }
    }

    if (count == 0.0f) return Vector2(-1, -1);

    float xMid = sumX / count;
    float yMid = height / 2.0f;

    return Vector2(xMid, yMid + QUADRANT::OFFSET_Y);
}

void ImageProcessor::quadrantize(const std::shared_ptr<Vision>& v) {
    quadrants_.clear();

    cv::Mat frame = v->getCurrentFrame();

    float totalHeight = static_cast<float>(frame.rows);
    float availableHeight = totalHeight - (QUADRANT::SPACING_Y * (QUADRANT::NUM - 1));
    float quadrantHeight = availableHeight / QUADRANT::NUM;

    for (int i = 0; i < QUADRANT::NUM; ++i) {
        int startY = static_cast<int>(QUADRANT::OFFSET_Y + i * (quadrantHeight + QUADRANT::SPACING_Y));
        int height = static_cast<int>(std::max(0.0f, std::min(quadrantHeight, totalHeight - startY)));

        Quadrant q;
        q.setROI(0, startY, frame.cols, height);
        quadrants_.push_back(q);
    }
}

std::vector<Vector2> ImageProcessor::processQuadrants(const cv::Mat& frame) {
    lineMidpoints_.clear();

    for (auto& q : quadrants_) {
        Vector2 midpoint = q.self(frame);
        lineMidpoints_.push_back(midpoint);
    }

    double timestamp = std::chrono::duration<double>( std::chrono::steady_clock::now().time_since_epoch() ).count();

    fullHistory_.emplace_back(timestamp, lineMidpoints_);
    tempHistory_.emplace_back(timestamp, lineMidpoints_);

    while (!tempHistory_.empty() && (timestamp - tempHistory_.front().first) > HISTORY::TEMP_DURATION) {
        tempHistory_.pop_front();
    }

    return lineMidpoints_;
}