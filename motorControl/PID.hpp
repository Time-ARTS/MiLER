#ifndef MILLER_PID_HPP
#define MILLER_PID_HPP

class PID
{
public:
    double update(double error, double dt);

private:
    double prevError_ = 0;
    double integral_ = 0;
};

#endif //MILLER_PID_HPP
