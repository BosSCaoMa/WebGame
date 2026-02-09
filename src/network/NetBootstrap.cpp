#include "NetBootstrap.h"

#include "EventLoop.h"
#include "LoginAcceptor.h"
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

    eventLoop_ = std::make_unique<EventLoop>(config_, dispatcher_, diag_);
    if (!eventLoop_->Start()) {
        return false;
    }

    acceptor_ = std::make_unique<LoginAcceptor>(config_, *eventLoop_, diag_);
    if (!acceptor_->Start()) {
        eventLoop_->Stop();
        eventLoop_.reset();
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
    if (eventLoop_) {
        eventLoop_->Stop();
    }
    acceptor_.reset();
    eventLoop_.reset();
    started_ = false;
    LOG_INFO("NetBootstrap stopped");
}

void NetBootstrap::RegisterBuiltInHandlers() {
    dispatcher_.Register("PING", [](const Packet&, TcpConnection& conn) {
        conn.Send(MessageCodec::Encode("PONG", {}));
    });

    dispatcher_.Register("ECHO", [](const Packet& pkt, TcpConnection& conn) {
        conn.Send(MessageCodec::Encode("ECHO", pkt.payload));
    });

    dispatcher_.Register("QUIT", [](const Packet&, TcpConnection& conn) {
        conn.Send(MessageCodec::Encode("BYE", {}));
        conn.Close();
    });
}

} // namespace webgame::net
