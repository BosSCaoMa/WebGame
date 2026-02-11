#include "EventLoopGroup.h"

#include "EventLoop.h"
#include "NetDiag.h"
#include "ProtocolDispatcher.h"

#include "LogM.h"

#include <algorithm>
#include <stdexcept>

namespace webgame::net {

EventLoopGroup::EventLoopGroup(const NetConfig& config, ProtocolDispatcher& dispatcher, NetDiag& diag)
    : config_(config), dispatcher_(dispatcher), diag_(diag) {}

EventLoopGroup::~EventLoopGroup() {
    Stop();
}

bool EventLoopGroup::Start() {
    if (started_) {
        return true;
    }
    // 至少保证一个事件循环，避免配置异常导致完全无法监听
    int loopCount = std::max(1, config_.ioThreadCount);
    loops_.reserve(loopCount);
    for (int i = 0; i < loopCount; ++i) {
        auto loop = std::make_unique<EventLoop>(config_, dispatcher_, diag_);
        if (!loop->Start()) {
            LOG_ERROR("EventLoop %d failed to start", i);
            Stop();
            return false;
        }
        loops_.push_back(std::move(loop));
    }
    started_ = true;
    LOG_INFO("EventLoopGroup started with %zu loops", loops_.size());
    return true;
}

void EventLoopGroup::Stop() {
    if (!started_) {
        return;
    }
    for (auto& loop : loops_) {
        if (loop) {
            loop->Stop();
        }
    }
    loops_.clear();
    started_ = false;
    next_.store(0, std::memory_order_relaxed);
    LOG_INFO("EventLoopGroup stopped");
}

EventLoop& EventLoopGroup::NextLoop() {
    if (loops_.empty()) {
        throw std::runtime_error("EventLoopGroup has no loops");
    }
    // 原子 round-robin，避免加锁即可在多线程接入层下分发连接
    size_t idx = next_.fetch_add(1, std::memory_order_relaxed);
    return *loops_[idx % loops_.size()];
}

} // namespace webgame::net
