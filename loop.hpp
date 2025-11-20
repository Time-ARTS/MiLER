#ifndef MILLER_LOOP_HPP
#define MILLER_LOOP_HPP

#pragma once
#include <memory>
#include <tuple>

class ImageProcessor;
class Vision;
class VectorTranslator;
// class PID;
// class MotorController;

void runLoop();

std::tuple<
    std::shared_ptr<ImageProcessor>,
    std::shared_ptr<Vision>,
    std::shared_ptr<VectorTranslator>
    // std::shared_ptr<PID>,
    // std::shared_ptr<MotorController>
> startupp();

void loop(const std::tuple<
    std::shared_ptr<ImageProcessor>,
    std::shared_ptr<Vision>,
    std::shared_ptr<VectorTranslator>
    // std::shared_ptr<PID>,
    // std::shared_ptr<MotorController>    
>& workers);

#endif //MILLER_LOOP_HPP