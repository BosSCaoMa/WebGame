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

namespace {

std::string Trim(const std::string& input) {
    auto begin = input.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    auto end = input.find_last_not_of(" \t\r\n");
    return input.substr(begin, end - begin + 1);
}

}

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
struct RcvTimeoutGuard {
    int fd;
    bool restore;
    timeval oldTv;

    RcvTimeoutGuard(const RcvTimeoutGuard&) = delete;
    RcvTimeoutGuard& operator=(const RcvTimeoutGuard&) = delete;

    ~RcvTimeoutGuard() {
        if (restore) {
            ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &oldTv, sizeof(oldTv));
        } else {
            timeval zero{};
            ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &zero, sizeof(zero));
        }
    }
};

bool LoginAcceptor::PerformHandshake(int fd, const sockaddr_in& addr, ClientContextPtr& outCtx) {
    // 1) 保存旧的 RCVTIMEO，并在函数退出时恢复，避免影响后续连接IO（风险C）
    timeval oldTv{};
    socklen_t oldLen = sizeof(oldTv);
    bool haveOld = (::getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &oldTv, &oldLen) == 0);

    RcvTimeoutGuard guard{fd, haveOld, oldTv};

    timeval tv{};
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) {
        return false;
    }

    // 2) 读取首行：最多512字节，遇到'\n'停止
    std::string payload;
    payload.reserve(512);

    char buffer[256];
    while (payload.size() < 512) {
        ssize_t n = ::recv(fd, buffer, sizeof(buffer), 0);
        if (n > 0) {
            size_t oldSize = payload.size();
            payload.append(buffer, static_cast<size_t>(n));
            // 只在新追加区域附近找 '\n'（可选的小优化）
            if (payload.find('\n', oldSize == 0 ? 0 : oldSize - 1) != std::string::npos) {
                break;
            }
            continue;
        }

        if (n == 0) { // 对端正常关闭
            return false;
        }

        // n < 0: 区分错误原因（风险D）
        if (errno == EINTR) {
            continue; // 信号打断，重试
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) { // 超时
            return false;
        }
        return false; // 其他错误，直接返回失败
    }

    auto newline = payload.find('\n');
    if (newline == std::string::npos) {
        return false;
    }

    std::string line = Trim(payload.substr(0, newline));
    std::string rest = payload.substr(newline + 1);

    if (line.rfind("LOGIN", 0) != 0) {
        return false;
    }

    std::string content = Trim(line.substr(5));
    if (content.empty()) {
        return false;
    }

    auto space = content.find(' ');
    std::string account = Trim(content.substr(0, space));
    std::string token = (space == std::string::npos) ? std::string{} : Trim(content.substr(space + 1));

    outCtx = std::make_shared<ClientContext>();
    char ipBuf[INET_ADDRSTRLEN] = {0};
    ::inet_ntop(AF_INET, &addr.sin_addr, ipBuf, sizeof(ipBuf));
    outCtx->remoteAddress = ipBuf;
    outCtx->remotePort = ntohs(addr.sin_port);
    outCtx->account = account;
    outCtx->token = token;
    outCtx->pendingInbound = std::move(rest);
    outCtx->TransitTo(ClientState::Authed);
    outCtx->TouchHeartbeat();

    LOG_INFO("Login success account=%s ip=%s", account.c_str(), outCtx->remoteAddress.c_str());
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
    if (!PerformHandshake(fd, addr, ctx)) {
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

} // namespace webgame::net
