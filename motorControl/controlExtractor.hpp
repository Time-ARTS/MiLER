#ifndef MILLER_VECTORTRANSLATOR_HPP
#define MILLER_VECTORTRANSLATOR_HPP

#pragma once
#include <vector>

#include "../constructs.hpp"

class ControlExtractor
{
public:
    static std::vector<double> createLine(const std::vector<Vector2>& points);

    private:
    static std::tuple<cv::Mat, cv::Mat> computeMatrix(const std::vector<Vector2>& points);
};


#endif // MILLER_VECTORTRANSLATOR_HPP
