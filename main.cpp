#include "network/NetworkModule.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

namespace {
std::atomic<bool> gRunning{true};

void SignalHandler(int)
{
    gRunning.store(false);
}
}

bool LoadDataConfig()
{

    return true;
}

bool LoadNetworkModule()
{
    return webgame::net::InitializeNetwork();
}

int main()
{
    std::cout << "服务器启动" << std::endl;

    std::signal(SIGINT, SignalHandler);

    if (!LoadDataConfig()) {
        std::cout << "加载数据配置失败" << std::endl;
        return -1;
    }

    if (!LoadNetworkModule()) {
        std::cout << "加载网络模块失败" << std::endl;
        return -1;
    }

    std::cout << "服务器运行中，按 Ctrl+C 退出" << std::endl;
    while (gRunning.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "正在关闭服务器..." << std::endl;

    webgame::net::ShutdownNetwork();
    return 0;
}