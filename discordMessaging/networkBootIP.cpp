#include "discordConfig.hpp"
#include "discordMessaging.hpp"
#include "networkInfo.hpp"
#include "settings.hpp"

int main()
{
    DiscordMessenger::init(DiscordConfig::loadDiscordConfig(SETTINGS::CONFIG_PATH));
    NetworkInfo::startIPMonitor();
    return 0;
}
