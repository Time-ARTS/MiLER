#include "quadrantHandler.hpp"

#include <chrono>

#include "vision.hpp"
#include "../settings.hpp"
#include <opencv2/opencv.hpp>

#include "discordMessaging/discordMessaging.hpp"

using namespace SETTINGS;

Vector2 Quadrant::self(const cv::Mat& frame) const
{
    if (roi.empty()) return Vector2(-1, -1);

    const cv::Mat qFrame = frame(roi);

    const int width = qFrame.cols;
    const int height = qFrame.rows;

    double weightedSumX = 0.0;
    double weightedSumY = 0.0;
    double totalWeight = 0.0;

    for (int y = 0; y < height; ++y)
    {
        const uchar* row = qFrame.ptr<uchar>(y);
        for (int x = 0; x < width; ++x)
        {
            const double weight = 255.0 - static_cast<double>(row[x]);
            if (weight <= 0.0) continue;

            weightedSumX += x * weight;
            weightedSumY += y * weight;
            totalWeight += weight;
        }
    }

    if (totalWeight == 0.0) return Vector2(-1, -1);

    const float xMid = static_cast<float>(weightedSumX / totalWeight);
    const float yMid = static_cast<float>(weightedSumY / totalWeight);

    // Offset by ROI position to get full-frame coordinates
    return Vector2(roi.x + xMid, roi.y + yMid);
}

void ImageProcessor::quadrantize(const std::shared_ptr<Vision>& v)
{
    quadrants_.clear();

    const cv::Mat frame = v->getCurrentFrame();

    const auto totalHeight = static_cast<float>(frame.rows);
    const float availableHeight = totalHeight - QUADRANT::SPACING_Y * (QUADRANT::NUM - 1);
    const float quadrantHeight = availableHeight / QUADRANT::NUM;

    for (int i = 0; i < QUADRANT::NUM; ++i)
    {
        const int startY = static_cast<int>(QUADRANT::OFFSET_Y + static_cast<float>(i) * (quadrantHeight +
            QUADRANT::SPACING_Y));
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
        Vector2 midpoint = q.self(frame);
        lineMidpoints_.push_back(midpoint);
    }

    double timestamp = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();

    fullHistory_.emplace_back(timestamp, lineMidpoints_);
    tempHistory_.emplace_back(timestamp, lineMidpoints_);

    while (!tempHistory_.empty() && timestamp - tempHistory_.front().first > HISTORY::TEMP_DURATION)
    {
        tempHistory_.pop_front();
    }

    if (CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord("Quadrants solved", MessageType::SUCCESS);
    return lineMidpoints_;
}
