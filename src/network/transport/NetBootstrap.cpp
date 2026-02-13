#include "NetBootstrap.h"

#include "AuthService.h"
#include "EventLoop.h"
#include "EventLoopGroup.h"
#include "LoginAcceptor.h"
#include "SessionManager.h"
#include "TcpConnection.h"

#include "LogM.h"

namespace webgame::net {

NetBootstrap::NetBootstrap() {
    config_ = NetConfig::LoadDefault();
    RegisterBuiltInHandlers();
}

NetBootstrap::~NetBootstrap() {
    Stop();
}

bool NetBootstrap::Start() {
    if (started_) {
        return true;
    }

    // 先拉起 IO 线程池，确保有事件循环可以接管新连接
    loopGroup_ = std::make_unique<EventLoopGroup>(config_, dispatcher_, diag_);
    if (!loopGroup_->Start()) {
        loopGroup_.reset();
        return false;
    }

    // 接入层最后启动，这样认证成功的 fd 能立即分发到可用的 EventLoop
    acceptor_ = std::make_unique<LoginAcceptor>(config_, *loopGroup_, diag_);
    if (!acceptor_->Start()) {
        loopGroup_->Stop();
        loopGroup_.reset();
        return false;
    }

    started_ = true;
    LOG_INFO("NetBootstrap started");
    return true;
}

void NetBootstrap::Stop() {
    if (!started_) {
        return;
    }
    if (acceptor_) {
        acceptor_->Stop();
    }
    if (loopGroup_) {
        loopGroup_->Stop();
    }
    acceptor_.reset();
    loopGroup_.reset();
    started_ = false;
    LOG_INFO("NetBootstrap stopped");
}

void NetBootstrap::RegisterBuiltInHandlers() {
    dispatcher_.Register("GET", "/ping", [](const HttpRequest&, TcpConnection& conn) {
        HttpResponse resp;
        resp.SetHeader("Content-Type", "application/json");
        resp.body = "{\"status\":\"pong\"}";
        conn.SendHttpResponse(resp);
    });

    dispatcher_.Register("POST", "/echo", [](const HttpRequest& req, TcpConnection& conn) {
        HttpResponse resp;
        resp.SetHeader("Content-Type", "text/plain; charset=utf-8");
        resp.body = req.body;
        conn.SendHttpResponse(resp);
    });

    dispatcher_.Register("POST", "/login", [](const HttpRequest& req, TcpConnection& conn) {
        HttpResponse resp;
        resp.SetHeader("Content-Type", "application/json");
        auto account = req.Header("x-account");
        auto token = req.Header("x-token");

        if (account.empty() || token.empty()) {
            resp.statusCode = 400;
            resp.reason = "Bad Request";
            resp.body = "{\"error\":\"missing credentials\"}";
            conn.SendHttpResponse(resp);
            return;
        }

        if (!auth::ValidateCredential(account, token)) {
            resp.statusCode = 401;
            resp.reason = "Unauthorized";
            resp.body = "{\"error\":\"invalid credential\"}";
            conn.SendHttpResponse(resp);
            return;
        }

        auto ctx = conn.Context();
        ctx->account = account;
        ctx->TransitTo(ClientState::Authed);
        ctx->TouchHeartbeat();
        SessionManager::Instance().Register(account, conn.shared_from_this());

        resp.body = "{\"status\":\"ok\"}";
        conn.SendHttpResponse(resp);
    });
}

} // namespace webgame::net
