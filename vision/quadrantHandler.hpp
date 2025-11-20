#ifndef MILLER_QUADRANTHANDLER_HPP
#define MILLER_QUADRANTHANDLER_HPP

#pragma once
#include <deque>
#include <memory>
#include <vector>
#include <opencv2/opencv.hpp>
#include "../settings.hpp"
#include "../constructs.hpp"

class Vision;

struct Quadrant {
    cv::Rect roi;

    void setROI(int x, int y, int width, int height) { roi = cv::Rect(x, y, width, height); }
    Vector2 self(const cv::Mat& frame) const;
};

class ImageProcessor {
public:
    void quadrantize(const std::shared_ptr<Vision>& v);
    std::vector<Vector2> processQuadrants(const cv::Mat& frame);

    const std::vector<Vector2>& getLinePoints() const { return lineMidpoints_; }
    const std::deque<std::pair<float, std::vector<Vector2>>>& getTempHistory() const { return tempHistory_; }
    const std::vector<std::pair<float, std::vector<Vector2>>>& getFullHistory() const { return fullHistory_; }

private:
    std::vector<Quadrant> quadrants_;
    std::vector<Vector2> lineMidpoints_;

    std::deque<std::pair<float, std::vector<Vector2>>> tempHistory_;
    std::vector<std::pair<float, std::vector<Vector2>>> fullHistory_;
};

#endif // MILLER_QUADRANTHANDLER_HPP
