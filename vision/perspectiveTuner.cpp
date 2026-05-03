#include "perspectiveTuner.hpp"
#include "vision.hpp"
#include "../config.hpp"
#include "../external/json.hpp"

#include <fstream>
#include <opencv2/opencv.hpp>

static constexpr int SLIDER_SCALE = 1000; // sliders are integers, so we scale 0.0-1.0 -> 0-1000
static constexpr auto WINDOW = "Perspective Tuner";

// Trackbar state (OpenCV needs plain ints)
static int s_vanishing_y = 0;
static int s_top_width = 0;
static int s_bottom_width = 0;

static void noopCallback(int, void*)
{
}

void PerspectiveTuner::open(Vision& vision)
{
    const auto& cfg = ConfigService::get().perspective;

    // Initialise slider positions from current config
    s_vanishing_y = static_cast<int>(cfg.vanishing_y * SLIDER_SCALE);
    s_top_width = static_cast<int>(cfg.trapezoid_top_width * SLIDER_SCALE);
    s_bottom_width = static_cast<int>(cfg.trapezoid_bottom_width * SLIDER_SCALE);

    cv::namedWindow(WINDOW, cv::WINDOW_NORMAL);
    cv::resizeWindow(WINDOW, 800, 700);

    cv::createTrackbar("Vanishing Y  (camera angle)", WINDOW, &s_vanishing_y, SLIDER_SCALE, noopCallback);
    cv::createTrackbar("Top width", WINDOW, &s_top_width, SLIDER_SCALE, noopCallback);
    cv::createTrackbar("Bottom width", WINDOW, &s_bottom_width, SLIDER_SCALE, noopCallback);

    std::cout << "[Tuner] Controls:\n"
        << "  ENTER  - save to config.json and exit\n"
        << "  R      - reset sliders to current config values\n"
        << "  ESC/Q  - exit without saving\n";

    while (true)
    {
        cv::Mat raw = vision.getRawFrame();
        if (raw.empty()) continue;

        // Build a live perspective config from sliders
        Config::Perspective live;
        live.enabled = true;
        live.vanishing_y = s_vanishing_y / static_cast<double>(SLIDER_SCALE);
        live.trapezoid_top_width = s_top_width / static_cast<double>(SLIDER_SCALE);
        live.trapezoid_bottom_width = s_bottom_width / static_cast<double>(SLIDER_SCALE);

        // Left half: raw frame with trapezoid overlay
        cv::Mat display;
        cv::cvtColor(raw, display, cv::COLOR_GRAY2BGR);
        drawOverlay(display, live);

        // Right half: warped result
        cv::Mat warped = computeWarp(raw, live);
        cv::Mat warpedBGR;
        cv::cvtColor(warped, warpedBGR, cv::COLOR_GRAY2BGR);

        // Draw a vertical centerline on warped view so you can
        // check that parallel lines are straight
        cv::line(warpedBGR,
                 {warpedBGR.cols / 2, 0},
                 {warpedBGR.cols / 2, warpedBGR.rows},
                 {91, 206, 250}, 1);

        // Stack side-by-side
        cv::Mat combined;
        cv::hconcat(display, warpedBGR, combined);

        // Labels
        cv::putText(combined, "RAW  (trapezoid)",
                    {10, 24}, cv::FONT_HERSHEY_SIMPLEX, 0.6, {200, 200, 200}, 1);
        cv::putText(combined, "WARPED  (bird's-eye)",
                    {display.cols + 10, 24}, cv::FONT_HERSHEY_SIMPLEX, 0.6, {200, 200, 200}, 1);

        cv::imshow(WINDOW, combined);

        if (const int key = cv::waitKey(30) & 0xFF; key == 13) // ENTER — save and exit
        {
            saveToConfig(live);
            ConfigService::reload(); // reload so the rest of the program sees it
            std::cout << "[Tuner] Saved. Reloaded config.\n";
            break;
        }
        else if (key == 'r' || key == 'R') // reset
        {
            const auto& original = ConfigService::get().perspective;
            s_vanishing_y = static_cast<int>(original.vanishing_y * SLIDER_SCALE);
            s_top_width = static_cast<int>(original.trapezoid_top_width * SLIDER_SCALE);
            s_bottom_width = static_cast<int>(original.trapezoid_bottom_width * SLIDER_SCALE);
            cv::setTrackbarPos("Vanishing Y  (camera angle)", WINDOW, s_vanishing_y);
            cv::setTrackbarPos("Top width", WINDOW, s_top_width);
            cv::setTrackbarPos("Bottom width", WINDOW, s_bottom_width);
            std::cout << "[Tuner] Reset to saved values.\n";
        }
        else if (key == 27 || key == 'q' || key == 'Q') // ESC or Q — discard
        {
            std::cout << "[Tuner] Exited without saving.\n";
            break;
        }
    }

    cv::destroyWindow(WINDOW);
}

void PerspectiveTuner::drawOverlay(cv::Mat& frame, const Config::Perspective& cfg)
{
    const float W = static_cast<float>(frame.cols);
    const float H = static_cast<float>(frame.rows);
    const float vy = static_cast<float>(cfg.vanishing_y) * H;
    const float topH = static_cast<float>(cfg.trapezoid_top_width) * W * 0.5f;
    const float botH = static_cast<float>(cfg.trapezoid_bottom_width) * W * 0.5f;
    const float midX = W * 0.5f;

    const std::vector<cv::Point> pts = {
        {static_cast<int>(midX - topH), static_cast<int>(vy)},
        {static_cast<int>(midX + topH), static_cast<int>(vy)},
        {static_cast<int>(midX + botH), static_cast<int>(H)},
        {static_cast<int>(midX - botH), static_cast<int>(H)},
    };

    // Fill with semi-transparent tint
    cv::Mat overlay = frame.clone();
    cv::fillConvexPoly(overlay, pts, {80, 160, 80});
    cv::addWeighted(overlay, 0.25, frame, 0.75, 0, frame);

    // Draw outline
    for (int i = 0; i < 4; ++i)
        cv::line(frame, pts[i], pts[(i + 1) % 4], {91, 206, 91}, 2);

    // Corner dots
    for (const auto& p : pts)
        cv::circle(frame, p, 5, {250, 206, 91}, -1);
}

cv::Mat PerspectiveTuner::computeWarp(const cv::Mat& frame, const Config::Perspective& cfg)
{
    const float W = static_cast<float>(frame.cols);
    const float H = static_cast<float>(frame.rows);
    const float vy = static_cast<float>(cfg.vanishing_y) * H;
    const float topH = static_cast<float>(cfg.trapezoid_top_width) * W * 0.5f;
    const float botH = static_cast<float>(cfg.trapezoid_bottom_width) * W * 0.5f;
    const float midX = W * 0.5f;

    const std::vector<cv::Point2f> src = {
        {midX - topH, vy}, {midX + topH, vy},
        {midX + botH, H}, {midX - botH, H},
    };
    const std::vector<cv::Point2f> dst = {
        {0, 0}, {W, 0}, {W, H}, {0, H}
    };

    const cv::Mat M = cv::getPerspectiveTransform(src, dst);
    cv::Mat result;
    cv::warpPerspective(frame, result, M, {frame.cols, frame.rows});
    return result;
}

void PerspectiveTuner::saveToConfig(const Config::Perspective& cfg)
{
    std::ifstream in(ConfigService::path());
    nlohmann::json j;
    in >> j;
    in.close();

    j["perspective"]["enabled"] = cfg.enabled;
    j["perspective"]["vanishing_y"] = cfg.vanishing_y;
    j["perspective"]["trapezoid_top_width"] = cfg.trapezoid_top_width;
    j["perspective"]["trapezoid_bottom_width"] = cfg.trapezoid_bottom_width;

    std::ofstream out(ConfigService::path());
    out << j.dump(2);
}
