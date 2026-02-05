#include "vision.hpp"
#include "../settings.hpp"
#include "quadrantHandler.hpp"

#include <memory>
#include <opencv2/opencv.hpp>
#include "discordMessaging/discordMessaging.hpp"

#ifdef __linux__
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#endif

Vision::Vision(const std::shared_ptr<ImageProcessor>& imageProcessor)
    : imageProcessor_(imageProcessor)
{
    if (!SETTINGS::CONTROL::MODULE::ENABLE_VISION) return;

#ifdef __linux__
    linuxSetup();
#elif _WIN32
    windowsSetup();
#else
    genericSetup();
#endif

    cameraTest();
}

cv::Mat Vision::getCurrentFrame()
{
    if (!cap_.isOpened())
        throw std::runtime_error("Camera not opened.");

    if (!cap_.read(currentFrame_))
        throw std::runtime_error("Could not read frame from camera.");

    cv::cvtColor(currentFrame_, currentFrame_, cv::COLOR_BGR2GRAY);

    cv::GaussianBlur(
        currentFrame_, currentFrame_,
        cv::Size(SETTINGS::VISION::GAUSSIAN_BLUR_KERNEL, SETTINGS::VISION::GAUSSIAN_BLUR_KERNEL),
        SETTINGS::VISION::GAUSSIAN_SIGMA);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        DiscordMessenger::sendMessageToDiscord("Frame grabbed succsessfully", MessageType::SUCCESS);
    return currentFrame_;
}

bool Vision::detectedLine(cv::Mat* debugMask)
{
    lineDetected_ = false;

    if (!SETTINGS::CONTROL::MODULE::ENABLE_VISION) return false;

    const cv::Mat frame = getCurrentFrame();
    if (frame.empty()) return false;

    cv::Mat mask;
    cv::threshold(frame, mask, SETTINGS::VISION::THRESHOLD_MIN, SETTINGS::VISION::THRESHOLD_MAX, cv::THRESH_BINARY_INV);

    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, {5, 5}));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, {9, 9}));

    if (debugMask) *debugMask = mask.clone();

    const cv::Moments m = cv::moments(mask, true);
    return m.m00 > SETTINGS::VISION::MIN_LINE_AREA;
}

#ifdef __linux__

static int getCameraPriority(const v4l2_capability& cap)
{
    std::string card(reinterpret_cast<const char*>(cap.card));
    std::string driver(reinterpret_cast<const char*>(cap.driver));

    int best = INT_MAX;

    for (const auto& [key, rank] : SETTINGS::VISION::CAMERA_PRIORITY)
    {
        if (card.find(key) != std::string::npos ||
            driver.find(key) != std::string::npos)
        {
            best = std::min(best, rank);
        }
    }

    return (best == INT_MAX) ? -1 : best;
}

int autoDetectCameraIndex()
{
    int bestIndex = -1;
    int bestPriority = INT_MAX;

    for (int i = 0; i < SETTINGS::VISION::MAX_VIDEO_DEVICES; ++i)
    {
        std::string dev = "/dev/video" + std::to_string(i);
        int fd = open(dev.c_str(), O_RDONLY | O_NONBLOCK);

        if (fd < 0)
            continue;

        v4l2_capability cap{};
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0)
        {
            int prio = getCameraPriority(cap);

            std::string card(reinterpret_cast<char*>(cap.card));
            std::string driver(reinterpret_cast<char*>(cap.driver));

            if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
            {
                DiscordMessenger::sendMessageToDiscord(
                    "Camera: " + card + " | Driver: " + driver +
                    " | Index: " + std::to_string(i) +
                    " | Priority: " + (prio >= 0 ? std::to_string(prio) : "none"),
                    MessageType::INFO
                );
            }

            if (prio >= 0 && prio < bestPriority)
            {
                bestPriority = prio;
                bestIndex = i;

                if (bestPriority == 0)
                    break;
            }
        }

        close(fd);
    }

    return bestIndex >= 0 ? bestIndex : 0; // fallback
}

void Vision::linuxSetup()
{
    int camIndex = SETTINGS::VISION::CAMERA_INDEX;

    if (SETTINGS::VISION::AUTO_DETECT_CAMERA)
    {
        camIndex = autoDetectCameraIndex();
    }

    if (camIndex < 0)
        camIndex = 0;

    cap_.open(camIndex, SETTINGS::VISION::CAMERA_SOURCE);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
    {
        DiscordMessenger::sendMessageToDiscord(
            "Linux camera opened with index: " + std::to_string(camIndex),
            MessageType::SUCCESS
        );
    }
}
#elif WIN32

void Vision::windowsSetup()
{
    int camIndex = SETTINGS::VISION::CAMERA_INDEX;

    if (camIndex < 0) camIndex = 0;

    cap_.open(camIndex, SETTINGS::VISION::CAMERA_SOURCE);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
    {
        DiscordMessenger::sendMessageToDiscord(
            "Windows camera opened with index: " + std::to_string(camIndex),
            MessageType::SUCCESS
        );
    }
}
#else
void Vision::generalSetup()
{
    int camIndex = SETTINGS::VISION::CAMERA_INDEX;

    if (camIndex < 0) camIndex = 0;

    cap_.open(camIndex);

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
    {
        DiscordMessenger::sendMessageToDiscord(
            "Generic camera backend used. Index: " + std::to_string(camIndex),
            MessageType::INFO
        );
    }
}
#endif

void Vision::cameraTest()
{
#ifdef __linux__
    if (!cap_.isOpened())
    {
        throw std::runtime_error("Camera not opened for test.");
    }

    cv::Mat testFrame;
    bool success = false;

    for (int attempt = 0; attempt < SETTINGS::VISION::MAX_TEST_ATTEMPTS; ++attempt)
    {
        if (cap_.read(testFrame) && !testFrame.empty())
        {
            success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    if (!success)
        throw std::runtime_error("Failed to grab a frame from the camera after multiple attempts.");

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
    {
        const int backend = static_cast<int>(cap_.get(cv::CAP_PROP_BACKEND));

        const int width = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_WIDTH));
        const int height = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_HEIGHT));

        int fd = open(("/dev/video" + std::to_string(SETTINGS::VISION::CAMERA_INDEX)).c_str(),
                      O_RDONLY | O_NONBLOCK);
        std::string cardName = "Unknown";
        std::string driverName = "Unknown";

        if (fd >= 0)
        {
            v4l2_capability capInfo{};
            if (ioctl(fd, VIDIOC_QUERYCAP, &capInfo) == 0)
            {
                cardName = reinterpret_cast<char*>(capInfo.card);
                driverName = reinterpret_cast<char*>(capInfo.driver);
            }
            close(fd);
        }

        if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
        {
            std::string infoMsg = "Camera Test Passed: \n";
            infoMsg += "Card: " + cardName + "\n";
            infoMsg += "Driver: " + driverName + "\n";
            infoMsg += "Index: " + std::to_string(SETTINGS::VISION::CAMERA_INDEX) + "\n";
            infoMsg += "Resolution: " + std::to_string(width) + "x" + std::to_string(height) + "\n";
            infoMsg += "Backend: " + std::to_string(backend);

            DiscordMessenger::sendMessageToDiscord(infoMsg, MessageType::SUCCESS);

            cv::imshow("Camera Test Preview", testFrame);
            cv::waitKey(3000);
            cv::destroyWindow("Camera Test Preview");
        }
    }

#else
    if (!cap_.isOpened())
        throw std::runtime_error("Camera not opened for test.");

    cv::Mat testFrame;
    if (!cap_.read(testFrame) || testFrame.empty())
        throw std::runtime_error("Failed to grab a frame from the camera.");

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
    {
        const int backend = static_cast<int>(cap_.get(cv::CAP_PROP_BACKEND));
        const int width = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_WIDTH));
        const int height = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_HEIGHT));

        std::string infoMsg = "Camera Test Passed (Windows/Generic):\n";
        infoMsg += "Index: " + std::to_string(SETTINGS::VISION::CAMERA_INDEX) + "\n";
        infoMsg += "Resolution: " + std::to_string(width) + "x" + std::to_string(height) + "\n";
        infoMsg += "Backend: " + std::to_string(backend);

        DiscordMessenger::sendMessageToDiscord(infoMsg, MessageType::SUCCESS);

        cv::imshow("Camera Test Preview", testFrame);
        cv::waitKey(3000);
        cv::destroyWindow("Camera Test Preview");
    }
#endif
}
