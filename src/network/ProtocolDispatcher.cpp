#include "ProtocolDispatcher.h"

#include "TcpConnection.h"

#include "LogM.h"

namespace webgame::net {

void ProtocolDispatcher::Register(const std::string& command, Handler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_[command] = std::move(handler);
}

void ProtocolDispatcher::Dispatch(const Packet& packet, TcpConnection& connection) {
    Handler handler;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(packet.command);
        if (it != handlers_.end()) {
            handler = it->second;
        }
    }

    if (handler) {
        handler(packet, connection);
    } else {
        LOG_WARN("Unknown command: %s", packet.command.c_str());
        connection.Send(MessageCodec::Encode("ERR", "unknown command"));
    }
}

} // namespace webgame::net
