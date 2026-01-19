#include "discordMessaging.hpp"
#include "settings.hpp"
#include <curl/curl.h>
#include <unordered_map>
#include <thread>
#include <iostream>

void DiscordMessenger::sendMessageToDiscord(const std::string& text, const MessageType& type) {
    if (!SETTINGS::CONTROL::MODULE::ENABLE_DISCORD) return;

    CURL* curl = curl_easy_init();
    if (!curl) return;

    std::string webhook_url = SETTINGS::CONTROL::DISCORD::WEBHOOK_URL;

    static const std::unordered_map<MessageType, std::string> typeToString = {
        {MessageType::INFO, "Info"},
        {MessageType::WARNING, "Warning"},
        {MessageType::ERROR, "Error"},
        {MessageType::DEBUG, "Debug"},
        {MessageType::SUCCESS, "Success"}
    };

    static const std::unordered_map<MessageType, int> typeToColor = {
        {MessageType::INFO, 5814783},
        {MessageType::WARNING, 16753920},
        {MessageType::ERROR, 16711680},
        {MessageType::DEBUG, 10070735},
        {MessageType::SUCCESS, 32768}
    };

    std::string typeStr = typeToString.at(type);
    int color = typeToColor.at(type);

    std::string payload =
        "{ \"content\": \"" + text +
        "\", \"username\": \"MiLER Bot\", \"embeds\": [{\"title\": \"" +
        typeStr + "\", \"description\": \"" + text + "\", \"color\": " +
        std::to_string(color) + "}]}";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, webhook_url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Discord send failed: " << curl_easy_strerror(res)
                  << ", retrying in " << SETTINGS::CONTROL::DISCORD::RETRY_DELAY_MS << "ms\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(SETTINGS::CONTROL::DISCORD::RETRY_DELAY_MS));
        curl_easy_perform(curl);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}
