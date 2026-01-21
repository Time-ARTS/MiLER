#ifndef MILER_SPEEDGOVENOR_HPP
#define MILER_SPEEDGOVENOR_HPP

class SpeedGovernor {
public:
    static float compute(float curvature, float baseSpeed);

private:
    constexpr static float curvatureGain = 0.5f;
};

#endif //MILER_SPEEDGOVENOR_HPP