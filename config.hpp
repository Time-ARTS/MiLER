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
    const std::string& config_path = "";
};

#endif //MILER_CONFIG_HPP