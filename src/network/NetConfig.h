#pragma once

#include <cstdint>
#include <string>

namespace webgame::net {

struct NetConfig {
    std::string host = "0.0.0.0";
    uint16_t port = 18888;
    int backlog = 128;
    int maxConnections = 4096;
    int loginThreadPoolSize = 2;
    int heartbeatTimeoutSec = 120;
    bool useEdgeTrigger = true;
    bool enableGuardFilter = true;

    static NetConfig LoadDefault();
};

} // namespace webgame::net
