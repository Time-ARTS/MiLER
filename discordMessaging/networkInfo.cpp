#include "networkInfo.hpp"

#include <thread>
#include <chrono>

#include "discordMessaging/discordMessaging.hpp"

#ifdef __linux__
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#endif

namespace NetworkInfo
{
#ifdef __linux__
    static IPResult getWlanIP_Linux()
    {
        struct ifaddrs* ifaddr;

        if (getifaddrs(&ifaddr) == -1)
            return { IPResult::ERROR, "getifaddrs failed" };

        for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_addr) continue;

            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                std::string iface(ifa->ifa_name);

                if (iface == "wlan0" || iface.rfind("wl", 0) == 0)
                {
                    char host[INET_ADDRSTRLEN];
                    auto* sa = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);

                    inet_ntop(AF_INET, &sa->sin_addr, host, INET_ADDRSTRLEN);

                    freeifaddrs(ifaddr);
                    return { IPResult::OK, host };
                }
            }
        }

        freeifaddrs(ifaddr);
        return { IPResult::NO_IP, "NOT FOUND" };
    }
#endif

    IPResult getWlanIP()
    {
#ifdef __linux__
        return getWlanIP_Linux();
#else
        return { IPResult::ERROR, "UNSUPPORTED PLATFORM" };
#endif
    }

    void sendStartupNetworkInfo()
    {
        IPResult result;

#ifdef __linux__
        for (int i = 0; i < 10; ++i)
        {
            result = getWlanIP();

            if (result.status == IPResult::OK)
                break;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (result.status == IPResult::NO_IP)
        {
            DiscordMessenger::sendMessageToDiscord(
                "Startup: wlan IP not found",
                MessageType::WARNING,
                true
            );
        }
        else if (result.status == IPResult::ERROR)
        {
            DiscordMessenger::sendMessageToDiscord(
                result.ip,
                MessageType::ERROR,
                true
            );
        }
        else
        {
            DiscordMessenger::sendMessageToDiscord(
                "System online | IP: " + result.ip + "\nSSH: pi@" + result.ip,
                MessageType::SUCCESS,
                true
            );
        }
#else
        DiscordMessenger::sendMessageToDiscord(
            "System online | Network info unsupported on this platform",
            MessageType::WARNING,
            true
        );
#endif
    }
}
