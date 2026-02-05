#ifndef MILER_DEBUGRENDERER_HPP
#define MILER_DEBUGRENDERER_HPP

#pragma once
#include <opencv2/opencv.hpp>
#include "constructs.hpp"
#include "quadrantHandler.hpp"

class DebugRenderer
{
public:
    static void openWindow();
    static void closeWindow();
    static void renderSearchView(const cv::Mat& frame, const cv::Mat& mask);
    static void render(cv::Mat& frame, const std::vector<Quadrant>& quadrants, const std::vector<Vector2>& points, const std::vector<double>& line, const DriveCommand& cmd);
};

#endif //MILER_DEBUGRENDERER_HPP
