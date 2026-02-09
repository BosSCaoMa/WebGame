#pragma once

#include "ClientContext.h"
#include "NetConfig.h"

#include <atomic>
#include <memory>
#include <netinet/in.h>
#include <thread>

namespace webgame::net {

class EventLoop;
class NetDiag;

class LoginAcceptor {
public:
    LoginAcceptor(const NetConfig& config, EventLoop& loop, NetDiag& diag);
    ~LoginAcceptor();

    bool Start();
    void Stop();

private:
    void AcceptLoop();
    bool GuardFilter(const sockaddr_in& addr) const;
    bool PerformHandshake(int fd, const sockaddr_in& addr, ClientContextPtr& outCtx);

    const NetConfig config_;
    EventLoop& loop_;
    NetDiag& diag_;

    int listenFd_ = -1;
    std::atomic<bool> running_{false};
    std::thread acceptThread_;
};

} // namespace webgame::net
