#ifndef MILER_DISCORDCONFIG_HPP
#define MILER_DISCORDCONFIG_HPP

#pragma once
#include <string>

struct DiscordConfig
{
    std::string webhook_url;
    bool active;
    bool logging;
    int retry_delay_ms;

    static DiscordConfig loadDiscordConfig(const std::string& path);
};

#endif //MILER_DISCORDCONFIG_HPP