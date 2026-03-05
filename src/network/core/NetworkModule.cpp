#include "NetworkModule.h"

#include "NetBootstrap.h"
#include "LogM.h"

#include <memory>

namespace webgame::net {

namespace {
std::unique_ptr<NetBootstrap> gBootstrap;
}

bool InitializeNetwork() {
    LOG_INFO("InitializeNetwork enter");
    if (gBootstrap && gBootstrap->Start()) {
        LOG_INFO("InitializeNetwork reuse existing bootstrap");
        return true;
    }
    gBootstrap = std::make_unique<NetBootstrap>();
    bool ok = gBootstrap->Start();
    LOG_INFO("InitializeNetwork exit status=%d", ok ? 1 : 0);
    return ok;
}

void ShutdownNetwork() {
    LOG_INFO("ShutdownNetwork enter");
    if (gBootstrap) {
        gBootstrap->Stop();
        gBootstrap.reset();
    }
    LOG_INFO("ShutdownNetwork exit");
}

} // namespace webgame::net
