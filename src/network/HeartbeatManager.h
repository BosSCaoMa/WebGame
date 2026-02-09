#pragma once

#include <chrono>
#include <functional>

namespace webgame::net {

class HeartbeatManager {
public:
    using Clock = std::chrono::steady_clock;

    explicit HeartbeatManager(int timeoutSeconds)
        : timeout_(std::chrono::seconds(timeoutSeconds)) {}

    template <typename Fn>
    void Scan(Fn&& predicate) {
        predicate(timeout_);
    }

private:
    std::chrono::steady_clock::duration timeout_;
};

} // namespace webgame::net
