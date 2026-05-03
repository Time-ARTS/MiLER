#ifndef MILER_CONFIG_HPP
#define MILER_CONFIG_HPP

#pragma once
#include <string>
#include "constructs.hpp"

class ConfigService
{
public:
    static void init(const std::string& path);
    static void reload();
    static Config& get() { return config_; }
    static const std::string& path() { return path_; }

private:
    static Config config_;
    static std::string path_;
};

#endif
