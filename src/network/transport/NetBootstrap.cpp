#include "NetBootstrap.h"

#include "AuthService.h"
#include "BattleManager.h"
#include "CharacterConfig.h"
#include "EventLoop.h"
#include "EventLoopGroup.h"
#include "LoginAcceptor.h"
#include "Player.h"
#include "SessionManager.h"
#include "SkillConfig.h"
#include "TcpConnection.h"

#include "LogM.h"
#include "json.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>

namespace webgame::net {

namespace {
uint64_t NowMs();

std::atomic<uint64_t> gBattleIdSeq{1};
const uint64_t gServerStartMs = NowMs();

struct RouteMetric {
    uint64_t totalRequests = 0;
    uint64_t errorRequests = 0;
    uint64_t totalLatencyMs = 0;
    uint64_t maxLatencyMs = 0;
};

std::mutex gRouteMetricMutex;
std::unordered_map<std::string, RouteMetric> gRouteMetrics;

void RecordRouteMetric(const std::string& route, uint64_t latencyMs, int statusCode) {
    std::lock_guard<std::mutex> lock(gRouteMetricMutex);
    auto& metric = gRouteMetrics[route];
    metric.totalRequests += 1;
    if (statusCode >= 400) {
        metric.errorRequests += 1;
    }
    metric.totalLatencyMs += latencyMs;
    metric.maxLatencyMs = std::max(metric.maxLatencyMs, latencyMs);
}

nlohmann::json BuildMetricsSnapshot() {
    nlohmann::json routes = nlohmann::json::array();
    uint64_t totalRequests = 0;
    uint64_t totalErrors = 0;

    std::lock_guard<std::mutex> lock(gRouteMetricMutex);
    for (const auto& entry : gRouteMetrics) {
        const auto& route = entry.first;
        const auto& metric = entry.second;
        const double avgLatency = metric.totalRequests == 0
                                    ? 0.0
                                    : static_cast<double>(metric.totalLatencyMs) / static_cast<double>(metric.totalRequests);
        totalRequests += metric.totalRequests;
        totalErrors += metric.errorRequests;
        routes.push_back({
            {"route", route},
            {"requests", metric.totalRequests},
            {"errors", metric.errorRequests},
            {"avg_latency_ms", avgLatency},
            {"max_latency_ms", metric.maxLatencyMs},
        });
    }

    std::sort(routes.begin(), routes.end(), [](const nlohmann::json& a, const nlohmann::json& b) {
        return a.value("route", std::string()) < b.value("route", std::string());
    });

    const uint64_t nowMs = NowMs();
    return {
        {"module", "net_observability"},
        {"uptime_ms", nowMs - gServerStartMs},
        {"total_requests", totalRequests},
        {"total_errors", totalErrors},
        {"routes", routes},
    };
}

uint64_t NowMs() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::steady_clock::now().time_since_epoch())
                                     .count());
}

std::string NewBattleId(const std::string& account) {
    const uint64_t seq = gBattleIdSeq.fetch_add(1, std::memory_order_relaxed);
    return account + "-" + std::to_string(seq);
}

std::string ToBattleResultString(BattleManager::Result result) {
    switch (result) {
        case BattleManager::Result::WIN:
            return "win";
        case BattleManager::Result::LOSE:
            return "lose";
        case BattleManager::Result::DRAW:
            return "draw";
        case BattleManager::Result::ONGOING:
        default:
            return "ongoing";
    }
}

int TeamHpSum(const std::vector<BattleCharacter>& team) {
    int sum = 0;
    for (const auto& ch : team) {
        if (ch.currentAttr.hp > 0) {
            sum += static_cast<int>(ch.currentAttr.hp);
        }
    }
    return sum;
}

int TeamAliveCount(const std::vector<BattleCharacter>& team) {
    int count = 0;
    for (const auto& ch : team) {
        if (ch.isAlive) {
            ++count;
        }
    }
    return count;
}

nlohmann::json BattleLogsToJson(BattleManager& manager, size_t limit = 80) {
    nlohmann::json logs = nlohmann::json::array();
    auto recentLogs = manager.getRecentBattleLogs(limit);
    for (const auto& line : recentLogs) {
        logs.push_back(line);
    }
    return logs;
}

nlohmann::json BuildBattleSummary(const std::string& battleId,
                                 const std::string& account,
                                 BattleManager& manager,
                                 const std::string& result) {
    const auto& userTeam = manager.getUserTeam();
    const auto& enemyTeam = manager.getEnemyTeam();
    return {
        {"battle_id", battleId},
        {"account", account},
        {"round", manager.getRound()},
        {"player_hp", TeamHpSum(userTeam)},
        {"enemy_hp", TeamHpSum(enemyTeam)},
        {"player_alive", TeamAliveCount(userTeam)},
        {"enemy_alive", TeamAliveCount(enemyTeam)},
        {"ended", true},
        {"result", result},
        {"updated_at_ms", NowMs()},
    };
}

nlohmann::json BuildDamageBoard(BattleManager& manager) {
    nlohmann::json board = nlohmann::json::array();
    const auto& stats = manager.getDamageStats();

    auto appendTeam = [&](const std::vector<BattleCharacter>& team, const char* side) {
        for (const auto& ch : team) {
            int64_t damage = 0;
            auto it = stats.find(ch.battleId);
            if (it != stats.end()) {
                damage = it->second;
            }
            board.push_back({
                {"side", side},
                {"battle_id", ch.battleId},
                {"name", ch.name},
                {"damage", damage},
                {"alive", ch.isAlive},
            });
        }
    };

    appendTeam(manager.getUserTeam(), "user");
    appendTeam(manager.getEnemyTeam(), "enemy");

    std::sort(board.begin(), board.end(), [](const nlohmann::json& a, const nlohmann::json& b) {
        return a.value("damage", 0LL) > b.value("damage", 0LL);
    });

    return board;
}

nlohmann::json BuildDamageSummary(const nlohmann::json& board) {
    int64_t userTotal = 0;
    int64_t enemyTotal = 0;
    std::string mvpName;
    int64_t mvpDamage = 0;

    for (const auto& row : board) {
        const int64_t dmg = row.value("damage", 0LL);
        if (row.value("side", std::string()) == "user") {
            userTotal += dmg;
        } else {
            enemyTotal += dmg;
        }
        if (dmg > mvpDamage) {
            mvpDamage = dmg;
            mvpName = row.value("name", std::string("-"));
        }
    }

    return {
        {"user_total", userTotal},
        {"enemy_total", enemyTotal},
        {"mvp_name", mvpName},
        {"mvp_damage", mvpDamage},
    };
}

int ParsePositiveInt(const nlohmann::json& payload, const char* key, int fallback) {
    if (!payload.contains(key)) {
        return fallback;
    }
    if (!payload[key].is_number_integer()) {
        return -1;
    }
    int val = payload[key].get<int>();
    return val > 0 ? val : -1;
}

std::vector<int> ParseTeamConfigIds(const nlohmann::json& payload, const char* key) {
    std::vector<int> ids;
    if (!payload.contains(key) || !payload[key].is_array()) {
        return ids;
    }
    for (const auto& node : payload[key]) {
        if (node.is_number_integer()) {
            int id = node.get<int>();
            if (id > 0) {
                ids.push_back(id);
            }
            continue;
        }

        if (node.is_object()) {
            int id = 0;
            if (node.contains("id") && node["id"].is_number_integer()) {
                id = node["id"].get<int>();
            } else if (node.contains("character_id") && node["character_id"].is_number_integer()) {
                id = node["character_id"].get<int>();
            }
            if (id > 0) {
                ids.push_back(id);
            }
        }
    }
    return ids;
}

std::vector<int> ParseTeamIds(const nlohmann::json& payload, const char* key) {
    std::vector<int> ids;
    if (!payload.contains(key) || !payload[key].is_array()) {
        return ids;
    }
    for (const auto& node : payload[key]) {
        if (node.is_number_integer()) {
            int id = node.get<int>();
            if (id > 0) {
                ids.push_back(id);
            }
        }
    }
    return ids;
}

std::vector<int> BuildDefaultTeam(const std::vector<int>& allIds, size_t startIndex) {
    std::vector<int> team;
    if (allIds.empty()) {
        return team;
    }
    for (size_t i = 0; i < 3 && i < allIds.size(); ++i) {
        team.push_back(allIds[(startIndex + i) % allIds.size()]);
    }
    return team;
}

bool FillPlayerWithCharacters(Player& player, const std::vector<int>& ids, std::string& err) {
    for (int charId : ids) {
        Character ch = CharacterConfig::instance().create(charId);
        if (ch.id == 0) {
            err = "invalid character id in team";
            return false;
        }
        if (!player.addCharacter(ch)) {
            err = "duplicate character id in team";
            return false;
        }
        if (!player.addToBattleTeam(charId)) {
            err = "failed to add character to battle team";
            return false;
        }
    }
    return !player.battleTeam.empty();
}

}

NetBootstrap::NetBootstrap() {
    config_ = NetConfig::LoadDefault();
    LOG_INFO("NetBootstrap config host=%s port=%d ioThreads=%d loginWorkers=%d", config_.host.c_str(), config_.port, config_.ioThreadCount, config_.loginThreadPoolSize);
    RegisterBuiltInHandlers();
}

NetBootstrap::~NetBootstrap() {
    Stop();
}

bool NetBootstrap::Start() {
    if (started_) {
        return true;
    }

    // 先拉起 IO 线程池，确保有事件循环可以接管新连接
    loopGroup_ = std::make_unique<EventLoopGroup>(config_, dispatcher_, diag_);
    if (!loopGroup_->Start()) {
        loopGroup_.reset();
        return false;
    }

    // 接入层最后启动，这样认证成功的 fd 能立即分发到可用的 EventLoop
    acceptor_ = std::make_unique<LoginAcceptor>(config_, *loopGroup_, diag_);
    if (!acceptor_->Start()) {
        loopGroup_->Stop();
        loopGroup_.reset();
        return false;
    }

    started_ = true;
    LOG_INFO("NetBootstrap started");
    return true;
}

void NetBootstrap::Stop() {
    if (!started_) {
        return;
    }
    if (acceptor_) {
        acceptor_->Stop();
    }
    if (loopGroup_) {
        loopGroup_->Stop();
    }
    acceptor_.reset();
    loopGroup_.reset();
    started_ = false;
    LOG_INFO("NetBootstrap stopped");
}

void NetBootstrap::RegisterBuiltInHandlers() {
    LOG_INFO("Register route GET /ping");
    dispatcher_.Register("GET", "/ping", [](const HttpRequest&, TcpConnection& conn) {
        const uint64_t startedMs = NowMs();
        LOG_INFO("Handle route GET /ping");
        HttpResponse resp;
        resp.SetHeader("Content-Type", "application/json");
        resp.body = "{\"status\":\"pong\"}";
        RecordRouteMetric("GET /ping", NowMs() - startedMs, resp.statusCode);
        conn.SendHttpResponse(resp);
    });

    LOG_INFO("Register route GET /metrics");
    dispatcher_.Register("GET", "/metrics", [](const HttpRequest&, TcpConnection& conn) {
        const uint64_t startedMs = NowMs();
        HttpResponse resp;
        resp.SetHeader("Content-Type", "application/json");
        resp.body = BuildMetricsSnapshot().dump();
        RecordRouteMetric("GET /metrics", NowMs() - startedMs, resp.statusCode);
        conn.SendHttpResponse(resp);
    });

    LOG_INFO("Register route POST /echo");
    dispatcher_.Register("POST", "/echo", [](const HttpRequest& req, TcpConnection& conn) {
        const uint64_t startedMs = NowMs();
        LOG_INFO("Handle route POST /echo body_size=%zu", req.body.size());
        HttpResponse resp;
        resp.SetHeader("Content-Type", "text/plain; charset=utf-8");
        resp.body = req.body;
        RecordRouteMetric("POST /echo", NowMs() - startedMs, resp.statusCode);
        conn.SendHttpResponse(resp);
    });

    LOG_INFO("Register route POST /login");
    dispatcher_.Register("POST", "/login", [](const HttpRequest& req, TcpConnection& conn) {
        const uint64_t startedMs = NowMs();
        HttpResponse resp;
        auto sendResponse = [&](const char* routeName) {
            RecordRouteMetric(routeName, NowMs() - startedMs, resp.statusCode);
            conn.SendHttpResponse(resp);
        };
        resp.SetHeader("Content-Type", "application/json");
        auto account = req.Header("x-account");
        auto token = req.Header("x-token");

        LOG_INFO("Handle route POST /login account=%s body_size=%zu", account.empty() ? "<empty>" : account.c_str(), req.body.size());

        if (account.empty() || token.empty()) {
            resp.statusCode = 400;
            resp.reason = "Bad Request";
            resp.body = "{\"error\":\"missing credentials\"}";
            sendResponse("POST /login");
            return;
        }

        if (!auth::ValidateCredential(account, token)) {
            resp.statusCode = 401;
            resp.reason = "Unauthorized";
            resp.body = "{\"error\":\"invalid credential\"}";
            sendResponse("POST /login");
            return;
        }

        auto ctx = conn.Context();
        ctx->account = account;
        ctx->TransitTo(ClientState::Authed);
        ctx->TouchHeartbeat();
        SessionManager::Instance().Register(account, conn.shared_from_this());

        resp.body = "{\"status\":\"ok\"}";
        sendResponse("POST /login");
    });

    LOG_INFO("Register route POST /battle");
    dispatcher_.Register("POST", "/battle", [](const HttpRequest& req, TcpConnection& conn) {
        const uint64_t startedMs = NowMs();
        HttpResponse resp;
        auto sendResponse = [&](const char* routeName) {
            RecordRouteMetric(routeName, NowMs() - startedMs, resp.statusCode);
            conn.SendHttpResponse(resp);
        };
        resp.SetHeader("Content-Type", "application/json");

        auto account = req.Header("x-account");
        LOG_INFO("Handle route POST /battle account=%s body_size=%zu", account.empty() ? "<empty>" : account.c_str(), req.body.size());

        if (account.empty()) {
            resp.statusCode = 400;
            resp.reason = "Bad Request";
            resp.body = "{\"code\":4001,\"error\":\"missing x-account\"}";
            sendResponse("POST /battle");
            return;
        }

        try {
            const auto payload = nlohmann::json::parse(req.body);
            if (!payload.contains("action") || !payload["action"].is_string()) {
                resp.statusCode = 400;
                resp.reason = "Bad Request";
                resp.body = "{\"code\":4002,\"error\":\"invalid action\"}";
                sendResponse("POST /battle");
                return;
            }

            const std::string action = payload["action"].get<std::string>();
            if (action != "auto") {
                resp.statusCode = 400;
                resp.reason = "Bad Request";
                resp.body = "{\"code\":4003,\"error\":\"unsupported action, only auto is allowed\"}";
                sendResponse("POST /battle");
                return;
            }

            auto allIds = CharacterConfig::instance().getAllIds();
            if (allIds.size() < 2) {
                resp.statusCode = 500;
                resp.reason = "Internal Server Error";
                resp.body = "{\"code\":5001,\"error\":\"character config not ready\"}";
                sendResponse("POST /battle");
                return;
            }

            auto userTeam = ParseTeamIds(payload, "user_team");
            if (userTeam.empty()) {
                userTeam = ParseTeamConfigIds(payload, "user_generals");
            }

            auto enemyTeam = ParseTeamIds(payload, "enemy_team");
            if (enemyTeam.empty()) {
                enemyTeam = ParseTeamConfigIds(payload, "enemy_generals");
            }

            if (userTeam.empty()) {
                userTeam = BuildDefaultTeam(allIds, 0);
            }
            if (enemyTeam.empty()) {
                enemyTeam = BuildDefaultTeam(allIds, 1);
            }

            Player user(1, account + "_user");
            Player enemy(2, account + "_enemy");

            std::string err;
            if (!FillPlayerWithCharacters(user, userTeam, err) ||
                !FillPlayerWithCharacters(enemy, enemyTeam, err)) {
                resp.statusCode = 400;
                resp.reason = "Bad Request";
                resp.body = "{\"code\":4006,\"error\":\"invalid battle team\"}";
                sendResponse("POST /battle");
                return;
            }

            BattleManager manager(&user, &enemy);
            BattleManager::Result result = manager.runBattle();
            std::string resultText = ToBattleResultString(result);
            std::string battleId = NewBattleId(account);
            nlohmann::json damageBoard = BuildDamageBoard(manager);

            nlohmann::json out = {
                {"code", 0},
                {"status", "ok"},
                {"module", "battle"},
                {"action", "auto"},
                {"message", "auto battle completed"},
                {"session", BuildBattleSummary(battleId, account, manager, resultText)},
                {"damage_board", damageBoard},
                {"damage_summary", BuildDamageSummary(damageBoard)},
                {"battle_logs", BattleLogsToJson(manager)},
                {"team_config", {
                    {"user_team", userTeam},
                    {"enemy_team", enemyTeam},
                }},
                {"loadout", payload.contains("loadout") && payload["loadout"].is_object() ? payload["loadout"] : nlohmann::json::object()},
            };

            resp.body = out.dump();
            sendResponse("POST /battle");
        } catch (const std::exception&) {
            resp.statusCode = 400;
            resp.reason = "Bad Request";
            resp.body = "{\"code\":4000,\"error\":\"invalid json\"}";
            sendResponse("POST /battle");
        }
    });
}

} // namespace webgame::net
