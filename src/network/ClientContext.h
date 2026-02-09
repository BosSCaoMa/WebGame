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

    void SetAttribute(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(attrMutex);
        attributes[key] = value;
    }

    std::string GetAttribute(const std::string& key) const {
        std::lock_guard<std::mutex> lock(attrMutex);
        auto it = attributes.find(key);
        if (it == attributes.end()) {
            return {};
        }
        return it->second;
    }

private:
    mutable std::mutex attrMutex;
    std::unordered_map<std::string, std::string> attributes;
};

using ClientContextPtr = std::shared_ptr<ClientContext>;

} // namespace webgame::net
