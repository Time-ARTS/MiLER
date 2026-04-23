#include "controlExtractor.hpp"
#include "settings.hpp"
#include "PID.hpp"

#include <vector>
#include <numeric>
#include <ranges>
#include <cmath>
#include <opencv2/core.hpp>

#include "config.hpp"

ControlExtractor::ControlExtractor(const std::shared_ptr<PID>& pid) : pid_(pid)
{
}

std::vector<double> ControlExtractor::createLine(const std::vector<Vector2>& points) const
{
    std::vector<Vector2> validPoints;
    for (const auto& p : points)
    {
        if (p.x >= 0 && p.y >= 0)
            validPoints.push_back(p);
    }

    if (validPoints.size() < 3)
        return DEFAULT_CENTERLINE_;

    const auto [A, B] = computeMatrix(validPoints);

    cv::Mat coeff;
    if (!cv::solve(A, B, coeff, cv::DECOMP_SVD))
        return DEFAULT_CENTERLINE_;

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
    const auto& config_ = ConfigService::get();

    const double base = config_.motor.default_speed;

    if (estimatedLine.empty())
        return {base, base};

    const int H = static_cast<int>(estimatedLine.size());
    const double centerX = SETTINGS::SCREEN::CENTER_X;

    // --- Gaussian weighted center ---
    const double y0    = config_.line_control.weight_center;
    const double sigma = config_.line_control.weight_sigma;

    double num = 0.0;
    double den = 0.0;

    for (int y = 0; y < H; ++y)
    {
        const double y_prime = (H - 1) - y;
        const double w = std::exp(-std::pow(y_prime - y0, 2) / (2 * sigma * sigma));

        num += w * estimatedLine[y];
        den += w;
    }

    // --- Safety check ---
    if (den < 1e-6)
        return {base, base};

    double raw_x_star = num / den;

    // --- Temporal smoothing (critical) ---
    static double last_x_star = SETTINGS::SCREEN::CENTER_X;
    const double alpha = 0.2; // responsiveness vs stability

    double x_star = alpha * raw_x_star + (1.0 - alpha) * last_x_star;
    last_x_star = x_star;

    // --- NORMALIZED error [-1, 1] ---
    double error = (x_star - centerX) / centerX;

    // Clamp just in case of numerical spikes
    error = std::clamp(error, -1.0, 1.0);

    // --- PID ---
    const double dt = config_.timing.robot_loop_interval_ms / 1000.0;
    double control = pid_->update(error, dt);

    // --- Scale control to motor domain ---
    const double control_max = base * 0.5; // max turning authority
    control *= control_max;

    // --- Clamp control (prevents flips) ---
    control = std::clamp(control, -control_max, control_max);

    // --- Motor mix ---
    double left  = base + control;
    double right = base - control;

    // --- Final hardware clamp ---
    left  = std::clamp(left,  config_.motor.min_speed, config_.motor.max_speed);
    right = std::clamp(right, config_.motor.min_speed, config_.motor.max_speed);

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