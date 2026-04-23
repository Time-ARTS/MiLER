#ifndef MILER_CONFIG_HPP
#define MILER_CONFIG_HPP

#pragma once

#include <string>

#include "constructs.hpp"

class ConfigService
{
public:
    static void init(const std::string& path);
    static Config& get();

private:
    static Config config_;
};

#endif //MILER_CONFIG_HPP