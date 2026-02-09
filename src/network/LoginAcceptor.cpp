#include "LoginAcceptor.h"

#include "EventLoop.h"
#include "EventLoopGroup.h"
#include "NetDiag.h"

#include "LogM.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

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

    listenFd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (listenFd_ < 0) {
        LOG_ERROR("Failed to create socket: %s", strerror(errno));
        return false;
    }

    int opt = 1;
    ::setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config_.port);
    if (::inet_pton(AF_INET, config_.host.c_str(), &addr.sin_addr) <= 0) {
        LOG_ERROR("Invalid bind host: %s", config_.host.c_str());
        return false;
    }

    if (::bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        LOG_ERROR("bind failed: %s", strerror(errno));
        return false;
    }

    if (::listen(listenFd_, config_.backlog) < 0) {
        LOG_ERROR("listen failed: %s", strerror(errno));
        return false;
    }

    running_.store(true);
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
}

void LoginAcceptor::AcceptLoop() {
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

        if (!GuardFilter(clientAddr)) {
            diag_.IncRejected();
            ::close(fd);
            continue;
        }

        ClientContextPtr ctx;
        if (!PerformHandshake(fd, clientAddr, ctx)) {
            diag_.IncRejected();
            ::close(fd);
            continue;
        }

        diag_.IncAccepted();
        auto attachFd = fd;
        EventLoop* targetLoop = &loopGroup_.NextLoop();
        targetLoop->QueueInLoop([attachFd, ctx, targetLoop]() {
            if (!targetLoop->AttachConnection(attachFd, ctx)) {
                ::close(attachFd);
            }
        });
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

bool LoginAcceptor::PerformHandshake(int fd, const sockaddr_in& addr, ClientContextPtr& outCtx) {
    timeval tv{};
    tv.tv_sec = 2;
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::string payload;
    payload.reserve(512);
    char buffer[256];
    while (payload.size() < 512) {
        ssize_t n = ::recv(fd, buffer, sizeof(buffer), 0);
        if (n <= 0) {
            return false;
        }
        payload.append(buffer, static_cast<size_t>(n));
        if (payload.find('\n') != std::string::npos) {
            break;
        }
    }
    auto newline = payload.find('\n');
    if (newline == std::string::npos) {
        return false;
    }
    std::string line = Trim(payload.substr(0, newline));
    auto rest = payload.substr(newline + 1);

    if (line.rfind("LOGIN", 0) != 0) {
        return false;
    }
    std::string content = Trim(line.substr(5));
    if (content.empty()) {
        return false;
    }
    auto space = content.find(' ');
    std::string account = Trim(content.substr(0, space));
    std::string token = space == std::string::npos ? std::string{} : Trim(content.substr(space + 1));

    outCtx = std::make_shared<ClientContext>();
    char ipBuf[INET_ADDRSTRLEN] = {0};
    ::inet_ntop(AF_INET, &addr.sin_addr, ipBuf, sizeof(ipBuf));
    outCtx->remoteAddress = ipBuf;
    outCtx->remotePort = ntohs(addr.sin_port);
    outCtx->account = account;
    outCtx->token = token;
    outCtx->pendingInbound = rest;
    outCtx->TransitTo(ClientState::Authed);
    outCtx->TouchHeartbeat();
    LOG_INFO("Login success account=%s ip=%s", account.c_str(), outCtx->remoteAddress.c_str());
    return true;
}

} // namespace webgame::net
