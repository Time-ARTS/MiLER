#ifndef MILLER_VECTORTRANSLATOR_HPP
#define MILLER_VECTORTRANSLATOR_HPP

#pragma once
#include <vector>

#include "../constructs.hpp"

class ControlExtractor
{
public:
    static ControlSignals extract(const std::vector<Vector2>& points);

private:
    static Vector2 computeMeanVector(const std::vector<Vector2>& points);
    static float computeDeviation(const Vector2& vector);
    static float computeCurvature(const std::vector<Vector2>& points);
};


#endif // MILLER_VECTORTRANSLATOR_HPP
