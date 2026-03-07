#include "NetworkModule.h"
#include "CharacterConfig.h"
#include "SkillConfig.h"
#include "LogM.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

namespace {
std::atomic<bool> gRunning{true};

void SignalHandler(int)
{
    gRunning.store(false);
}
}

bool LoadDataConfig()
{
    namespace fs = std::filesystem;

    const std::vector<fs::path> roots = {
        fs::current_path(),
        fs::current_path() / "..",
        fs::current_path() / "../..",
    };

    fs::path skillPath;
    fs::path characterPath;

    for (const auto& root : roots) {
        fs::path s = root / "src/data/config/skills.json";
        fs::path c = root / "src/data/config/characters.json";
        if (fs::exists(s) && fs::exists(c)) {
            skillPath = s;
            characterPath = c;
            break;
        }
    }

    if (skillPath.empty() || characterPath.empty()) {
        std::cout << "未找到技能或武将配置文件" << std::endl;
        return false;
    }

    if (!SkillConfig::instance().loadFromFile(skillPath.string())) {
        std::cout << "加载技能配置失败: " << skillPath << std::endl;
        return false;
    }

    if (!CharacterConfig::instance().loadFromFile(characterPath.string())) {
        std::cout << "加载武将配置失败: " << characterPath << std::endl;
        return false;
    }

    std::cout << "配置加载完成: " << skillPath << " , " << characterPath << std::endl;

    return true;
}

bool LoadNetworkModule()
{
    return webgame::net::InitializeNetwork();
}

int main()
{
    LogM::getInstance().setLevel(LOGM_ERROR);
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