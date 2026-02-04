#include "controlExtractor.hpp"
#include "settings.hpp"

#include <vector>

std::vector<double> ControlExtractor::createLine(const std::vector<Vector2>& points)
{
    if (points.empty()) return SETTINGS::LINE_CONTROL::DEFAULT_CENTERLINE;

    const auto [A, B] = computeMatrix(points);

    cv::Mat coeff;
    if (!cv::solve(A, B, coeff, cv::DECOMP_SVD))
    {
        return SETTINGS::LINE_CONTROL::DEFAULT_CENTERLINE;
    }

    std::vector<double> line(480);
    for (int y = 0; y < 480; y++)
    {
        line[y] = coeff.at<double>(0, 0) * y * y + coeff.at<double>(1, 0) * y + coeff.at<double>(2, 0);
    }

    return line;
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