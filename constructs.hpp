#ifndef MILLER_CONSTRUCTS_HPP
#define MILLER_CONSTRUCTS_HPP

#pragma once

constexpr double PI = 3.141592653589793238463;

struct Vector2 {
    float x;
    float y;
};

struct Speed {
    float left{};
    float right{};
    float deviation{};
};

enum class MessageType {
    INFO,
    WARNING,
    ERROR,
    DEBUG,
    SUCCESS
};

#endif //MILLER_CONSTRUCTS_HPP