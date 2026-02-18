#ifndef MILLER_VECTORTRANSLATOR_HPP
#define MILLER_VECTORTRANSLATOR_HPP

#pragma once
#include <vector>
#include <opencv2/core/mat.hpp>

#include "../constructs.hpp"

class ControlExtractor
{
public:
    static std::vector<double> createLine(const std::vector<Vector2>& points);
    static std::pair<double, double> createMotorLine(const std::vector<double>& estimatedLine);

private:
    static std::tuple<cv::Mat, cv::Mat> computeMatrix(const std::vector<Vector2>& points);
};


#endif // MILLER_VECTORTRANSLATOR_HPP
