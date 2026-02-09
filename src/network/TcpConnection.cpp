#include "TcpConnection.h"

#include "EventLoop.h"
#include "NetConfig.h"
#include "NetDiag.h"
#include "ProtocolDispatcher.h"

#include "LogM.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

namespace webgame::net {

namespace {
constexpr uint32_t kBaseEvents = EPOLLIN | EPOLLRDHUP | EPOLLERR;
}

TcpConnection::TcpConnection(EventLoop& loop,
                             int fd,
                             ClientContextPtr ctx,
                             ProtocolDispatcher& dispatcher,
                             const NetConfig& config,
                             NetDiag& diag)
    : loop_(loop), fd_(fd), ctx_(std::move(ctx)), dispatcher_(dispatcher), config_(config), diag_(diag) {
    if (!ctx_->pendingInbound.empty()) {
        inboundBuffer_.append(ctx_->pendingInbound);
        ctx_->pendingInbound.clear();
    }
    ctx_->TransitTo(ClientState::Authed);
    ctx_->TouchHeartbeat();
    lastActivity_ = std::chrono::steady_clock::now();
}

TcpConnection::~TcpConnection() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

void TcpConnection::OnEvent(uint32_t events) {
    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        HandlePeerClosed();
        return;
    }
    if (events & EPOLLIN) {
        HandleReadable();
    }
    if (events & EPOLLOUT) {
        HandleWritable();
    }
}

void TcpConnection::Send(const std::string& data) {
    if (closing_.load()) {
        return;
    }
    auto self = shared_from_this();
    loop_.RunInLoop([self, data]() {
        if (self->closing_.load()) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(self->outboundMutex_);
            self->outboundQueue_.push_back(data);
        }
        self->FlushOutbound();
    });
}

void TcpConnection::Close() {
    auto self = shared_from_this();
    loop_.RunInLoop([self]() { self->HandlePeerClosed(); });
}

void TcpConnection::HandleReadable() {
    char buffer[4096];
    while (true) {
        ssize_t n = ::recv(fd_, buffer, sizeof(buffer), 0);
        if (n > 0) {
            inboundBuffer_.append(buffer, static_cast<size_t>(n));
            lastActivity_ = std::chrono::steady_clock::now();
            ctx_->TouchHeartbeat();
        } else if (n == 0) {
            HandlePeerClosed();
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            LOG_ERROR("recv failed fd=%d err=%s", fd_, strerror(errno));
            HandlePeerClosed();
            return;
        }
    }

    std::vector<Packet> packets;
    codec_.Decode(inboundBuffer_, packets);
    for (const auto& pkt : packets) {
        diag_.IncMessagesIn();
        dispatcher_.Dispatch(pkt, *this);
        lastActivity_ = std::chrono::steady_clock::now();
    }
}

void TcpConnection::HandleWritable() {
    FlushOutbound();
}

void TcpConnection::HandlePeerClosed() {
    if (closing_.exchange(true)) {
        return;
    }
    loop_.DetachConnection(fd_);
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    ctx_->TransitTo(ClientState::Closing);
}

void TcpConnection::FlushOutbound() {
    if (closing_.load()) {
        return;
    }

    std::lock_guard<std::mutex> lock(outboundMutex_);
    while (!outboundQueue_.empty()) {
        auto& front = outboundQueue_.front();
        ssize_t n = ::send(fd_, front.data(), front.size(), 0);
        if (n > 0) {
            diag_.IncMessagesOut();
            if (static_cast<size_t>(n) == front.size()) {
                outboundQueue_.pop_front();
            } else {
                front.erase(0, static_cast<size_t>(n));
                loop_.UpdateConnectionEvents(fd_, this, kBaseEvents | EPOLLOUT);
                return;
            }
        } else {
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                loop_.UpdateConnectionEvents(fd_, this, kBaseEvents | EPOLLOUT);
                return;
            }
            LOG_ERROR("send failed fd=%d err=%s", fd_, strerror(errno));
            HandlePeerClosed();
            return;
        }
    }
    loop_.UpdateConnectionEvents(fd_, this, kBaseEvents);
}

} // namespace webgame::net
