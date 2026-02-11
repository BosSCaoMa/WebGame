#pragma once

#include <string>

namespace webgame::auth {

struct LoginPayload {
	std::string account;
	std::string token;
	std::string pendingInbound;
};

bool ValidateLoginPayload(const std::string& payload, LoginPayload& outPayload);

} // namespace webgame::auth