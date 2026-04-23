#include "discordConfig.hpp"
#include "external/json.hpp"

#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

DiscordConfig DiscordConfig::loadDiscordConfig(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open config file");

    json j;
    file >> j;

    DiscordConfig cfg;
    cfg.webhook_url = j["discord"]["webhook_url"];
    cfg.active = j["modules"]["discord"];
    cfg.logging = j["modules"]["logging"];
    cfg.retry_delay_ms = j["discord"]["retry_delay_ms"];

    return cfg;
}