#ifndef MILLER_PID_HPP
#define MILLER_PID_HPP

class PID
{
public:
    PID(const float kp, const float ki, const float kd)
        : kp_(kp), ki_(ki), kd_(kd)
    {
    }

    float compute(float error, float dt);

private:
    float kp_, ki_, kd_;
    float prevError_ = 0;
    float integral_ = 0;
};

#endif //MILLER_PID_HPP
