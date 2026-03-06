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
    int loginThreadPoolSize = 1; // 处理登录的线程池大小
    int heartbeatTimeoutSec = 120;  // 心跳超时时间，单位秒
    int ioThreadCount = 3; // IO 线程数量，默认为 3，建议根据 CPU 核数调整
    bool useEdgeTrigger = true; // 是否使用边缘触发模式，默认为 true
    bool enableGuardFilter = true; // 是否启用连接过滤器，默认为 true

    static bool TryReadPositiveIntEnv(const char* key, int& outVal) {
        if (const char* env = std::getenv(key); env && env[0] != '\0') {
            char* end = nullptr;
            long value = std::strtol(env, &end, 10);
            if (end != env && *end == '\0' && value > 0 && value <= 1024) {
                outVal = static_cast<int>(value);
                return true;
            }
        }
        return false;
    }

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
        TryReadPositiveIntEnv("WEBGAME_IO_THREADS", cfg.ioThreadCount);
        TryReadPositiveIntEnv("WEBGAME_LOGIN_WORKERS", cfg.loginThreadPoolSize);
        return cfg;
    };
};

} // namespace webgame::net
