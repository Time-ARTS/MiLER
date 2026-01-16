#ifndef MILLER_LOOP_HPP
#define MILLER_LOOP_HPP

#pragma once
#include <memory>
#include <tuple>

#include "settings.hpp"
#include "constructs.hpp"

class ImageProcessor;
class Vision;
class VectorTranslator;
// class PID;
class MotorController;

//extern GPIO activeLED;

void runLoop();

std::tuple<
    std::shared_ptr<ImageProcessor>,
    std::shared_ptr<Vision>,
    std::shared_ptr<VectorTranslator>,
    std::shared_ptr<MotorController>
    // std::shared_ptr<PID>,
> startupp();

void loop(const std::tuple<
    std::shared_ptr<ImageProcessor>,
    std::shared_ptr<Vision>,
    std::shared_ptr<VectorTranslator>,
    std::shared_ptr<MotorController>
    // std::shared_ptr<PID>,    
>& workers);

#endif //MILLER_LOOP_HPP