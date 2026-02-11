#include "AuthService.h"

#include <map>

namespace {

std::string Trim(const std::string& input) {
    auto begin = input.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    auto end = input.find_last_not_of(" \t\r\n");
    return input.substr(begin, end - begin + 1);
}

std::map<std::string, std::string> userPwdMap = {
    {"user1", "password1"},
    {"user2", "password2"},
    {"user3", "password3"},
};

bool CheckCredential(const std::string& account, const std::string& token) {
    auto it = userPwdMap.find(account);
    if (it == userPwdMap.end()) {
        return false;
    }
    return it->second == token;
}

} // namespace

namespace webgame::auth {

bool ValidateLoginPayload(const std::string& payload, LoginPayload& outPayload) {
    auto newline = payload.find('\n');
    if (newline == std::string::npos) {
        return false;
    }

    std::string line = Trim(payload.substr(0, newline));
    outPayload.pendingInbound = payload.substr(newline + 1);

    if (line.rfind("LOGIN", 0) != 0) {
        return false;
    }

    std::string content = Trim(line.substr(5));
    if (content.empty()) {
        return false;
    }

    auto space = content.find(' ');
    outPayload.account = Trim(content.substr(0, space));
    outPayload.token = (space == std::string::npos) ? std::string{} : Trim(content.substr(space + 1));

    if (outPayload.account.empty()) {
        return false;
    }

    return CheckCredential(outPayload.account, outPayload.token);
}

} // namespace webgame::auth