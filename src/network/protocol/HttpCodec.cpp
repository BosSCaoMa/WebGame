#include "HttpCodec.h"

#include <charconv>
#include <sstream>

namespace webgame::net {

namespace {

constexpr char kCRLF[] = "\r\n";

size_t FindHeaderEnd(const std::string& buffer) {
    return buffer.find("\r\n\r\n");
}

} // namespace

std::string ToLower(std::string_view src) {
    std::string result(src);
    for (auto& ch : result) {
        if (ch >= 'A' && ch <= 'Z') {
            ch = static_cast<char>(ch + 32);
        }
    }
    return result;
}

std::string Trim(std::string_view src) {
    auto begin = src.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos) {
        return {};
    }
    auto end = src.find_last_not_of(" \t\r\n");
    return std::string(src.substr(begin, end - begin + 1));
}

std::string HttpRequest::Header(std::string_view key) const {
    auto lower = ToLower(key);
    auto it = headers.find(lower);
    if (it == headers.end()) {
        return {};
    }
    return it->second;
}

void HttpResponse::SetHeader(std::string key, std::string value) {
    headers[ToLower(key)] = std::move(value);
}

std::string HttpResponse::Serialize() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << ' ' << reason << kCRLF;

    bool hasContentLength = false;
    bool hasConnection = false;
    for (const auto& [key, value] : headers) {
        if (key == "content-length") {
            hasContentLength = true;
        }
        if (key == "connection") {
            hasConnection = true;
        }
        oss << key << ": " << value << kCRLF;
    }

    if (!hasContentLength) {
        oss << "Content-Length: " << body.size() << kCRLF;
    }
    if (!hasConnection) {
        oss << "Connection: keep-alive" << kCRLF;
    }
    oss << kCRLF;
    oss << body;
    return oss.str();
}

/*
        POST /api/v1/resource HTTP/1.1
        Host: example.com
        Content-Type: application/json
        Content-Length: 18

        {"key": "value"}
*/
bool HttpCodec::Decode(std::string& buffer, HttpRequest& outRequest)
{
    size_t headerEnd = FindHeaderEnd(buffer);
    if (headerEnd == std::string::npos) {
        return false;
    }
    const size_t bodyStart = headerEnd + 4;

    std::string headerSection = buffer.substr(0, headerEnd); // 请求头部分 ：请求行 + 头部
    size_t lineEnd = headerSection.find("\r\n");
    if (lineEnd == std::string::npos) {
        throw HttpParseError("invalid request line");
    }
    std::string requestLine = headerSection.substr(0, lineEnd); // 请求行
    std::string_view lineView(requestLine);

    size_t firstSpace = lineView.find(' ');
    size_t secondSpace = std::string_view::npos;
    if (firstSpace != std::string_view::npos) {
        secondSpace = lineView.find(' ', firstSpace + 1);
    }
    if (firstSpace == std::string_view::npos || secondSpace == std::string_view::npos) {
        throw HttpParseError("Malformed request line");
    }

    outRequest.method = std::string(lineView.substr(0, firstSpace));
    outRequest.path = std::string(lineView.substr(firstSpace + 1, secondSpace - firstSpace - 1));
    outRequest.version = std::string(lineView.substr(secondSpace + 1));

    outRequest.headers.clear();
    outRequest.body.clear();

    size_t pos = lineEnd + 2;
    while (pos < headerSection.size()) {
        size_t next = headerSection.find("\r\n", pos);
        size_t len = (next == std::string::npos) ? headerSection.size() - pos : next - pos;
        std::string line = headerSection.substr(pos, len);
        pos = (next == std::string::npos) ? headerSection.size() : next + 2;
        if (line.empty()) {
            continue;
        }
        auto colon = line.find(':');
        if (colon == std::string::npos) {
            throw HttpParseError("Malformed header line");
        }
        std::string key = ToLower(line.substr(0, colon));
        std::string value = Trim(std::string_view(line).substr(colon + 1));
        outRequest.headers[key] = std::move(value);
    }
    
    // 解析消息体
    size_t contentLength = 0;
    auto it = outRequest.headers.find("content-length");
    if (it != outRequest.headers.end() && !it->second.empty()) {
        auto view = std::string_view(it->second);
        auto* begin = view.data();
        auto* end = view.data() + view.size();
        std::from_chars(begin, end, contentLength);
    }

    if (buffer.size() < bodyStart + contentLength) {
        return false;
    }

    outRequest.body.assign(buffer.data() + bodyStart, contentLength);
    buffer.erase(0, bodyStart + contentLength);
    return true;
}

} // namespace webgame::net
