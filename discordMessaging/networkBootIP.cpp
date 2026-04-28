#include "discordConfig.hpp"
#include "discordMessaging.hpp"
#include "networkInfo.hpp"

int main()
{
    DiscordMessenger::init(DiscordConfig::loadDiscordConfig("../config.json"));
    NetworkInfo::startIPMonitor();
    return 0;
}
