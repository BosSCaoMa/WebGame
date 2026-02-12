#include "LoginAcceptor.h"

#include "EventLoop.h"
#include "EventLoopGroup.h"
#include "NetDiag.h"

#include "LogM.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <string>

namespace webgame::net {

LoginAcceptor::LoginAcceptor(const NetConfig& config, EventLoopGroup& loopGroup, NetDiag& diag)
    : config_(config), loopGroup_(loopGroup), diag_(diag) {}

LoginAcceptor::~LoginAcceptor() {
    Stop();
}

bool LoginAcceptor::Start() {
    if (running_.load()) {
        return true;
    }

    // 创建一个 IPv4 的流式套接字（SOCK_STREAM）
    listenFd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (listenFd_ < 0) {
        LOG_ERROR("Failed to create socket: %s", strerror(errno));
        return false;
    }

    // 设置套接字选项，允许地址重用，防止服务器重启后端口被占用
    int opt = 1;
    ::setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 允许在套接字关闭后短时间内重新绑定同一个端口。这对于在服务端崩溃或重启后能够快速重启监听服务很有用

    // 绑定套接字到指定地址和端口
    sockaddr_in addr{};
    addr.sin_family = AF_INET; // IPv4 地址族
    addr.sin_port = htons(config_.port); // 网络字节序（大端序）
    if (::inet_pton(AF_INET, config_.host.c_str(), &addr.sin_addr) <= 0) { // 将配置中的 host 字符串（如 "127.0.0.1"）转换为网络字节序的二进制形式，存入 addr.sin_addr
        LOG_ERROR("Invalid bind host: %s", config_.host.c_str());
        return false;
    }

    if (::bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        LOG_ERROR("bind failed: %s", strerror(errno));
        return false;
    }

    // 开始监听，进入被动模式，等待客户端连接。
    // 指定监听队列的最大长度，通常是一个比较大的整数，表示在调用 accept() 之前，最多能有多少个连接等待
    if (::listen(listenFd_, config_.backlog) < 0) {
        LOG_ERROR("listen failed: %s", strerror(errno));
        return false;
    }

    running_.store(true);
    workerRunning_.store(true);

    int workerCount = std::max(1, config_.loginThreadPoolSize);
    workerThreads_.reserve(workerCount);
    for (int i = 0; i < workerCount; ++i) {
        workerThreads_.emplace_back([this]() { this->WorkerLoop(); });
    }

    acceptThread_ = std::thread([this]() { this->AcceptLoop(); });
    LOG_INFO("LoginAcceptor listening on %s:%d", config_.host.c_str(), config_.port);
    return true;
}

void LoginAcceptor::Stop() {
    if (!running_.exchange(false)) {
        return;
    }
    if (listenFd_ >= 0) {
        ::close(listenFd_);
        listenFd_ = -1;
    }
    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }

    workerRunning_.store(false);
    queueCv_.notify_all();
    for (auto& th : workerThreads_) {
        if (th.joinable()) {
            th.join();
        }
    }
    workerThreads_.clear();
}

void LoginAcceptor::AcceptLoop()
{
    while (running_.load()) {
        sockaddr_in clientAddr{};
        socklen_t len = sizeof(clientAddr);
        int fd = ::accept4(listenFd_, reinterpret_cast<sockaddr*>(&clientAddr), &len, SOCK_CLOEXEC);
        if (fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EMFILE || errno == ENFILE) {
                LOG_ERROR("accept limit reached: %s", strerror(errno));
            }
            continue;
        }

        // 先快速做风控/限流，失败直接丢弃连接
        if (!GuardFilter(clientAddr)) {
            diag_.IncRejected();
            ::close(fd);
            continue;
        }

        EnqueueConnection(fd, clientAddr);
    }
}

bool LoginAcceptor::GuardFilter(const sockaddr_in& addr) const {
    if (!config_.enableGuardFilter) {
        return true;
    }
    // 示例：简单地拒绝回环以外的 0.0.0.0
    if (addr.sin_addr.s_addr == INADDR_ANY) {
        return false;
    }
    return true;
}
void LoginAcceptor::EnqueueConnection(int fd, const sockaddr_in& addr) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        if (pendingQueue_.size() >= config_.maxConnections) {
            LOG_WARN("Too many pending connections, rejecting fd=%d", fd);
            ::close(fd);          // 必须关
            diag_.IncRejected();  // 计数（可选）
            return;
        }
        pendingQueue_.push_back(PendingConn{fd, addr});
    }
    queueCv_.notify_one();
}

void LoginAcceptor::WorkerLoop() {
    while (true) {
        PendingConn task{};
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [this]() {
                return !workerRunning_.load() || !pendingQueue_.empty();
            });
            if (!workerRunning_.load() && pendingQueue_.empty()) {
                break;
            }
            task = pendingQueue_.front();
            pendingQueue_.pop_front();
        }
        ProcessTask(task.fd, task.addr);
    }
}

void LoginAcceptor::ProcessTask(int fd, const sockaddr_in& addr) {
    ClientContextPtr ctx;
    if (!PerformHandshake(addr, ctx)) {
        diag_.IncRejected();
        ::close(fd);
        return;
    }

    diag_.IncAccepted();
    EventLoop* targetLoop = &loopGroup_.NextLoop();
    targetLoop->QueueInLoop([attachFd = fd, ctx, targetLoop]() {
        if (!targetLoop->AttachConnection(attachFd, ctx)) {
            ::close(attachFd);
        }
    });
}

bool LoginAcceptor::PerformHandshake(const sockaddr_in& addr, ClientContextPtr& outCtx) {
    outCtx = std::make_shared<ClientContext>();
    char ipBuf[INET_ADDRSTRLEN] = {0};
    ::inet_ntop(AF_INET, &addr.sin_addr, ipBuf, sizeof(ipBuf));
    outCtx->remoteAddress = ipBuf;
    outCtx->remotePort = ntohs(addr.sin_port);
    outCtx->TouchHeartbeat();
    return true;
}

} // namespace webgame::net
