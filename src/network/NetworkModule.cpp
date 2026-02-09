#include "NetworkModule.h"

#include "NetBootstrap.h"

#include <memory>

namespace webgame::net {

namespace {
std::unique_ptr<NetBootstrap> gBootstrap;
}

bool InitializeNetwork() {
    if (gBootstrap && gBootstrap->Start()) {
        return true;
    }
    gBootstrap = std::make_unique<NetBootstrap>();
    return gBootstrap->Start();
}

void ShutdownNetwork() {
    if (gBootstrap) {
        gBootstrap->Stop();
        gBootstrap.reset();
    }
}

} // namespace webgame::net
