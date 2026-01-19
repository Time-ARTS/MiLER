#include "loop.hpp"
#include "settings.hpp"
#include "discordMessaging/discordMessaging.hpp"
#include <iostream>

void runLoop() {
    if (!SETTINGS::CONTROL::MODULE::ENABLE_VISION) return;

    loop(startupp());

    if (SETTINGS::CONTROL::MODULE::ENABLE_DISCORD) {
        DiscordMessenger messenger;
        messenger.sendMessageToDiscord("Loop iteration started.", MessageType::DEBUG);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(SETTINGS::CONTROL::THREAD::ROBOT_LOOP_INTERVAL_MS));
}

std::tuple<
    std::shared_ptr<ImageProcessor>,
    std::shared_ptr<Vision>,
    std::shared_ptr<VectorTranslator>,
    std::shared_ptr<MotorController>
> startupp() {
    auto ip = std::make_shared<ImageProcessor>();
    auto v  = std::make_shared<Vision>();
    auto vt = std::make_shared<VectorTranslator>();
    // auto pid = std::make_shared<PID>();
    auto mc = std::make_shared<MotorController>();

    ip->quadrantize(v);

    return {ip, v, vt, mc};
}

void loop(const std::tuple<
    std::shared_ptr<ImageProcessor>,
    std::shared_ptr<Vision>,
    std::shared_ptr<VectorTranslator>,
    std::shared_ptr<MotorController>
    // std::shared_ptr<PID>,
>& workers) 
{
    auto [ip, v, vt, mc] = workers;

    auto points = v->processFrame(ip);
    auto speed = vt->translate(points, SETTINGS::MOTOR::DEFAULT_SPEED);
    //auto correction = pid->computeCorrection(speed.deviation);
    //mc->drive(speed.left + correction, speed.right - correction);

    mc->drive(speed);
}
