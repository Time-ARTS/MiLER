#include "controlExtractor.hpp"
#include "settings.hpp"
#include "PID.hpp"

#include <vector>
#include <numeric>
#include <ranges>
#include <cmath>

ControlExtractor::ControlExtractor(const std::shared_ptr<PID>& pid) : pid_(pid)
{
}

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

std::pair<double, double> ControlExtractor::createMotorVector(const std::vector<double>& estimatedLine) const
{
    if (estimatedLine.empty())
        return {SETTINGS::MOTOR::DEFAULT_SPEED, SETTINGS::MOTOR::DEFAULT_SPEED};

    const auto H = static_cast<int>(estimatedLine.size());
    constexpr double centerX = SETTINGS::SCREEN::CENTER_X;

    constexpr double y0    = SETTINGS::LINE_CONTROL::WEIGHT_CENTER;
    constexpr double sigma = SETTINGS::LINE_CONTROL::WEIGHT_SIGMA;

    // Create index view [0, H)
    auto indices = std::views::iota(0, H);

    // Accumulate weighted sum using transform_reduce
    const auto [num, den] = std::transform_reduce(
        indices.begin(), indices.end(),
        std::pair{0.0, 0.0},
        [](auto a, auto b) {
            return std::pair{a.first + b.first, a.second + b.second};
        },
        [&](const int y) {
            const double y_prime = (H - 1) - y;
            const double w = std::exp(-std::pow(y_prime - y0, 2) / (2 * sigma * sigma));
            return std::pair{w * estimatedLine[y], w};
        }
    );

    const double x_star = num / den;

    // --- Local slope (central difference) ---
    const int y_center = std::clamp<int>(H - 1 - static_cast<int>(y0), 1, H - 2);

    const double slope =
        (estimatedLine[y_center + 1] - estimatedLine[y_center - 1]) / 2.0;

    // --- Error model ---
    const double offset = x_star - centerX;

    const double error =
        offset * SETTINGS::LINE_CONTROL::OFFSET_GAIN +
        slope  * SETTINGS::LINE_CONTROL::ANGLE_GAIN;

    // --- Motor output ---
    constexpr double base = SETTINGS::MOTOR::DEFAULT_SPEED;
    constexpr double dt = SETTINGS::CONTROL::THREAD::ROBOT_LOOP_INTERVAL_MS / 1000.0f;

    const double control = pid_->update(error, dt);

    const double left  = std::clamp(base + control, SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);
    const double right = std::clamp(base - control, SETTINGS::MOTOR::MIN_SPEED, SETTINGS::MOTOR::MAX_SPEED);

    return {left, right};
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