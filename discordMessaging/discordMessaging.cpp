#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef ERROR
#undef ERROR
#endif
#endif

#include "discordMessaging.hpp"
#include "settings.hpp"

#include <curl/curl.h>
#include <unordered_map>
#include <thread>
#include <iostream>
#include <chrono>


void DiscordMessenger::init()
{
#ifdef _WIN32
    curl_global_init(CURL_GLOBAL_DEFAULT);
#endif
}

void DiscordMessenger::shutdown()
{
#ifdef _WIN32
    curl_global_cleanup();
#endif
}

void DiscordMessenger::sendMessageToDiscord(const std::string& text, const MessageType& type)
{
    if constexpr (!SETTINGS::CONTROL::MODULE::ENABLE_DISCORD) return;

    CURL* curl = curl_easy_init();
    if (!curl) return;

    const std::string webhook_url = SETTINGS::CONTROL::DISCORD::WEBHOOK_URL;

    static const std::unordered_map<MessageType, std::string> typeToString = {
        {MessageType::INFO, "INFO"},
        {MessageType::WARNING, "WARNING"},
        {MessageType::ERROR, "ERROR"},
        {MessageType::DEBUG, "DEBUG"},
        {MessageType::SUCCESS, "SUCCESS"}
    };

    static const std::unordered_map<MessageType, int> typeToColor = {
        {MessageType::INFO, 5814783},
        {MessageType::WARNING, 16753920},
        {MessageType::ERROR, 16711680},
        {MessageType::DEBUG, 10070735},
        {MessageType::SUCCESS, 32768}
    };

    const std::string& typeStr = typeToString.at(type);
    const int color = typeToColor.at(type);

    const std::string payload =
        R"({ "username": "MiLER Bot", "embeds": [{"title": ")" + typeStr +
        R"(", "description": ")" + text +
        R"(", "color": )" + std::to_string(color) + "}]}";

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, webhook_url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

    if (const CURLcode res = curl_easy_perform(curl); res != CURLE_OK) {
        std::cerr << "Discord send failed: " << curl_easy_strerror(res)
                  << ", retrying in " << SETTINGS::CONTROL::DISCORD::RETRY_DELAY_MS << "ms\n";

        std::this_thread::sleep_for(
            std::chrono::milliseconds(SETTINGS::CONTROL::DISCORD::RETRY_DELAY_MS)
        );

        curl_easy_perform(curl);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}