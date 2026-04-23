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
            return { IPStatus::ERROR, "getifaddrs failed" };

        for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_addr) continue;

            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                std::string iface(ifa->ifa_name);

                // Only WiFi interfaces
                if (iface == "wlan0" || iface.rfind("wl", 0) == 0)
                {
                    char host[INET_ADDRSTRLEN];
                    auto* sa = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);

                    inet_ntop(AF_INET, &sa->sin_addr, host, INET_ADDRSTRLEN);

                    std::string ip(host);

                    // Ignore loopback
                    if (ip == "127.0.0.1")
                        continue;

                    freeifaddrs(ifaddr);
                    return { IPStatus::OK, ip };
                }
            }
        }

        freeifaddrs(ifaddr);
        return { IPStatus::NO_IP, "NOT FOUND" };
    }
#endif

    IPResult getWlanIP()
    {
#ifdef __linux__
        return getWlanIP_Linux();
#else
        return { IPStatus::ERROR, "UNSUPPORTED PLATFORM" };
#endif
    }

    void startIPMonitor()
    {
        std::thread([]()
        {
            std::string last_ip = "";
            bool had_ip = false;

            while (true)
            {
                IPResult result = getWlanIP();

                if (result.status == IPStatus::OK)
                {
                    if (!had_ip || result.ip != last_ip)
                    {
                        last_ip = result.ip;
                        had_ip = true;

                        DiscordMessenger::sendMessageToDiscord(
                            "IP Updated | " + result.ip + "\nSSH: pi@" + result.ip,
                            MessageType::SUCCESS,
                            true
                        );
                    }
                }
                else
                {
                    if (had_ip)
                    {
                        had_ip = false;

                        DiscordMessenger::sendMessageToDiscord(
                            "Connection lost (no IP)",
                            MessageType::WARNING,
                            true
                        );
                    }
                }

                std::this_thread::sleep_for(std::chrono::seconds(5));
            }

        }).detach();
    }

}