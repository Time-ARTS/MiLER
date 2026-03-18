#include "controlExtractor.hpp"
#include "settings.hpp"

#include <vector>

std::vector<double> ControlExtractor::createLine(const std::vector<Vector2>& points)
{
    std::vector<Vector2> validPoints;
    for (const auto& p : points)
    {
        if (p.x >= 0 && p.y >= 0)
            validPoints.push_back(p);
    }

    if (validPoints.size() < 3)
        return SETTINGS::LINE_CONTROL::DEFAULT_CENTERLINE;

    const auto [A, B] = computeMatrix(validPoints);

    cv::Mat coeff;
    if (!cv::solve(A, B, coeff, cv::DECOMP_SVD))
        return SETTINGS::LINE_CONTROL::DEFAULT_CENTERLINE;

    std::vector<double> line(480);
    for (int y = 0; y < 480; y++)
    {
        double x = coeff.at<double>(0,0)*y*y + coeff.at<double>(1,0)*y + coeff.at<double>(2,0);
        line[y] = std::clamp(x, 0.0, static_cast<double>(SETTINGS::SCREEN::CENTER_X*2 - 1));
    }

    return line;
}

std::pair<double, double> ControlExtractor::createMotorVector(const std::vector<double>& estimatedLine)
{
    if (estimatedLine.empty())
        return {SETTINGS::MOTOR::DEFAULT_SPEED, SETTINGS::MOTOR::DEFAULT_SPEED};

    // Compute error between estimated line and robot center
    const double lookaheadY = std::min<int>(estimatedLine.size() - 1, 20);

    const double lateralError = estimatedLine[lookaheadY] - SETTINGS::SCREEN::CENTER_X; // +ve -> need right curve, -ve -> left curve

    // Translate lateralError to differential drive speeds
    // Use configurable parameters like wheelbase and speed factor
    constexpr double curvatureGain = SETTINGS::LINE_CONTROL::CURVATURE_GAIN; // tunable
    constexpr double speed = SETTINGS::MOTOR::DEFAULT_SPEED;

    double leftSpeed, rightSpeed;

    // Motor speeds calculated so that the resulting curve moves laterally toward target
    if (lateralError > 0)
    {
        // Curve right: slow left motor
        leftSpeed  = speed;
        rightSpeed = speed * std::max(0.0, 1.0 - curvatureGain * lateralError);
    }
    else if (lateralError < 0)
    {
        // Curve left: slow right motor
        leftSpeed  = speed * std::max(0.0, 1.0 + curvatureGain * lateralError);
        rightSpeed = speed;
    }
    else
    {
        leftSpeed  = speed;
        rightSpeed = speed; // straight
    }

    // Clamp speeds to allowed range
    leftSpeed  = std::clamp(leftSpeed, 0.0, speed);
    rightSpeed = std::clamp(rightSpeed, 0.0, speed);

    return {leftSpeed, rightSpeed};
}

std::tuple<cv::Mat, cv::Mat> ControlExtractor::computeMatrix(const std::vector<Vector2>& points)
{
    cv::Mat A = cv::Mat::zeros(3, 3, CV_64F);
    cv::Mat B = cv::Mat::zeros(3, 1, CV_64F);

    for (const auto& [x, y] : points)
    {
        const double y2 = y * y;
        const double y3 = y2 * y;
        const double y4 = y3 * y;

        A.at<double>(0, 0) += y4;
        A.at<double>(0, 1) += y3;
        A.at<double>(0, 2) += y2;
        A.at<double>(1, 0) += y3;
        A.at<double>(1, 1) += y2;
        A.at<double>(1, 2) += y;
        A.at<double>(2, 0) += y2;
        A.at<double>(2, 1) += y;
        A.at<double>(2, 2) += 1;

        B.at<double>(0, 0) += x * y2;
        B.at<double>(1, 0) += x * y;
        B.at<double>(2, 0) += x;
    }

    return {A, B};
}