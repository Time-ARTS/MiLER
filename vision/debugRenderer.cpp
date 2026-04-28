#include "debugRenderer.hpp"

#include "config.hpp"
#include "quadrantHandler.hpp"
#include "discordMessaging/discordMessaging.hpp"

void DebugRenderer::openWindow()
{
    DiscordMessenger::sendMessageToDiscord("Opening debugWindow: MILLER SEARCH VIEW", MessageType::DEBUG);
    cv::namedWindow("MILLER SEARCH VIEW", cv::WINDOW_NORMAL);
}

void DebugRenderer::closeWindow()
{
    DiscordMessenger::sendMessageToDiscord("Closing debugWindow: MILLER SEARCH VIEW", MessageType::DEBUG);
    cv::destroyWindow("MILLER SEARCH VIEW");
}

void DebugRenderer::render(const cv::Mat& frame, const std::vector<Quadrant>& quadrants, const std::vector<Vector2>& points,
                           const std::vector<double>& line, const double& leftSpeed, const double& rightSpeed)
{
    cv::Mat botView;
    cv::cvtColor(frame, botView, cv::COLOR_GRAY2BGR);

    // Quadrants
    for (const auto& [roi] : quadrants)
        cv::rectangle(botView, roi, {184, 169, 245}, 1);

    // Detected points in each quadrant
    for (const auto& [x, y] : points)
        cv::circle(botView, cv::Point(x, y), 5, {250, 206, 91}, -1);

    // Draw centerline
    const auto& config_ = ConfigService::get().screen;
    cv::line(botView, {config_.center_x, config_.center_y}, {config_.center_x, 0}, {91, 206, 250}, 2);

    // Draw the predictive line
    cv::Point prevPoint = { static_cast<int>(line[0]), 0 };
    for (int i = 1; i < line.size(); ++i)
    {
        const cv::Point currPoint = { static_cast<int>(line[i]), i };
        cv::line(botView, prevPoint, currPoint, {0, 206, 0}, 2);
        prevPoint = currPoint;
    }

    // Draw motor-line
    renderMotorLine(botView, static_cast<float>(leftSpeed), static_cast<float>(rightSpeed));

    if (const auto& config = ConfigService::get(); config.modules.debug)
        DiscordMessenger::sendMessageToDiscord("Window successfully rendered: MILLER DEBUG VIEW", MessageType::DEBUG);

    cv::imshow("MILLER DEBUG VIEW", botView);
    cv::waitKey(1);
}

void DebugRenderer::renderMotorLine(const cv::Mat& frame, const float left, const float right)
{
    const float base = static_cast<float>(ConfigService::get().motor.default_speed);
    const float diff = (right - left) / base;

    const auto& config_ = ConfigService::get().screen;
    const cv::Point center = {config_.center_x, config_.center_y};
    constexpr float circleRadius = 50.0f;
    const cv::Scalar circleColor = {0, 0, 255};

    // Draw base circle
    cv::circle(frame, center, circleRadius, circleColor, 2);

    // Map diff [-1, 1] -> angle [-90, 90] degrees, offset by 90 so 0 = straight up
    // diff=0 -> 90deg (up), diff=1 -> 0deg (right), diff=-1 -> 180deg (left)
    const float angleDeg = 90.0f - (diff * 90.0f);
    const float angleRad = angleDeg * CV_PI / 180.0f;

    const cv::Point lineEnd = {
        center.x + static_cast<int>(circleRadius * std::cos(angleRad)),
        center.y - static_cast<int>(circleRadius * std::sin(angleRad))  // y flipped in image space
    };

    // Direction line
    cv::line(frame, center, lineEnd, {0, 0, 255}, 2);
}