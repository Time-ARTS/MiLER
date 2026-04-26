#ifndef MILLER_VECTORTRANSLATOR_HPP
#define MILLER_VECTORTRANSLATOR_HPP

#pragma once
#include <vector>
#include <opencv2/core/mat.hpp>

#include "PID.hpp"
#include "settings.hpp"
#include "../constructs.hpp"

class ControlExtractor
{
public:
    explicit ControlExtractor(const std::shared_ptr<PID>& pid);

    std::vector<double> createLine(const std::vector<Vector2>& points) const;
    std::pair<double, double> createMotorVector(const std::vector<double>& estimatedLine) const;

private:
    static std::tuple<cv::Mat, cv::Mat> computeMatrix(const std::vector<Vector2>& points);
    const std::shared_ptr<PID>& pid_;

    const std::vector<double> DEFAULT_CENTERLINE_ = std::vector<double>(SETTINGS::SCREEN::CENTER_Y, SETTINGS::SCREEN::CENTER_X);

    mutable double last_x_star = SETTINGS::SCREEN::CENTER_X;
};


#endif // MILLER_VECTORTRANSLATOR_HPP
