#ifndef MILER_SPEEDGOVENOR_HPP
#define MILER_SPEEDGOVENOR_HPP
#include <vector>

class SpeedGovernor
{
public:
    static double compute(double curvature);
    static double compute(const std::vector<double>& line);
};

#endif //MILER_SPEEDGOVENOR_HPP
