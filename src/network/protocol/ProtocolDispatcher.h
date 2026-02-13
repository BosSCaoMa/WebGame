#pragma once

#include "HttpCodec.h"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace webgame::net {

class TcpConnection;

class ProtocolDispatcher {
public:
    using Handler = std::function<void(const HttpRequest&, TcpConnection&)>;

    void Register(const std::string& method, const std::string& path, Handler handler);
    void Dispatch(const HttpRequest& request, TcpConnection& connection);

private:
    std::mutex mutex_;
    std::unordered_map<std::string, Handler> handlers_;
};

} // namespace webgame::net
