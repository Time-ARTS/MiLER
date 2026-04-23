#ifndef NETWORK_INFO_HPP
#define NETWORK_INFO_HPP

#pragma once

#include <string>

enum class IPStatus
{
    OK,
    NO_IP,
    ERROR
};

struct IPResult
{
    IPStatus status;
    std::string ip;
};

namespace NetworkInfo
{
    IPResult getWlanIP();
    void startIPMonitor();
}

#endif // NETWORK_INFO_HPP