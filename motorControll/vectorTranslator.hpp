#ifndef MILLER_VECTORTRANSLATOR_HPP
#define MILLER_VECTORTRANSLATOR_HPP

#pragma once
#include <vector>

#include "../settings.hpp"
#include "../constructs.hpp"

class VectorTranslator {
public:
    Speed translate(const std::vector<Vector2>& points, float preferredSpeed);

private:
    Speed speed;
    static Vector2 computeVectors(const std::vector<Vector2>& points);
    static float computeAngularDeviation(const Vector2& vector);
    static float computeCurvature(const std::vector<Vector2>& points);
    Speed computeWheelSpeeds(float baseSpeed, float deviation, float curvature);
};


#endif // MILLER_VECTORTRANSLATOR_HPP