#pragma once

#include <atomic>

namespace webgame::net {

class NetDiag {
public:
    void IncAccepted() { acceptedConnections_.fetch_add(1, std::memory_order_relaxed); }
    void IncRejected() { rejectedConnections_.fetch_add(1, std::memory_order_relaxed); }
    void IncMessagesIn() { messagesIn_.fetch_add(1, std::memory_order_relaxed); }
    void IncMessagesOut() { messagesOut_.fetch_add(1, std::memory_order_relaxed); }

    int Accepted() const { return acceptedConnections_.load(std::memory_order_relaxed); }
    int Rejected() const { return rejectedConnections_.load(std::memory_order_relaxed); }
    int MessagesIn() const { return messagesIn_.load(std::memory_order_relaxed); }
    int MessagesOut() const { return messagesOut_.load(std::memory_order_relaxed); }

private:
    std::atomic<int> acceptedConnections_{0};
    std::atomic<int> rejectedConnections_{0};
    std::atomic<int> messagesIn_{0};
    std::atomic<int> messagesOut_{0};
};

} // namespace webgame::net
