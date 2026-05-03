#ifndef MILLER_PERSPECTIVETUNER_HPP
#define MILLER_PERSPECTIVETUNER_HPP

#pragma once
#include <opencv2/opencv.hpp>
#include "../constructs.hpp"

class Vision;

class PerspectiveTuner
{
public:
    static void open(Vision& vision, std::string config_path);

private:
    static void drawOverlay(cv::Mat& frame, const Config::Perspective& cfg);
    static cv::Mat computeWarp(const cv::Mat& frame, const Config::Perspective& cfg);
    static void saveToConfig(const Config::Perspective& cfg);

    static std::string config_path_;
};

#endif