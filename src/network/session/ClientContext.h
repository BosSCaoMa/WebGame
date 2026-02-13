/*
    * ClientContext.h
    *
    *  Created on: 2026年2月8日
    * 
    * 功能描述: 客户端连接上下文，包含连接状态、账号信息、心跳时间等
    * 版本: 1.0
*/
#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace webgame::net {

enum class ClientState {
    Handshaking,
    Authed,
    InGame,
    Closing
};

struct ClientContext {
    std::string remoteAddress;
    uint16_t remotePort = 0;
    std::string account;
    std::string token;
    std::string pendingInbound;
    std::atomic<ClientState> state{ClientState::Handshaking};
    std::chrono::steady_clock::time_point lastHeartbeat = std::chrono::steady_clock::now();

    void TouchHeartbeat() {
        lastHeartbeat = std::chrono::steady_clock::now();
    }

    void TransitTo(ClientState next) {
        state.store(next, std::memory_order_relaxed);
    }

    ClientState GetState() const {
        return state.load(std::memory_order_relaxed);
    }

private:
    mutable std::mutex attrMutex;
};

using ClientContextPtr = std::shared_ptr<ClientContext>;

} // namespace webgame::net
