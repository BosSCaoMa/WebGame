#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unordered_map>
#include <unistd.h>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::string token;
};

inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool parse_http_request(const std::string& raw, HttpRequest& req);
std::unordered_map<std::string, std::string> parse_url_encoded(const std::string& data);
void handle_client(int client_fd);
int ProcWebConnect(int port);
