#pragma once

#include "MessageCodec.h"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace webgame::net {

class TcpConnection;

class ProtocolDispatcher {
public:
    using Handler = std::function<void(const Packet&, TcpConnection&)>;

    void Register(const std::string& command, Handler handler);
    void Dispatch(const Packet& packet, TcpConnection& connection);

private:
    std::mutex mutex_;
    std::unordered_map<std::string, Handler> handlers_;
};

} // namespace webgame::net
