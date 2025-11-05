#ifndef MILLER_CONSTANTS_HPP
#define MILLER_CONSTANTS_HPP

#pragma once

/*
Titles : // --- Title ---
SubTitles : // SUBTITLE
Descriptions : // Description of what is here
Comment : // ** Ignore **
*/

namespace SETTINGS {

// --- QUADRANT SETTINGS ---
namespace QUADRANT {
    constexpr int NUM = 3;
    constexpr float OFFSET_Y = 0.0f;
    constexpr float SPACING_Y = 0.0f;
}

// --- HISTORY SETTINGS ---
namespace HISTORY {
    constexpr double TEMP_DURATION = 2.0;
}

// --- LINE THRESHOLD SETTINGS ---
namespace LINE_THRESHOLD {
    constexpr int MIN = 50;
    constexpr int MAX = 255;
    constexpr int CANNY_LOW = 50;
    constexpr int CANNY_HIGH = 150;
}

// --- VISION / PROCESSING SETTINGS ---
namespace VISION {
    constexpr int GAUSSIAN_BLUR_KERNEL = 5;
    constexpr double GAUSSIAN_SIGMA = 0.0;

    constexpr double HOUGH_RHO = 1.0;
    constexpr double HOUGH_THETA = CV_PI / 180.0;
    constexpr int HOUGH_THRESHOLD = 50;
    constexpr int HOUGH_MIN_LINE = 50;
    constexpr int HOUGH_MAX_GAP = 10;
}

}

#endif //MILLER_CONSTANTS_HPP