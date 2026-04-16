#include "debugRenderer.hpp"

#include "quadrantHandler.hpp"
#include "../settings.hpp"
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
        cv::rectangle(botView, roi, SETTINGS::QUADRANT::RENDER_QUADRANT, 1);

    // Detected points in each quadrant
    for (const auto& [x, y] : points)
        cv::circle(botView, cv::Point(x, y), 5, SETTINGS::QUADRANT::RENDER_POINT, -1);

    // Draw centerline
    cv::line(botView, SETTINGS::SCREEN::CENTER_BOTTOM, SETTINGS::SCREEN::CENTER_TOP, SETTINGS::QUADRANT::RENDER_CENTERLINE, 2);

    // Draw the predictive line
    cv::Point prevPoint = { static_cast<int>(line[0]), 0 };
    for (int i = 1; i < line.size(); ++i)
    {
        const cv::Point currPoint = { static_cast<int>(line[i]), i };
        cv::line(botView, prevPoint, currPoint, SETTINGS::QUADRANT::RENDER_LINE, 2);
        prevPoint = currPoint;
    }

    // Draw motor-line
    renderMotorLine(botView, static_cast<float>(leftSpeed), static_cast<float>(rightSpeed));

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord("Window successfully rendered: MILLER DEBUG VIEW", MessageType::DEBUG);

    cv::imshow("MILLER DEBUG VIEW", botView);
    cv::waitKey(1);
}

void DebugRenderer::renderMotorLine(const cv::Mat& frame, const float left, const float right)
{
    const float diff = right - left;          // turning direction
    const float avgSpeed = (left + right) / 2.0f;  // overall speed

    const cv::Point center = SETTINGS::SCREEN::CENTER_BOTTOM;
    constexpr float circleRadius = SETTINGS::QUADRANT::CIRCLE_RADIUS;
    const auto circleColor = SETTINGS::QUADRANT::RENDER_MOTOR_LINE;

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
    cv::line(frame, center, lineEnd, SETTINGS::QUADRANT::RENDER_MOTOR_LINE, 2);
}