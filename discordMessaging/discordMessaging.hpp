#ifndef DISCORD_MESSAGING_HPP
#define DISCORD_MESSAGING_HPP

#pragma once

#include <string>
#include "constructs.hpp"
#include "discordConfig.hpp"

class DiscordMessenger
{
public:
    static void init(const DiscordConfig& cfg);
    static void shutdown();
    static void sendMessageToDiscord(const std::string& text, const MessageType& type = MessageType::INFO, const bool& passThrough = false);

private:
    static inline DiscordConfig config_;
};
#endif // DISCORD_MESSAGING_HPP
