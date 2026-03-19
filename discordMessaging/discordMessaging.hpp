#ifndef DISCORD_MESSAGING_HPP
#define DISCORD_MESSAGING_HPP

#pragma once

#include <string>
#include "constructs.hpp"

class DiscordMessenger
{
public:
    static void init();
    static void shutdown();
    static std::string truncate(const std::string& str, size_t maxLen);
    static std::string escapeJson(const std::string& input);
    static void sendMessageToDiscord(const std::string& text, const MessageType& type = MessageType::INFO);
};

#endif // DISCORD_MESSAGING_HPP
