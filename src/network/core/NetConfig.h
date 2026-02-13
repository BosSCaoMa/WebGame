#pragma once

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
        return cfg;
    };
};

} // namespace webgame::net
