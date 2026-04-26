#ifndef MILLER_VISION_HPP
#define MILLER_VISION_HPP

#pragma once
#include <memory>
#include <opencv2/opencv.hpp>

#include "quadrantHandler.hpp"

namespace CAMERA
{
#ifdef __linux__
    inline std::string CAMERA_INDEX =
        "libcamerasrc ! "
        "video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! "
        "videoconvert ! "
        "video/x-raw,format=BGR ! "
        "appsink";

    inline std::string CAMERA_INFO = CAMERA_INDEX;
    inline auto CAMERA_SOURCE = cv::CAP_GSTREAMER;
#elif defined(_WIN32)
    inline int CAMERA_INDEX = 0;
    inline std::string CAMERA_INFO = std::to_string(CAMERA_INDEX);
    inline auto CAMERA_SOURCE = cv::CAP_DSHOW;
#else
    inline int CAMERA_INDEX = 0;
    inline std::string CAMERA_INFO = std::to_string(CAMERA_INDEX);
    inline auto CAMERA_SOURCE = cv::CAP_ANY;
#endif
}

class Vision
{
    cv::VideoCapture cap_;
    cv::Mat currentFrame_;
    cv::Size warpSize_;
    cv::Mat perspectiveMatrix_;

public:
    explicit Vision(const std::shared_ptr<ImageProcessor>& imageProcessor);
    cv::Mat getRawFrame();
    cv::Mat getCurrentFrame();
    void initPerspective(int frameWidth, int frameHeight);

    bool detectedLine();
    std::vector<Vector2> processFrame() const { return imageProcessor_->processQuadrants(currentFrame_); }

    ~Vision() { if (cap_.isOpened()) cap_.release(); }

private:
    void camTest() const;
    void frameTest();
    const std::shared_ptr<ImageProcessor>& imageProcessor_;

    bool lineDetected_ = false;
};

#endif //MILLER_VISION_HPP