#ifndef MILLER_VISION_HPP
#define MILLER_VISION_HPP

#pragma once
#include <memory>
#include <opencv2/opencv.hpp>

#include "quadrantHandler.hpp"

class Vision {
    cv::VideoCapture cap_;
    cv::Mat currentFrame_;
    
    public:
    Vision();
    cv::Mat getCurrentFrame();

    bool detectedLine();
    std::vector<Vector2> processFrame(const std::shared_ptr<ImageProcessor>& ip);

    ~Vision() { if (cap_.isOpened()) cap_.release(); }
};

#endif //MILLER_VISION_HPP