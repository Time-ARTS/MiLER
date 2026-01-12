#ifndef MILLER_CONSTRUCTS_HPP
#define MILLER_CONSTRUCTS_HPP

#pragma once

#include <gpiod.h>
#include <stdexcept>

struct Vector2 {
    float x;
    float y;
};

struct Speed {
    float left{};
    float right{};
    float deviation{};
};

class GPIO {
    GPIO(int pin) {
        chip = gpiod_chip_open_by_number(4);
        if (!chip) throw std::runtime_error("Failed to open gpiochip4");

        line = gpiod_chip_get_line(chip, pin);
        if (!line) throw std::runtime_error("Failed to get GPIO line");

        if (gpiod_line_request_output(line, "robot-control", 0) < 0)
            throw std::runtime_error("Failed to request line as output");
    }

    ~GPIO() {
        if (line) gpiod_line_release(line);
        if (chip) gpiod_chip_close(chip);
    }

    void set(bool value) {
        gpiod_line_set_value(line, value ? 1 : 0);
    }

    void adaptiveSet(int value, int maxValue = 225) {
        gpiod_line_set_value(line, value != 0);
    }

private:
    gpiod_chip* chip{};
    gpiod_line* line{};
};

#endif //MILLER_CONSTRUCTS_HPP