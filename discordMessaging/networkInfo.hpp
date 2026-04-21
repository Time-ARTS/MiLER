#ifndef NETWORK_INFO_HPP
#define NETWORK_INFO_HPP

#pragma once
#include <string>

namespace NetworkInfo
{
    std::string getWlanIP();

    void sendStartupNetworkInfo();
}

#endif // NETWORK_INFO_HPP