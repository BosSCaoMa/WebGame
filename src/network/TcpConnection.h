#pragma once

#include "ClientContext.h"
#include "MessageCodec.h"

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <sys/epoll.h>

namespace webgame::net {

class EventLoop;
class ProtocolDispatcher;
class NetDiag;
struct NetConfig;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop& loop,
                  int fd,
                  ClientContextPtr ctx,
                  ProtocolDispatcher& dispatcher,
                  const NetConfig& config,
                  NetDiag& diag);
    ~TcpConnection();

    int Fd() const { return fd_; }
    ClientContextPtr Context() const { return ctx_; }

    void OnEvent(uint32_t events);
    void Send(const std::string& data);
    void Close();
    std::chrono::steady_clock::time_point LastActivity() const { return lastActivity_; }

private:
    void HandleReadable();
    void HandleWritable();
    void HandlePeerClosed();
    void FlushOutbound();
    void ScheduleFlush();

    EventLoop& loop_;
    int fd_;
    ClientContextPtr ctx_;
    ProtocolDispatcher& dispatcher_;
    const NetConfig& config_;
    NetDiag& diag_;

    std::string inboundBuffer_;
    std::deque<std::string> outboundQueue_;
    std::mutex outboundMutex_;
    MessageCodec codec_;
    std::atomic<bool> closing_{false};
    std::chrono::steady_clock::time_point lastActivity_ = std::chrono::steady_clock::now();
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace webgame::net
