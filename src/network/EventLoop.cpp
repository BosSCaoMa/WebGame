#include "EventLoop.h"

#include "NetDiag.h"
#include "ProtocolDispatcher.h"
#include "TcpConnection.h"

#include "LogM.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

namespace {

int SetNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

}

namespace webgame::net {

EventLoop::EventLoop(const NetConfig& config, ProtocolDispatcher& dispatcher, NetDiag& diag)
    : config_(config), dispatcher_(dispatcher), diag_(diag), heartbeat_(config.heartbeatTimeoutSec) {}

EventLoop::~EventLoop() {
    Stop();
}

bool EventLoop::Start() {
    if (running_.load()) {
        return true;
    }
    epollFd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (epollFd_ < 0) {
        LOG_ERROR("Failed to create epoll: %s", strerror(errno));
        return false;
    }
    wakeupFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeupFd_ < 0) {
        LOG_ERROR("Failed to create eventfd: %s", strerror(errno));
        return false;
    }
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;
    if (::epoll_ctl(epollFd_, EPOLL_CTL_ADD, wakeupFd_, &ev) < 0) {
        LOG_ERROR("Failed to add wakeup fd: %s", strerror(errno));
        return false;
    }

    running_.store(true);
    loopThread_ = std::thread([this]() { this->Loop(); });
    loopThreadId_ = loopThread_.get_id();
    LOG_INFO("EventLoop started");
    return true;
}

void EventLoop::Stop() {
    if (!running_.exchange(false)) {
        return;
    }
    Wakeup();
    if (loopThread_.joinable()) {
        loopThread_.join();
    }
    if (wakeupFd_ >= 0) {
        ::close(wakeupFd_);
        wakeupFd_ = -1;
    }
    if (epollFd_ >= 0) {
        ::close(epollFd_);
        epollFd_ = -1;
    }
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    connections_.clear();
    LOG_INFO("EventLoop stopped");
}

void EventLoop::RunInLoop(Functor cb) {
    if (IsInLoopThread()) {
        cb();
    } else {
        QueueInLoop(std::move(cb));
    }
}

void EventLoop::QueueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    Wakeup();
}

bool EventLoop::AttachConnection(int fd, ClientContextPtr ctx) {
    if (SetNonBlocking(fd) != 0) {
        LOG_ERROR("Failed to set non-blocking for fd %d", fd);
        return false;
    }

    auto connection = std::make_shared<TcpConnection>(*this, fd, std::move(ctx), dispatcher_, config_, diag_);

    epoll_event ev{};
    ev.data.ptr = connection.get();
    ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
    if (config_.useEdgeTrigger) {
        ev.events |= EPOLLET;
    }

    if (::epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        LOG_ERROR("epoll_ctl add failed: %s", strerror(errno));
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        connections_[fd] = std::move(connection);
    }

    return true;
}

void EventLoop::DetachConnection(int fd) {
    ::epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    connections_.erase(fd);
}

void EventLoop::UpdateConnectionEvents(int fd, TcpConnection* conn, uint32_t events) {
    epoll_event ev{};
    ev.data.ptr = conn;
    ev.events = events;
    if (config_.useEdgeTrigger) {
        ev.events |= EPOLLET;
    }
    if (::epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        LOG_ERROR("epoll_ctl mod failed: %s", strerror(errno));
    }
}

bool EventLoop::IsInLoopThread() const {
    return loopThreadId_ == std::this_thread::get_id();
}

void EventLoop::Loop() {
    constexpr int kMaxEvents = 32;
    epoll_event events[kMaxEvents];

    while (running_.load()) {
        int timeoutMs = 1000;
        int n = ::epoll_wait(epollFd_, events, kMaxEvents, timeoutMs);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            LOG_ERROR("epoll_wait error: %s", strerror(errno));
            break;
        }

        for (int i = 0; i < n; ++i) {
            void* ptr = events[i].data.ptr;
            if (ptr == nullptr) {
                HandleWakeup();
                continue;
            }
            auto* connection = static_cast<TcpConnection*>(ptr);
            connection->OnEvent(events[i].events);
        }

        ProcessPendingTasks();
        ScanHeartbeats();
    }
}

void EventLoop::Wakeup() {
    if (wakeupFd_ < 0) {
        return;
    }
    uint64_t one = 1;
    ::write(wakeupFd_, &one, sizeof(one));
}

void EventLoop::HandleWakeup() {
    uint64_t one = 0;
    ::read(wakeupFd_, &one, sizeof(one));
}

void EventLoop::ProcessPendingTasks() {
    std::vector<Functor> functors;
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        functors.swap(pendingFunctors_);
    }
    for (auto& fn : functors) {
        fn();
    }
}

void EventLoop::ScanHeartbeats() {
    auto now = std::chrono::steady_clock::now();
    std::vector<std::shared_ptr<TcpConnection>> snapshot;
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        snapshot.reserve(connections_.size());
        for (auto& pair : connections_) {
            snapshot.push_back(pair.second);
        }
    }
    for (auto& conn : snapshot) {
        auto idle = now - conn->LastActivity();
        if (idle > std::chrono::seconds(config_.heartbeatTimeoutSec)) {
            LOG_WARN("Connection idle timeout fd=%d", conn->Fd());
            conn->Close();
        }
    }
}

} // namespace webgame::net
