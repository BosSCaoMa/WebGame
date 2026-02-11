#pragma once

#include "ClientContext.h"
#include "NetConfig.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <thread>
#include <vector>

namespace webgame::net {

class EventLoop;
class EventLoopGroup;
class NetDiag;

class LoginAcceptor {
public:
    LoginAcceptor(const NetConfig& config, EventLoopGroup& loopGroup, NetDiag& diag);
    ~LoginAcceptor();

    bool Start();
    void Stop();

private:
    void AcceptLoop();
    bool GuardFilter(const sockaddr_in& addr) const;
    bool PerformHandshake(int fd, const sockaddr_in& addr, ClientContextPtr& outCtx);
    void EnqueueConnection(int fd, const sockaddr_in& addr);
    void WorkerLoop();
    void ProcessTask(int fd, const sockaddr_in& addr);

    struct PendingConn {
        int fd;
        sockaddr_in addr;
    };

    const NetConfig config_;
    EventLoopGroup& loopGroup_;
    NetDiag& diag_;

    int listenFd_ = -1;
    std::atomic<bool> running_{false};
    std::thread acceptThread_;
    std::atomic<bool> workerRunning_{false};
    std::vector<std::thread> workerThreads_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    std::deque<PendingConn> pendingQueue_;
};

} // namespace webgame::net
