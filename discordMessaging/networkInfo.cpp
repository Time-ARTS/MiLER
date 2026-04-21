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
    static std::string getWlanIP_Linux()
    {
        struct ifaddrs* ifaddr;
        struct ifaddrs* ifa;

        if (getifaddrs(&ifaddr) == -1)
            return "ERROR: getifaddrs failed";

        std::string ip = "NOT FOUND";

        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_addr) continue;

            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                std::string iface(ifa->ifa_name);

                // Prefer wlan0, fallback to any wireless interface
                if (iface == "wlan0" || iface.rfind("wl", 0) == 0)
                {
                    char host[NI_MAXHOST];
                    auto* sa = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);

                    inet_ntop(AF_INET, &sa->sin_addr, host, NI_MAXHOST);
                    ip = host;
                    break;
                }
            }
        }

        freeifaddrs(ifaddr);
        return ip;
    }
#endif

    std::string getWlanIP()
    {
#ifdef __linux__
        return getWlanIP_Linux();
#else
        return "UNSUPPORTED PLATFORM";
#endif
    }

    void sendStartupNetworkInfo()
    {
        std::string ip;

        for (int i = 0; i < 10; ++i)
        {
            ip = getWlanIP();

            if (ip != "NOT FOUND")
                break;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

#ifdef __linux__
        if (ip == "NOT FOUND")
        {
            DiscordMessenger::sendMessageToDiscord(
                "Startup: wlan IP not found",
                MessageType::WARNING,
                true
            );
        }
        else if (ip.rfind("ERROR", 0) == 0)
        {
            DiscordMessenger::sendMessageToDiscord(
                ip,
                MessageType::ERROR,
                true
            );
        }
        else
        {
            DiscordMessenger::sendMessageToDiscord(
                "System online | IP: " + ip + "\nSSH: pi@" + ip,
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