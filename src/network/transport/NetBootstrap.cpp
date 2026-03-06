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

#include <atomic>
#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <random>
#include <unordered_map>

namespace webgame::net {

namespace {

struct BattleRuntime {
    std::string battleId;
    std::string account;
    std::unique_ptr<Player> user;
    std::unique_ptr<Player> enemy;
    std::unique_ptr<BattleManager> manager;
    bool ended = false;
    std::string result = "ongoing";
    uint64_t updatedAtMs = 0;
};

std::mutex gBattleSessionMutex;
std::unordered_map<std::string, std::unique_ptr<BattleRuntime>> gBattleSessions;
std::atomic<uint64_t> gBattleIdSeq{1};

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

nlohmann::json SessionToJson(const BattleRuntime& session) {
    int playerHp = 0;
    int enemyHp = 0;
    int playerAlive = 0;
    int enemyAlive = 0;
    int round = 0;

    if (session.manager) {
        const auto& userTeam = session.manager->getUserTeam();
        const auto& enemyTeam = session.manager->getEnemyTeam();
        playerHp = TeamHpSum(userTeam);
        enemyHp = TeamHpSum(enemyTeam);
        playerAlive = TeamAliveCount(userTeam);
        enemyAlive = TeamAliveCount(enemyTeam);
        round = session.manager->getRound();
    }

    return {
        {"battle_id", session.battleId},
        {"account", session.account},
        {"round", round},
        {"player_hp", playerHp},
        {"enemy_hp", enemyHp},
        {"player_alive", playerAlive},
        {"enemy_alive", enemyAlive},
        {"ended", session.ended},
        {"result", session.result},
        {"updated_at_ms", session.updatedAtMs},
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
        LOG_INFO("Handle route GET /ping");
        HttpResponse resp;
        resp.SetHeader("Content-Type", "application/json");
        resp.body = "{\"status\":\"pong\"}";
        conn.SendHttpResponse(resp);
    });

    LOG_INFO("Register route POST /echo");
    dispatcher_.Register("POST", "/echo", [](const HttpRequest& req, TcpConnection& conn) {
        LOG_INFO("Handle route POST /echo body_size=%zu", req.body.size());
        HttpResponse resp;
        resp.SetHeader("Content-Type", "text/plain; charset=utf-8");
        resp.body = req.body;
        conn.SendHttpResponse(resp);
    });

    LOG_INFO("Register route POST /login");
    dispatcher_.Register("POST", "/login", [](const HttpRequest& req, TcpConnection& conn) {
        HttpResponse resp;
        resp.SetHeader("Content-Type", "application/json");
        auto account = req.Header("x-account");
        auto token = req.Header("x-token");

        LOG_INFO("Handle route POST /login account=%s body_size=%zu", account.empty() ? "<empty>" : account.c_str(), req.body.size());

        if (account.empty() || token.empty()) {
            resp.statusCode = 400;
            resp.reason = "Bad Request";
            resp.body = "{\"error\":\"missing credentials\"}";
            conn.SendHttpResponse(resp);
            return;
        }

        if (!auth::ValidateCredential(account, token)) {
            resp.statusCode = 401;
            resp.reason = "Unauthorized";
            resp.body = "{\"error\":\"invalid credential\"}";
            conn.SendHttpResponse(resp);
            return;
        }

        auto ctx = conn.Context();
        ctx->account = account;
        ctx->TransitTo(ClientState::Authed);
        ctx->TouchHeartbeat();
        SessionManager::Instance().Register(account, conn.shared_from_this());

        resp.body = "{\"status\":\"ok\"}";
        conn.SendHttpResponse(resp);
    });

    LOG_INFO("Register route POST /battle");
    dispatcher_.Register("POST", "/battle", [](const HttpRequest& req, TcpConnection& conn) {
        HttpResponse resp;
        resp.SetHeader("Content-Type", "application/json");

        auto account = req.Header("x-account");
        LOG_INFO("Handle route POST /battle account=%s body_size=%zu", account.empty() ? "<empty>" : account.c_str(), req.body.size());

        if (account.empty()) {
            resp.statusCode = 400;
            resp.reason = "Bad Request";
            resp.body = "{\"code\":4001,\"error\":\"missing x-account\"}";
            conn.SendHttpResponse(resp);
            return;
        }

        try {
            const auto payload = nlohmann::json::parse(req.body);
            if (!payload.contains("action") || !payload["action"].is_string()) {
                resp.statusCode = 400;
                resp.reason = "Bad Request";
                resp.body = "{\"code\":4002,\"error\":\"invalid action\"}";
                conn.SendHttpResponse(resp);
                return;
            }

            const std::string action = payload["action"].get<std::string>();
            if (action != "start" && action != "attack" && action != "auto" && action != "end") {
                resp.statusCode = 400;
                resp.reason = "Bad Request";
                resp.body = "{\"code\":4003,\"error\":\"unsupported action\"}";
                conn.SendHttpResponse(resp);
                return;
            }

            if (action == "start") {
                auto runtime = std::make_unique<BattleRuntime>();
                runtime->battleId = NewBattleId(account);
                runtime->account = account;

                runtime->user = std::make_unique<Player>(1, account + "_user");
                runtime->enemy = std::make_unique<Player>(2, account + "_enemy");

                auto allIds = CharacterConfig::instance().getAllIds();
                if (allIds.size() < 2) {
                    resp.statusCode = 500;
                    resp.reason = "Internal Server Error";
                    resp.body = "{\"code\":5001,\"error\":\"character config not ready\"}";
                    conn.SendHttpResponse(resp);
                    return;
                }

                auto userTeam = ParseTeamIds(payload, "user_team");
                auto enemyTeam = ParseTeamIds(payload, "enemy_team");

                if (userTeam.empty()) {
                    userTeam = BuildDefaultTeam(allIds, 0);
                }
                if (enemyTeam.empty()) {
                    enemyTeam = BuildDefaultTeam(allIds, 1);
                }

                std::string err;
                if (!FillPlayerWithCharacters(*runtime->user, userTeam, err) ||
                    !FillPlayerWithCharacters(*runtime->enemy, enemyTeam, err)) {
                    resp.statusCode = 400;
                    resp.reason = "Bad Request";
                    resp.body = "{\"code\":4006,\"error\":\"invalid battle team\"}";
                    conn.SendHttpResponse(resp);
                    return;
                }

                runtime->manager = std::make_unique<BattleManager>(runtime->user.get(), runtime->enemy.get());
                runtime->updatedAtMs = NowMs();

                const std::string createdBattleId = runtime->battleId;
                {
                    std::lock_guard<std::mutex> lock(gBattleSessionMutex);
                    gBattleSessions[createdBattleId] = std::move(runtime);
                }

                std::lock_guard<std::mutex> lock(gBattleSessionMutex);
                auto createdIt = gBattleSessions.find(createdBattleId);
                if (createdIt == gBattleSessions.end() || !createdIt->second) {
                    resp.statusCode = 500;
                    resp.reason = "Internal Server Error";
                    resp.body = "{\"code\":5002,\"error\":\"failed to create battle session\"}";
                    conn.SendHttpResponse(resp);
                    return;
                }

                nlohmann::json out = {
                    {"code", 0},
                    {"status", "ok"},
                    {"module", "battle"},
                    {"action", "start"},
                    {"session", SessionToJson(*createdIt->second)},
                };
                resp.body = out.dump();
                conn.SendHttpResponse(resp);
                return;
            }

            const bool hasBattleId = payload.contains("battle_id") && payload["battle_id"].is_string() && !payload["battle_id"].get<std::string>().empty();
            if (!hasBattleId && action == "attack") {
                static thread_local std::mt19937 rng(std::random_device{}());
                std::uniform_int_distribution<int> playerDist(12, 20);
                std::uniform_int_distribution<int> enemyDist(8, 15);
                int playerDamage = playerDist(rng);
                int enemyDamage = enemyDist(rng);

                nlohmann::json out = {
                    {"code", 0},
                    {"status", "ok"},
                    {"module", "battle"},
                    {"action", "attack"},
                    {"mode", "stateless"},
                    {"event", {
                        {"player_damage", playerDamage},
                        {"enemy_damage", enemyDamage}
                    }},
                    {"message", "no battle_id, fallback to stateless simulation"},
                };
                resp.body = out.dump();
                conn.SendHttpResponse(resp);
                return;
            }

            if (!hasBattleId) {
                resp.statusCode = 400;
                resp.reason = "Bad Request";
                resp.body = "{\"code\":4005,\"error\":\"missing battle_id\"}";
                conn.SendHttpResponse(resp);
                return;
            }

            const std::string battleId = payload["battle_id"].get<std::string>();

            std::lock_guard<std::mutex> lock(gBattleSessionMutex);
            auto it = gBattleSessions.find(battleId);
            if (it == gBattleSessions.end()) {
                resp.statusCode = 404;
                resp.reason = "Not Found";
                resp.body = "{\"code\":4041,\"error\":\"battle not found\"}";
                conn.SendHttpResponse(resp);
                return;
            }

            BattleRuntime& session = *it->second;
            if (session.account != account) {
                resp.statusCode = 403;
                resp.reason = "Forbidden";
                resp.body = "{\"code\":4031,\"error\":\"battle does not belong to account\"}";
                conn.SendHttpResponse(resp);
                return;
            }

            if (action == "end") {
                session.ended = true;
                if (session.result == "ongoing") {
                    session.result = "ended_by_client";
                }
                session.updatedAtMs = NowMs();

                nlohmann::json out = {
                    {"code", 0},
                    {"status", "ok"},
                    {"module", "battle"},
                    {"action", "end"},
                    {"session", SessionToJson(session)},
                };
                resp.body = out.dump();
                gBattleSessions.erase(it);
                conn.SendHttpResponse(resp);
                return;
            }

            if (session.ended) {
                resp.statusCode = 409;
                resp.reason = "Conflict";
                resp.body = "{\"code\":4091,\"error\":\"battle already ended\"}";
                conn.SendHttpResponse(resp);
                return;
            }

            if (action == "auto") {
                if (!session.manager) {
                    resp.statusCode = 500;
                    resp.reason = "Internal Server Error";
                    resp.body = "{\"code\":5003,\"error\":\"battle manager not initialized\"}";
                    conn.SendHttpResponse(resp);
                    return;
                }

                BattleManager::Result result = session.manager->runBattle();
                session.result = ToBattleResultString(result);
                session.ended = true;
                session.updatedAtMs = NowMs();

                std::vector<nlohmann::json> events;
                events.push_back({
                    {"type", "auto_battle_executed"},
                    {"round", session.manager->getRound()},
                    {"result", session.result},
                });

                nlohmann::json out = {
                    {"code", 0},
                    {"status", "ok"},
                    {"module", "battle"},
                    {"action", "auto"},
                    {"events", events},
                    {"session", SessionToJson(session)},
                };
                resp.body = out.dump();
                conn.SendHttpResponse(resp);
                return;
            }

            static thread_local std::mt19937 rng(std::random_device{}());
            (void)rng;

            if (!session.manager) {
                resp.statusCode = 500;
                resp.reason = "Internal Server Error";
                resp.body = "{\"code\":5003,\"error\":\"battle manager not initialized\"}";
                conn.SendHttpResponse(resp);
                return;
            }

            BattleManager::Result result = session.manager->executeRound();
            session.result = ToBattleResultString(result);
            if (result != BattleManager::Result::ONGOING) {
                session.ended = true;
            }

            session.updatedAtMs = NowMs();

            std::vector<nlohmann::json> events;
            events.push_back({
                {"type", "round_executed"},
                {"round", session.manager->getRound()},
                {"result", session.result},
            });

            nlohmann::json out = {
                {"code", 0},
                {"status", "ok"},
                {"module", "battle"},
                {"action", "attack"},
                {"events", events},
                {"session", SessionToJson(session)},
            };
            resp.body = out.dump();
            conn.SendHttpResponse(resp);
        } catch (const std::exception&) {
            resp.statusCode = 400;
            resp.reason = "Bad Request";
            resp.body = "{\"code\":4000,\"error\":\"invalid json\"}";
            conn.SendHttpResponse(resp);
        }
    });
}

} // namespace webgame::net
