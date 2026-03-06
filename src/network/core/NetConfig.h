#pragma once

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>

namespace webgame::net {

struct NetConfig {
    std::string host = "127.0.0.1";
    uint16_t port = 18888;
    int backlog = 128; // 监听队列长度
    int maxConnections = 4096; // 最大连接数
    int loginThreadPoolSize = 2; // 处理登录的线程池大小
    int heartbeatTimeoutSec = 120;  // 心跳超时时间，单位秒
    int ioThreadCount = 2; // IO 线程数量，默认为 1，建议根据 CPU 核数调整
    bool useEdgeTrigger = true; // 是否使用边缘触发模式，默认为 true
    bool enableGuardFilter = true; // 是否启用连接过滤器，默认为 true

    static NetConfig LoadDefault() {
        NetConfig cfg;
        if (const char* envHost = std::getenv("WEBGAME_HOST"); envHost && envHost[0] != '\0') {
            cfg.host = envHost;
        }
        if (const char* envPort = std::getenv("WEBGAME_PORT"); envPort && envPort[0] != '\0') {
            char* end = nullptr;
            long port = std::strtol(envPort, &end, 10);
            if (end != envPort && *end == '\0' && port > 0 && port <= 65535) {
                cfg.port = static_cast<uint16_t>(port);
            }
        }
        return cfg;
    };
};

} // namespace webgame::net
