#ifndef MILLER_VISION_HPP
#define MILLER_VISION_HPP

#pragma once
#include <memory>
#include <opencv2/opencv.hpp>

#include "quadrantHandler.hpp"

class Vision
{
    cv::VideoCapture cap_;
    cv::Mat currentFrame_;

public:
    explicit Vision(const std::shared_ptr<ImageProcessor>& imageProcessor);
    cv::Mat getCurrentFrame();

    bool detectedLine();
    std::vector<Vector2> processFrame() const { return imageProcessor_->processQuadrants(currentFrame_); }

    ~Vision() { if (cap_.isOpened()) cap_.release(); }

private:
    const std::shared_ptr<ImageProcessor>& imageProcessor_;

    bool lineDetected_ = false;
};

#endif //MILLER_VISION_HPP
