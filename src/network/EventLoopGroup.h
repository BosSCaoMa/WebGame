#pragma once

#include "NetConfig.h"

#include <atomic>
#include <memory>
#include <vector>

namespace webgame::net {

class EventLoop;
class ProtocolDispatcher;
class NetDiag;

class EventLoopGroup {
public:
    EventLoopGroup(const NetConfig& config, ProtocolDispatcher& dispatcher, NetDiag& diag);
    ~EventLoopGroup();

    bool Start();
    void Stop();

    EventLoop& NextLoop();
    size_t Size() const { return loops_.size(); }

private:
    const NetConfig config_;
    ProtocolDispatcher& dispatcher_;
    NetDiag& diag_;
    std::vector<std::unique_ptr<EventLoop>> loops_;
    std::atomic<size_t> next_{0};
    bool started_ = false;
};

} // namespace webgame::net
