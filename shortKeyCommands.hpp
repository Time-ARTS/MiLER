#ifndef MILER_SHORTKEYCOMMANDS_HPP
#define MILER_SHORTKEYCOMMANDS_HPP

#pragma once
#include <functional>
#include <string>
#include <unordered_set>

class shortKeyCommands {
public:
    shortKeyCommands();

    void poll();
    void bind(int key, std::function<void()> action, const std::string& actionDescriptor = "");

    void flushBindingReport();

private:
    std::unordered_map<int, std::function<void()>> keyBindings_;
    std::string bindingReport_;
};
#endif //MILER_SHORTKEYCOMMANDS_HPP