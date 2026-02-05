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

void DebugRenderer::renderSearchView(const cv::Mat& frame, const cv::Mat& mask)
{
    cv::Mat display;
    cv::cvtColor(frame, display, cv::COLOR_GRAY2BGR);

    // Overlay mask in red
    cv::Mat redMask;
    cv::cvtColor(mask, redMask, cv::COLOR_GRAY2BGR);
    redMask.setTo(cv::Scalar(0, 0, 255), mask);

    cv::addWeighted(display, 1.0, redMask, 0.5, 0, display);

    DiscordMessenger::sendMessageToDiscord("Window successfully rendered: MILLER SEARCH VIEW", MessageType::DEBUG);

    cv::imshow("MILLER SEARCH VIEW", display);
    cv::waitKey(1);
}

void DebugRenderer::render(cv::Mat& frame, const std::vector<Quadrant>& quadrants, const std::vector<Vector2>& points,
                           const std::vector<double>& line, const DriveCommand& cmd)
{
    for (const auto& [roi] : quadrants)
        cv::rectangle(frame, roi, {255, 0, 0}, 1);

    for (const auto [x,y] : points)
    {
        cv::circle(frame, cv::Point(x, y), 5, {255, 0, 0}, -1);
    }

    // Draw the predictive line
    cv::Point prevPoint = {static_cast<int>(line[0]), 0};
    cv::Point currPoint{};
    for (int i = 1; i < line.size(); i++)
    {
        currPoint = {static_cast<int>(line[i]), i};
        cv::line(frame, prevPoint, currPoint, {127, 127, 127}, 2);
        prevPoint = currPoint;
    }

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord("Window successfully rendered: MILLER DEBUG VIEW", MessageType::DEBUG);

    cv::imshow("MILLER DEBUG VIEW", frame);
    cv::waitKey(1);
}