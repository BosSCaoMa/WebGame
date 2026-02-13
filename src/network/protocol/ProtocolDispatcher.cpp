#include "ProtocolDispatcher.h"

#include "TcpConnection.h"

#include "LogM.h"

namespace webgame::net {

namespace {

std::string MakeKey(const std::string& method, const std::string& path) {
    return method + " " + path;
}

}

void ProtocolDispatcher::Register(const std::string& method, const std::string& path, Handler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_[MakeKey(method, path)] = std::move(handler);
}

void ProtocolDispatcher::Dispatch(const HttpRequest& request, TcpConnection& connection) {
    Handler handler;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(MakeKey(request.method, request.path));
        if (it != handlers_.end()) {
            handler = it->second;
        }
    }

    if (handler) {
        handler(request, connection);
    } else {
        LOG_WARN("Unknown route: %s %s", request.method.c_str(), request.path.c_str());
        HttpResponse resp;
        resp.statusCode = 404;
        resp.reason = "Not Found";
        resp.SetHeader("Content-Type", "text/plain; charset=utf-8");
        resp.body = "Unknown route";
        connection.SendHttpResponse(resp);
    }
}

} // namespace webgame::net
