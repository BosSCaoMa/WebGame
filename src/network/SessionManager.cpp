#include "SessionManager.h"

#include "TcpConnection.h"

#include "LogM.h"

namespace webgame::net {

SessionManager& SessionManager::Instance() {
    static SessionManager instance;
    return instance;
}

void SessionManager::Register(const std::string& account, const std::shared_ptr<TcpConnection>& connection) {
    if (account.empty() || !connection) {
        return;
    }

    std::shared_ptr<TcpConnection> previous;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& entry = sessions_[account];
        previous = entry.connection.lock();
        entry.connection = connection;
        entry.fd = connection->Fd();
    }

    if (previous && previous != connection) {
        LOG_INFO("SessionManager replacing existing session account=%s", account.c_str());
        previous->Close();
    }
}

void SessionManager::Unregister(const std::string& account, int fd) {
    if (account.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(account);
    if (it == sessions_.end()) {
        return;
    }

    if (fd != -1 && it->second.fd != fd) {
        return;
    }

    sessions_.erase(it);
}

std::shared_ptr<TcpConnection> SessionManager::Find(const std::string& account) {
    if (account.empty()) {
        return {};
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(account);
    if (it == sessions_.end()) {
        return {};
    }

    auto conn = it->second.connection.lock();
    if (!conn) {
        sessions_.erase(it);
    }
    return conn;
}

size_t SessionManager::Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.size();
}

} // namespace webgame::net
