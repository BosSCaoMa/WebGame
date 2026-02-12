#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace webgame::net {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version = "HTTP/1.1";
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string Header(std::string_view key) const;
};

struct HttpResponse {
    int statusCode = 200;
    std::string reason = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    void SetHeader(std::string key, std::string value);
    std::string Serialize() const;
};

class HttpCodec {
public:
    bool Decode(std::string& buffer, HttpRequest& outRequest);
};

std::string ToLower(std::string_view src);
std::string Trim(std::string_view src);

class HttpParseError : public std::runtime_error {
public:
    explicit HttpParseError(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace webgame::net
