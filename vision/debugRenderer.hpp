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
    static void render(const cv::Mat& frame, const std::vector<Quadrant>& quadrants, const std::vector<Vector2>& points, const std::vector<double>& line, const double& leftSpeed, const double& rightSpeed);

private:
    static void renderMotorLine(const cv::Mat& frame, float left, float right);
};

#endif //MILER_DEBUGRENDERER_HPP
