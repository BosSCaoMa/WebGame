#pragma once

#include <string>

namespace webgame::auth {

bool ValidateCredential(const std::string& account, const std::string& token);

} // namespace webgame::auth