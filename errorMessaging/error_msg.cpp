#include <curl/curl.h>
#include <string>
#include <sstream>

void sendMessageToDiscord(const std::string& text, const std::string& type = "NORMAL")
{
    CURL* curl = curl_easy_init();
    if (!curl)
        return;

    std::string webhook ="https://discord.com/api/webhooks/1461702296311169086/""XsXPRHTLegvDMEkyxFg5xk3tF9MWm8G-_yrDhkDkkw5oGfygZH8HfFwugaMunoYeYan2";

	std::stringstream json;
    if (type == "ERROR")
    {
        json << "{ \"Content\": \"**Error Occurred:** \\n```" << text << "```\" }";
    } else {
        json << "{ \"Content\": \"Message: \\n```" << text << "```\" }";
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, webhook.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.str().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}
