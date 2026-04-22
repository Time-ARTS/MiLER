#ifndef MILLER_VECTORTRANSLATOR_HPP
#define MILLER_VECTORTRANSLATOR_HPP

#pragma once
#include <vector>
#include <opencv2/core/mat.hpp>

#include "PID.hpp"
#include "../constructs.hpp"

class ControlExtractor
{
public:
    explicit ControlExtractor(const std::shared_ptr<PID>& pid);

    static std::vector<double> createLine(const std::vector<Vector2>& points);
    std::pair<double, double> createMotorVector(const std::vector<double>& estimatedLine) const;

private:
    static std::tuple<cv::Mat, cv::Mat> computeMatrix(const std::vector<Vector2>& points);
    const std::shared_ptr<PID>& pid_;
};


#endif // MILLER_VECTORTRANSLATOR_HPP
