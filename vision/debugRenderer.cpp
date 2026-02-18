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
    cv::Mat botViewColored;
    cv::cvtColor(frame, botViewColored, cv::COLOR_GRAY2BGR);

    // Quadrants
    for (const auto& [roi] : quadrants)
        cv::rectangle(botViewColored, roi, SETTINGS::QUADRANT::RENDER_QUADRANT, 1);

    // Detected points in each quadrant
    for (const auto& [x, y] : points)
        cv::circle(botViewColored, cv::Point(x, y), 5, SETTINGS::QUADRANT::RENDER_POINT, -1);

    // Draw centerline
    cv::line(botViewColored, SETTINGS::SCREEN::CENTER_BOTTOM, SETTINGS::SCREEN::CENTER_TOP, SETTINGS::QUADRANT::RENDER_CENTERLINE, 2);

    // Draw the predictive line
    cv::Point prevPoint = { static_cast<int>(line[0]), 0 };
    for (int i = 1; i < line.size(); ++i)
    {
        const cv::Point currPoint = { static_cast<int>(line[i]), i };
        cv::line(botViewColored, prevPoint, currPoint, SETTINGS::QUADRANT::RENDER_LINE, 2);
        prevPoint = currPoint;
    }

    // Draw motor-line
    renderMotorLine(botViewColored, static_cast<float>(leftSpeed), static_cast<float>(rightSpeed));

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord("Window successfully rendered: MILLER DEBUG VIEW", MessageType::DEBUG);

    cv::imshow("MILLER DEBUG VIEW", botViewColored);
    cv::waitKey(1);
}

void DebugRenderer::renderMotorLine(const cv::Mat& frame, const float left, const float right)
{
    const float diff = right - left;          // turning direction
    const float curveStrength = diff * 0.02f; // scale factor

    cv::Point prev(SETTINGS::SCREEN::CENTER_X, frame.rows - 1);

    for (int i = 1; i < frame.rows; ++i)
    {
        const float t = i / 120.0f;
        const float bend = curveStrength * t * t;

        const int x = static_cast<int>(SETTINGS::SCREEN::CENTER_X + bend * 300.0f);
        const int y = frame.rows - 1 - i * 2;

        cv::Point curr(x, y);
        cv::line(frame, prev, curr, SETTINGS::QUADRANT::RENDER_MOTOR_LINE, 2);
        prev = curr;
    }

}
