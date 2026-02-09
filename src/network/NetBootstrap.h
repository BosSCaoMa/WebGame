#pragma once

#include "NetConfig.h"
#include "NetDiag.h"
#include "ProtocolDispatcher.h"

#include <memory>

namespace webgame::net {

class EventLoop;
class LoginAcceptor;

class NetBootstrap {
public:
    NetBootstrap();
    ~NetBootstrap();

    bool Start();
    void Stop();

    NetDiag& Diagnostics() { return diag_; }

private:
    void RegisterBuiltInHandlers();

    NetConfig config_;
    NetDiag diag_;
    ProtocolDispatcher dispatcher_;
    std::unique_ptr<EventLoop> eventLoop_;
    std::unique_ptr<LoginAcceptor> acceptor_;
    bool started_ = false;
};

} // namespace webgame::net
