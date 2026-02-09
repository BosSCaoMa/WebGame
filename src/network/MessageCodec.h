#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace webgame::net {

struct Packet {
    std::string command;
    std::string payload;
};

class MessageCodec {
public:
    MessageCodec() = default;

    // 将 buffer 中的消息按照 "CMD payload\n" 解析成 Packet 列表。
    // 解析成功的部分会从 buffer 中移除。
    void Decode(std::string& buffer, std::vector<Packet>& outPackets);

    static std::string Encode(std::string_view command, std::string_view payload);
};

} // namespace webgame::net
