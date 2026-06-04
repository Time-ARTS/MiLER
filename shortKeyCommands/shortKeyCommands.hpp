#ifndef MILER_SHORTKEYCOMMANDS_HPP
#define MILER_SHORTKEYCOMMANDS_HPP

#pragma once
#include <functional>
#include <string>

#include "constructs.hpp"

struct Trigger;

class shortKeyCommands {
public:
    shortKeyCommands();
    void bind(const std::string& stringKey, std::function<void()> action);
    void addTrigger(const std::string& stringKey, std::function<bool()> condition);

    void poll();
    void trigger(int key);

private:
    std::unordered_map<int, std::function<void()>> keyBindings_;
    std::vector<Trigger> triggers_;

#ifdef __linux__
    void enableRawMode();
    void disableRawMode();
    bool kbhitLinux();
    int getchLinux();
#endif
};
#endif //MILER_SHORTKEYCOMMANDS_HPP