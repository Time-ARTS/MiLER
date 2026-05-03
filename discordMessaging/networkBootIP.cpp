#include "discordConfig.hpp"
#include "discordMessaging.hpp"
#include "networkInfo.hpp"

const std::string config_path = "../config.json";

int main()
{
    DiscordMessenger::init(DiscordConfig::loadDiscordConfig(config_path));
    NetworkInfo::startIPMonitor();
    return 0;
}
