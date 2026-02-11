#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace webgame::net {

class TcpConnection;

class SessionManager {
public:
    static SessionManager& Instance();

    void Register(const std::string& account, const std::shared_ptr<TcpConnection>& connection);
    void Unregister(const std::string& account, int fd);
    std::shared_ptr<TcpConnection> Find(const std::string& account);
    size_t Size() const;

private:
    SessionManager() = default;

    struct SessionEntry {
        std::weak_ptr<TcpConnection> connection;
        int fd = -1;
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, SessionEntry> sessions_;
};

} // namespace webgame::net
