#ifndef NETWORK_INFO_HPP
#define NETWORK_INFO_HPP

#pragma once
#include <string>

namespace NetworkInfo
{
    struct IPResult
    {
        enum Status { OK, NO_INTERFACE, NO_IP, ERROR };

        Status status;
        std::string ip;
    };

    void sendStartupNetworkInfo();
}

#endif // NETWORK_INFO_HPP