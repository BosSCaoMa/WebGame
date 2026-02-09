#include "MessageCodec.h"

#include <algorithm>
#include <cctype>

namespace {

std::string Trim(std::string_view view) {
    auto begin = view.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos) {
        return {};
    }
    auto end = view.find_last_not_of(" \t\r\n");
    return std::string(view.substr(begin, end - begin + 1));
}

}

namespace webgame::net {

void MessageCodec::Decode(std::string& buffer, std::vector<Packet>& outPackets) {
    size_t newlinePos = std::string::npos;
    while ((newlinePos = buffer.find('\n')) != std::string::npos) {
        std::string line = buffer.substr(0, newlinePos);
        buffer.erase(0, newlinePos + 1);
        line = Trim(line);
        if (line.empty()) {
            continue;
        }
        auto spacePos = line.find(' ');
        Packet pkt;
        if (spacePos == std::string::npos) {
            pkt.command = line;
        } else {
            pkt.command = line.substr(0, spacePos);
            pkt.payload = Trim(line.substr(spacePos + 1));
        }
        outPackets.emplace_back(std::move(pkt));
    }
}

std::string MessageCodec::Encode(std::string_view command, std::string_view payload) {
    std::string line;
    line.reserve(command.size() + payload.size() + 2);
    line.append(command);
    if (!payload.empty()) {
        line.push_back(' ');
        line.append(payload);
    }
    line.push_back('\n');
    return line;
}

} // namespace webgame::net
