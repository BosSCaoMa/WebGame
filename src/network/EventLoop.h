#pragma once

#include "ClientContext.h"
#include "HeartbeatManager.h"
#include "NetConfig.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace webgame::net {

class ProtocolDispatcher;
class TcpConnection;
class NetDiag;

class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop(const NetConfig& config, ProtocolDispatcher& dispatcher, NetDiag& diag);
    ~EventLoop();

    bool Start();
    void Stop();
    void RunInLoop(Functor cb);
    void QueueInLoop(Functor cb);
    bool AttachConnection(int fd, ClientContextPtr ctx);
    void DetachConnection(int fd);
    void UpdateConnectionEvents(int fd, TcpConnection* conn, uint32_t events);
    bool IsInLoopThread() const;

    ProtocolDispatcher& Dispatcher() { return dispatcher_; }

private:
    void Loop();
    void Wakeup();
    void HandleWakeup();
    void ProcessPendingTasks();
    void ScanHeartbeats();

    const NetConfig config_;
    ProtocolDispatcher& dispatcher_;
    NetDiag& diag_;
    HeartbeatManager heartbeat_;

    int epollFd_ = -1;
    int wakeupFd_ = -1;
    std::thread loopThread_;
    std::atomic<bool> running_{false};
    std::thread::id loopThreadId_;

    std::mutex pendingMutex_;
    std::vector<Functor> pendingFunctors_;

    std::mutex connectionsMutex_;
    std::unordered_map<int, std::shared_ptr<TcpConnection>> connections_;
};

} // namespace webgame::net
