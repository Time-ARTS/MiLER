#ifndef MILLER_QUADRANTHANDLER_HPP
#define MILLER_QUADRANTHANDLER_HPP

#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "../constructs.hpp"

struct Quadrant {
    cv::roi roi;

    void setROI(int x, int y, int width, int height) { roi = cv::Rect(x, y, width, height); }
    Vector2 processSelf(const cv::Mat& frame) const;
};

class Camera {
public:
    std::vector<Vector2> processQuadrants(const cv::Mat& frame);
    const std::deque<std::pair<float, std::vector<Vector2>>>& getTempHistory() const { return tempHistory_; }
    const std::vector<std::pair<float, std::vector<Vector2>>>& getFullHistory() const { return fullHistory_; }

private:
    std::vector<Quadrant> quadrants_;
    std::vector<Vector2> lineMidpoints_;

    std::deque<std::pair<float, std::vector<Vector2>>> tempHistory_;
    std::vector<std::pair<float, std::vector<Vector2>>> fullHistory_;
};

#endif // MILLER_QUADRANTHANDLER_HPP
