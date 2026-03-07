#include "ProtocolDispatcher.h"

#include "TcpConnection.h"

#include "LogM.h"

#include <cstdlib>
#include <cstring>

namespace webgame::net {

namespace {

std::string MakeKey(const std::string& method, const std::string& path) {
    return method + " " + path;
}

bool VerboseConnLogEnabled() {
    static bool enabled = []() {
        const char* value = std::getenv("WEBGAME_VERBOSE_CONN_LOG");
        return value != nullptr && std::strcmp(value, "1") == 0;
    }();
    return enabled;
}

}

void ProtocolDispatcher::Register(const std::string& method, const std::string& path, Handler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_[MakeKey(method, path)] = std::move(handler);
}

void ProtocolDispatcher::Dispatch(const HttpRequest& request, TcpConnection& connection) {
    if (VerboseConnLogEnabled()) {
        LOG_INFO("Dispatch request method=%s path=%s fd=%d", request.method.c_str(), request.path.c_str(), connection.Fd());
    }
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
