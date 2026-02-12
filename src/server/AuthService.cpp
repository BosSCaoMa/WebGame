#include "AuthService.h"

#include <map>

namespace {

std::map<std::string, std::string> userPwdMap = {
    {"user1", "password1"},
    {"user2", "password2"},
    {"user3", "password3"},
};

} // namespace

namespace webgame::auth {

bool ValidateCredential(const std::string& account, const std::string& token) {
    auto it = userPwdMap.find(account);
    if (it == userPwdMap.end()) {
        return false;
    }
    return it->second == token;
}

} // namespace webgame::auth