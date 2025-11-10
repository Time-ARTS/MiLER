#ifndef MILLER_VISION_HPP
#define MILLER_VISION_HPP

#pragma once
#include "quadrantHandler.hpp"

class Vision {
    cv::VideoCapture cap_;
    cv::Mat currentFrame_;
    
    public:
    Vision();
    static cv::Mat getCurrentFrame();

    static bool detectedLine();
    static std::vector<Vector2> processFrame();

    ~Vision() { if (cap_.isOpened()) cap_.release(); }
};

#endif //MILLER_VISION_HPP