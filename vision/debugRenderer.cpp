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
    redMask.setTo(cv::Scalar(0,0,255), mask);

    cv::addWeighted(display, 1.0, redMask, 0.5, 0, display);

    cv::imshow("MILLER SEARCH VIEW", display);
    cv::waitKey(1);
}

void DebugRenderer::render(cv::Mat& frame,
                           const std::vector<Quadrant>& quadrants, const std::vector<Vector2>& points,
                           const float deviation, const float curvature, const float turnCorrection, const float speed)
{
    for (const auto& [roi] : quadrants)
        cv::rectangle(frame, roi, {255, 0, 0}, 1);

    for (const auto [x,y] : points)
    {
        cv::circle(frame, cv::Point(x, y), 5, {255, 0, 0}, -1);
    }

    // Draw PID steering vector
    const cv::Point origin(frame.cols / 2, frame.rows - 20);
    const cv::Point steer(origin.x + static_cast<int>(turnCorrection * 120),
                          origin.y - 80);

    cv::arrowedLine(frame, origin, steer, {0, 0, 255}, 3);

    // HUD
    cv::putText(frame, "Deviation: " + std::to_string(deviation), {10, 20},
                cv::FONT_HERSHEY_SIMPLEX, 0.5, {200, 200, 200}, 1);

    cv::putText(frame, "Turn: " + std::to_string(turnCorrection), {10, 40},
                cv::FONT_HERSHEY_SIMPLEX, 0.5, {200, 200, 200}, 1);

    cv::putText(frame, "Speed: " + std::to_string(speed), {10, 60},
                cv::FONT_HERSHEY_SIMPLEX, 0.5, {200, 200, 200}, 1);

    cv::imshow("MILLER DEBUG VIEW", frame);
    cv::waitKey(1);
}
