#include <memory>
#include <tuple>

#include "loop.hpp"
#include "settings.hpp"
#include "vision/vision.hpp"
#include "vision/quadrantHandler.hpp"
#include "motorControll/vectorTranslator.hpp"
#include "motorControll/PID.hpp"
#include "motorControll/motorController.hpp"

void runLoop() {
    loop(startupp());
}

std::tuple<
    std::shared_ptr<ImageProcessor>,
    std::shared_ptr<Vision>,
    std::shared_ptr<VectorTranslator>
    // std::shared_ptr<PID>,
    // std::shared_ptr<MotorController>
> startupp() 
{
    auto ip = std::make_shared<ImageProcessor>();
    auto v  = std::make_shared<Vision>();
    auto vt = std::make_shared<VectorTranslator>();
    // auto pid = std::make_shared<PID>();
    // auto mc = std::make_shared<MotorController>();

    ip->quadrantize(v);



    return {ip, v, vt};
}


void loop(const std::tuple<
    std::shared_ptr<ImageProcessor>,
    std::shared_ptr<Vision>,
    std::shared_ptr<VectorTranslator>,
    std::shared_ptr<MotorController>
    // std::shared_ptr<PID>,
>& workers) {
    auto [ip, v, vt, mc] = workers;

    activeLED.set(true);

    auto points = v->processFrame(ip);
    auto speed = vt->translate(points, SETTINGS::MOTOR::DEFAULT_SPEED);
    //auto correction = pid->computeCorrection(speed.deviation);
    //mc->drive(speed.left + correction, speed.right - correction);

    mc->drive(speed);

    activeLED.set(false);
}