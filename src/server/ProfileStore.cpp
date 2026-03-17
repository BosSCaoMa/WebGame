#include "ProfileStore.h"

#include "LogM.h"

#include <cstdlib>
#include <algorithm>
#include <chrono>

#if WEBGAME_HAS_LIBPQ
#include <libpq-fe.h>
#endif

namespace webgame::server {

namespace {

constexpr int kDailyTaskNeedBattles = 3;
constexpr RewardDelta kSignInReward{40, 80, 1};
constexpr RewardDelta kDailyTaskReward{90, 120, 2};

uint64_t CurrentDayKey() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto sec = duration_cast<seconds>(now.time_since_epoch()).count();
    return static_cast<uint64_t>(sec / 86400);
}

int64_t LevelupNeedExp(int level) {
    return static_cast<int64_t>(level) * 100;
}

void ApplyLevelProgress(PlayerProfile& profile) {
    while (profile.exp >= LevelupNeedExp(profile.level)) {
        profile.exp -= LevelupNeedExp(profile.level);
        profile.level += 1;
        profile.gold += 50;
    }
}

std::string GetEnvOrDefault(const char* key, const char* fallback) {
    const char* value = std::getenv(key);
    if (value == nullptr || value[0] == '\0') {
        return fallback;
    }
    return value;
}

void ApplyRewardToProfile(PlayerProfile& profile, const RewardDelta& reward) {
    profile.exp += reward.exp;
    profile.gold += reward.gold;
    profile.diamond += reward.diamond;
    ApplyLevelProgress(profile);
}

} // namespace

PlayerProfileStore& PlayerProfileStore::Instance() {
    static PlayerProfileStore instance;
    return instance;
}

PlayerProfileStore::~PlayerProfileStore() {
#if WEBGAME_HAS_LIBPQ
    std::lock_guard<std::mutex> lock(mutex_);
    if (conn_) {
        PQfinish(conn_);
        conn_ = nullptr;
    }
#endif
}

PlayerProfile PlayerProfileStore::GetOrCreate(const std::string& account) {
    std::lock_guard<std::mutex> lock(mutex_);
    EnsureInitializedLocked();

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        return GetOrCreatePgLocked(account);
    }
#endif

    return GetOrCreateMemoryLocked(account);
}

PlayerProfile PlayerProfileStore::ApplyBattleReward(const std::string& account, const BattleReward& reward) {
    std::lock_guard<std::mutex> lock(mutex_);
    EnsureInitializedLocked();

    PlayerProfile profile;
#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        profile = GetOrCreatePgLocked(account);
    } else {
        profile = GetOrCreateMemoryLocked(account);
    }
#else
    profile = GetOrCreateMemoryLocked(account);
#endif

    profile.exp += reward.exp;
    profile.gold += reward.gold;
    profile.diamond += reward.diamond;
    ApplyLevelProgress(profile);

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        if (!UpsertPgLocked(account, profile)) {
            LOG_WARN("ProfileStore update postgres failed, fallback to memory account=%s", account.c_str());
            memoryProfiles_[account] = profile;
            persistent_ = false;
            backendName_ = "memory-fallback";
            pgReady_ = false;
            if (conn_) {
                PQfinish(conn_);
                conn_ = nullptr;
            }
        }
    } else {
        memoryProfiles_[account] = profile;
    }
#else
    memoryProfiles_[account] = profile;
#endif

    return profile;
}

bool PlayerProfileStore::AppendBattleRecord(const std::string& account, const BattleRecord& record) {
    std::lock_guard<std::mutex> lock(mutex_);
    EnsureInitializedLocked();

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        if (AppendBattleRecordPgLocked(account, record)) {
            return true;
        }
        LOG_WARN("ProfileStore append battle record to postgres failed, fallback memory account=%s", account.c_str());
        persistent_ = false;
        backendName_ = "memory-fallback";
        pgReady_ = false;
        if (conn_) {
            PQfinish(conn_);
            conn_ = nullptr;
        }
    }
#endif

    return AppendBattleRecordMemoryLocked(account, record);
}

std::vector<BattleRecord> PlayerProfileStore::GetRecentBattleRecords(const std::string& account, size_t limit) {
    std::lock_guard<std::mutex> lock(mutex_);
    EnsureInitializedLocked();

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        return GetRecentBattleRecordsPgLocked(account, limit);
    }
#endif

    return GetRecentBattleRecordsMemoryLocked(account, limit);
}

std::vector<LeaderboardEntry> PlayerProfileStore::GetLeaderboard(size_t limit) {
    std::lock_guard<std::mutex> lock(mutex_);
    EnsureInitializedLocked();

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        return GetLeaderboardPgLocked(limit);
    }
#endif

    return GetLeaderboardMemoryLocked(limit);
}

DailyState PlayerProfileStore::GetDailyState(const std::string& account) {
    std::lock_guard<std::mutex> lock(mutex_);
    EnsureInitializedLocked();

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        return GetOrCreateDailyPgLocked(account);
    }
#endif

    return GetOrCreateDailyMemoryLocked(account);
}

void PlayerProfileStore::AddDailyBattleProgress(const std::string& account, int count) {
    if (count <= 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    EnsureInitializedLocked();

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        DailyState state = GetOrCreateDailyPgLocked(account);
        state.battleCount = std::max(0, state.battleCount + count);
        if (!UpsertDailyPgLocked(account, state)) {
            LOG_WARN("ProfileStore add daily progress postgres failed, fallback memory account=%s", account.c_str());
            pgReady_ = false;
            persistent_ = false;
            backendName_ = "memory-fallback";
            if (conn_) {
                PQfinish(conn_);
                conn_ = nullptr;
            }
            UpsertDailyMemoryLocked(account, state);
        }
        return;
    }
#endif

    DailyState state = GetOrCreateDailyMemoryLocked(account);
    state.battleCount = std::max(0, state.battleCount + count);
    UpsertDailyMemoryLocked(account, state);
}

DailyActionResult PlayerProfileStore::SignInDaily(const std::string& account) {
    std::lock_guard<std::mutex> lock(mutex_);
    EnsureInitializedLocked();

    DailyActionResult result;
    result.reward = kSignInReward;

    PlayerProfile profile;
    DailyState state;

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        profile = GetOrCreatePgLocked(account);
        state = GetOrCreateDailyPgLocked(account);
    } else {
        profile = GetOrCreateMemoryLocked(account);
        state = GetOrCreateDailyMemoryLocked(account);
    }
#else
    profile = GetOrCreateMemoryLocked(account);
    state = GetOrCreateDailyMemoryLocked(account);
#endif

    if (state.signedIn) {
        result.success = false;
        result.message = "already_signed_in";
        result.reward = RewardDelta{};
        result.profile = profile;
        result.state = state;
        return result;
    }

    state.signedIn = true;
    ApplyRewardToProfile(profile, result.reward);

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        if (!UpsertPgLocked(account, profile) || !UpsertDailyPgLocked(account, state)) {
            LOG_WARN("ProfileStore signin update postgres failed, fallback memory account=%s", account.c_str());
            pgReady_ = false;
            persistent_ = false;
            backendName_ = "memory-fallback";
            if (conn_) {
                PQfinish(conn_);
                conn_ = nullptr;
            }
            memoryProfiles_[account] = profile;
            UpsertDailyMemoryLocked(account, state);
        }
    } else {
        memoryProfiles_[account] = profile;
        UpsertDailyMemoryLocked(account, state);
    }
#else
    memoryProfiles_[account] = profile;
    UpsertDailyMemoryLocked(account, state);
#endif

    result.success = true;
    result.message = "ok";
    result.profile = profile;
    result.state = state;
    return result;
}

DailyActionResult PlayerProfileStore::ClaimDailyTask(const std::string& account) {
    std::lock_guard<std::mutex> lock(mutex_);
    EnsureInitializedLocked();

    DailyActionResult result;
    result.reward = kDailyTaskReward;

    PlayerProfile profile;
    DailyState state;

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        profile = GetOrCreatePgLocked(account);
        state = GetOrCreateDailyPgLocked(account);
    } else {
        profile = GetOrCreateMemoryLocked(account);
        state = GetOrCreateDailyMemoryLocked(account);
    }
#else
    profile = GetOrCreateMemoryLocked(account);
    state = GetOrCreateDailyMemoryLocked(account);
#endif

    if (state.taskClaimed) {
        result.success = false;
        result.message = "task_already_claimed";
        result.reward = RewardDelta{};
        result.profile = profile;
        result.state = state;
        return result;
    }

    if (state.battleCount < kDailyTaskNeedBattles) {
        result.success = false;
        result.message = "task_not_ready";
        result.reward = RewardDelta{};
        result.profile = profile;
        result.state = state;
        return result;
    }

    state.taskClaimed = true;
    ApplyRewardToProfile(profile, result.reward);

#if WEBGAME_HAS_LIBPQ
    if (pgReady_) {
        if (!UpsertPgLocked(account, profile) || !UpsertDailyPgLocked(account, state)) {
            LOG_WARN("ProfileStore claim task postgres failed, fallback memory account=%s", account.c_str());
            pgReady_ = false;
            persistent_ = false;
            backendName_ = "memory-fallback";
            if (conn_) {
                PQfinish(conn_);
                conn_ = nullptr;
            }
            memoryProfiles_[account] = profile;
            UpsertDailyMemoryLocked(account, state);
        }
    } else {
        memoryProfiles_[account] = profile;
        UpsertDailyMemoryLocked(account, state);
    }
#else
    memoryProfiles_[account] = profile;
    UpsertDailyMemoryLocked(account, state);
#endif

    result.success = true;
    result.message = "ok";
    result.profile = profile;
    result.state = state;
    return result;
}

std::string PlayerProfileStore::BackendName() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return backendName_;
}

bool PlayerProfileStore::IsPersistent() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return persistent_;
}

void PlayerProfileStore::EnsureInitializedLocked() {
    if (initialized_) {
        return;
    }
    initialized_ = true;

#if WEBGAME_HAS_LIBPQ
    if (EnsurePgConnectedLocked() && EnsureSchemaLocked()) {
        pgReady_ = true;
        persistent_ = true;
        backendName_ = "postgresql";
        LOG_INFO("ProfileStore initialized with PostgreSQL backend");
        return;
    }
    if (conn_) {
        PQfinish(conn_);
        conn_ = nullptr;
    }
    pgReady_ = false;
    persistent_ = false;
    backendName_ = "memory-fallback";
    LOG_WARN("ProfileStore using memory fallback backend");
#else
    persistent_ = false;
    backendName_ = "memory";
    LOG_WARN("ProfileStore built without libpq, using memory backend");
#endif
}

PlayerProfile PlayerProfileStore::GetOrCreateMemoryLocked(const std::string& account) {
    auto it = memoryProfiles_.find(account);
    if (it != memoryProfiles_.end()) {
        return it->second;
    }
    PlayerProfile profile;
    memoryProfiles_[account] = profile;
    return profile;
}

bool PlayerProfileStore::AppendBattleRecordMemoryLocked(const std::string& account, const BattleRecord& record) {
    auto& records = memoryBattleRecords_[account];
    records.push_back(record);
    constexpr size_t kMaxRecords = 100;
    if (records.size() > kMaxRecords) {
        records.erase(records.begin(), records.begin() + (records.size() - kMaxRecords));
    }
    return true;
}

std::vector<BattleRecord> PlayerProfileStore::GetRecentBattleRecordsMemoryLocked(const std::string& account, size_t limit) {
    std::vector<BattleRecord> result;
    auto it = memoryBattleRecords_.find(account);
    if (it == memoryBattleRecords_.end()) {
        return result;
    }

    const auto& records = it->second;
    if (records.empty() || limit == 0) {
        return result;
    }

    const size_t start = records.size() > limit ? records.size() - limit : 0;
    result.assign(records.begin() + static_cast<std::ptrdiff_t>(start), records.end());
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<LeaderboardEntry> PlayerProfileStore::GetLeaderboardMemoryLocked(size_t limit) {
    std::vector<LeaderboardEntry> entries;
    if (limit == 0) {
        return entries;
    }

    entries.reserve(memoryProfiles_.size());
    for (const auto& it : memoryProfiles_) {
        LeaderboardEntry entry;
        entry.account = it.first;
        entry.level = it.second.level;
        entry.exp = it.second.exp;
        entry.gold = it.second.gold;
        entry.diamond = it.second.diamond;
        entries.push_back(entry);
    }

    std::sort(entries.begin(), entries.end(), [](const LeaderboardEntry& left, const LeaderboardEntry& right) {
        if (left.level != right.level) {
            return left.level > right.level;
        }
        if (left.gold != right.gold) {
            return left.gold > right.gold;
        }
        return left.account < right.account;
    });

    if (entries.size() > limit) {
        entries.resize(limit);
    }
    return entries;
}

DailyState PlayerProfileStore::GetOrCreateDailyMemoryLocked(const std::string& account) {
    const uint64_t today = CurrentDayKey();
    auto it = memoryDailyStates_.find(account);
    if (it == memoryDailyStates_.end()) {
        DailyState state;
        state.dayKey = today;
        memoryDailyStates_[account] = state;
        return state;
    }

    DailyState state = it->second;
    if (state.dayKey != today) {
        state.dayKey = today;
        state.signedIn = false;
        state.battleCount = 0;
        state.taskClaimed = false;
        it->second = state;
    }
    return state;
}

void PlayerProfileStore::UpsertDailyMemoryLocked(const std::string& account, const DailyState& state) {
    memoryDailyStates_[account] = state;
}

#if WEBGAME_HAS_LIBPQ
bool PlayerProfileStore::EnsurePgConnectedLocked() {
    if (conn_) {
        if (PQstatus(conn_) == CONNECTION_OK) {
            return true;
        }
        PQfinish(conn_);
        conn_ = nullptr;
    }

    const std::string host = GetEnvOrDefault("WEBGAME_PG_HOST", "127.0.0.1");
    const std::string port = GetEnvOrDefault("WEBGAME_PG_PORT", "5432");
    const std::string dbname = GetEnvOrDefault("WEBGAME_PG_DB", "webgame");
    const std::string user = GetEnvOrDefault("WEBGAME_PG_USER", "webgame");
    const std::string password = GetEnvOrDefault("WEBGAME_PG_PASSWORD", "webgame");

    const std::string conninfo =
        "host=" + host +
        " port=" + port +
        " dbname=" + dbname +
        " user=" + user +
        " password=" + password +
        " connect_timeout=2";

    conn_ = PQconnectdb(conninfo.c_str());
    if (!conn_ || PQstatus(conn_) != CONNECTION_OK) {
        LOG_WARN("ProfileStore postgres connect failed: %s", conn_ ? PQerrorMessage(conn_) : "unknown");
        return false;
    }
    return true;
}

bool PlayerProfileStore::EnsureSchemaLocked() {
    static const char* kSql =
        "CREATE TABLE IF NOT EXISTS user_profiles ("
        "account VARCHAR(64) PRIMARY KEY,"
        "level INTEGER NOT NULL DEFAULT 1,"
        "exp BIGINT NOT NULL DEFAULT 0,"
        "gold BIGINT NOT NULL DEFAULT 1000,"
        "diamond BIGINT NOT NULL DEFAULT 0,"
        "updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()"
        ");";

    PGresult* result = PQexec(conn_, kSql);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG_WARN("ProfileStore ensure schema failed: %s", result ? PQresultErrorMessage(result) : "unknown");
        if (result) {
            PQclear(result);
        }
        return false;
    }
    PQclear(result);

    static const char* kBattleSql =
        "CREATE TABLE IF NOT EXISTS battle_records ("
        "id BIGSERIAL PRIMARY KEY,"
        "account VARCHAR(64) NOT NULL,"
        "battle_id VARCHAR(128) NOT NULL,"
        "result VARCHAR(16) NOT NULL,"
        "rounds INTEGER NOT NULL DEFAULT 0,"
        "user_total_damage BIGINT NOT NULL DEFAULT 0,"
        "enemy_total_damage BIGINT NOT NULL DEFAULT 0,"
        "reward_exp BIGINT NOT NULL DEFAULT 0,"
        "reward_gold BIGINT NOT NULL DEFAULT 0,"
        "reward_diamond BIGINT NOT NULL DEFAULT 0,"
        "created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_battle_records_account_created_at ON battle_records(account, created_at DESC);";

    result = PQexec(conn_, kBattleSql);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG_WARN("ProfileStore ensure battle schema failed: %s", result ? PQresultErrorMessage(result) : "unknown");
        if (result) {
            PQclear(result);
        }
        return false;
    }
    PQclear(result);

    static const char* kDailySql =
        "CREATE TABLE IF NOT EXISTS daily_states ("
        "account VARCHAR(64) PRIMARY KEY,"
        "day_key BIGINT NOT NULL DEFAULT 0,"
        "signed_in BOOLEAN NOT NULL DEFAULT FALSE,"
        "battle_count INTEGER NOT NULL DEFAULT 0,"
        "task_claimed BOOLEAN NOT NULL DEFAULT FALSE,"
        "updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()"
        ");";

    result = PQexec(conn_, kDailySql);
    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG_WARN("ProfileStore ensure daily schema failed: %s", result ? PQresultErrorMessage(result) : "unknown");
        if (result) {
            PQclear(result);
        }
        return false;
    }
    PQclear(result);
    return true;
}

PlayerProfile PlayerProfileStore::GetOrCreatePgLocked(const std::string& account) {
    const char* insertParams[1] = { account.c_str() };
    PGresult* insertResult = PQexecParams(
        conn_,
        "INSERT INTO user_profiles(account) VALUES($1) ON CONFLICT(account) DO NOTHING",
        1,
        nullptr,
        insertParams,
        nullptr,
        nullptr,
        0);
    if (!insertResult || PQresultStatus(insertResult) != PGRES_COMMAND_OK) {
        LOG_WARN("ProfileStore insert profile failed account=%s error=%s", account.c_str(), insertResult ? PQresultErrorMessage(insertResult) : "unknown");
        if (insertResult) {
            PQclear(insertResult);
        }
        return GetOrCreateMemoryLocked(account);
    }
    PQclear(insertResult);

    const char* queryParams[1] = { account.c_str() };
    PGresult* queryResult = PQexecParams(
        conn_,
        "SELECT level, exp, gold, diamond FROM user_profiles WHERE account=$1",
        1,
        nullptr,
        queryParams,
        nullptr,
        nullptr,
        0);

    if (!queryResult || PQresultStatus(queryResult) != PGRES_TUPLES_OK || PQntuples(queryResult) != 1) {
        LOG_WARN("ProfileStore query profile failed account=%s error=%s", account.c_str(), queryResult ? PQresultErrorMessage(queryResult) : "unknown");
        if (queryResult) {
            PQclear(queryResult);
        }
        return GetOrCreateMemoryLocked(account);
    }

    PlayerProfile profile;
    profile.level = std::atoi(PQgetvalue(queryResult, 0, 0));
    profile.exp = std::atoll(PQgetvalue(queryResult, 0, 1));
    profile.gold = std::atoll(PQgetvalue(queryResult, 0, 2));
    profile.diamond = std::atoll(PQgetvalue(queryResult, 0, 3));

    PQclear(queryResult);
    return profile;
}

bool PlayerProfileStore::UpsertPgLocked(const std::string& account, const PlayerProfile& profile) {
    const std::string levelStr = std::to_string(profile.level);
    const std::string expStr = std::to_string(profile.exp);
    const std::string goldStr = std::to_string(profile.gold);
    const std::string diamondStr = std::to_string(profile.diamond);

    const char* params[5] = {
        account.c_str(),
        levelStr.c_str(),
        expStr.c_str(),
        goldStr.c_str(),
        diamondStr.c_str(),
    };

    PGresult* result = PQexecParams(
        conn_,
        "INSERT INTO user_profiles(account, level, exp, gold, diamond, updated_at) VALUES($1, $2, $3, $4, $5, NOW()) "
        "ON CONFLICT(account) DO UPDATE SET level=EXCLUDED.level, exp=EXCLUDED.exp, gold=EXCLUDED.gold, diamond=EXCLUDED.diamond, updated_at=NOW()",
        5,
        nullptr,
        params,
        nullptr,
        nullptr,
        0);

    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG_WARN("ProfileStore upsert failed account=%s error=%s", account.c_str(), result ? PQresultErrorMessage(result) : "unknown");
        if (result) {
            PQclear(result);
        }
        return false;
    }
    PQclear(result);
    return true;
}

bool PlayerProfileStore::AppendBattleRecordPgLocked(const std::string& account, const BattleRecord& record) {
    const std::string roundStr = std::to_string(record.round);
    const std::string userDamageStr = std::to_string(record.userTotalDamage);
    const std::string enemyDamageStr = std::to_string(record.enemyTotalDamage);
    const std::string rewardExpStr = std::to_string(record.rewardExp);
    const std::string rewardGoldStr = std::to_string(record.rewardGold);
    const std::string rewardDiamondStr = std::to_string(record.rewardDiamond);

    const char* params[9] = {
        account.c_str(),
        record.battleId.c_str(),
        record.result.c_str(),
        roundStr.c_str(),
        userDamageStr.c_str(),
        enemyDamageStr.c_str(),
        rewardExpStr.c_str(),
        rewardGoldStr.c_str(),
        rewardDiamondStr.c_str(),
    };

    PGresult* result = PQexecParams(
        conn_,
        "INSERT INTO battle_records(account, battle_id, result, rounds, user_total_damage, enemy_total_damage, reward_exp, reward_gold, reward_diamond, created_at) "
        "VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, NOW())",
        9,
        nullptr,
        params,
        nullptr,
        nullptr,
        0);

    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG_WARN("ProfileStore insert battle record failed account=%s error=%s", account.c_str(), result ? PQresultErrorMessage(result) : "unknown");
        if (result) {
            PQclear(result);
        }
        return false;
    }
    PQclear(result);
    return true;
}

std::vector<BattleRecord> PlayerProfileStore::GetRecentBattleRecordsPgLocked(const std::string& account, size_t limit) {
    std::vector<BattleRecord> records;
    if (limit == 0) {
        return records;
    }

    const std::string limitStr = std::to_string(limit);
    const char* params[2] = {
        account.c_str(),
        limitStr.c_str(),
    };

    PGresult* result = PQexecParams(
        conn_,
        "SELECT battle_id, result, rounds, user_total_damage, enemy_total_damage, reward_exp, reward_gold, reward_diamond, "
        "(EXTRACT(EPOCH FROM created_at) * 1000)::BIGINT AS created_at_ms "
        "FROM battle_records WHERE account=$1 ORDER BY created_at DESC LIMIT $2::int",
        2,
        nullptr,
        params,
        nullptr,
        nullptr,
        0);

    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        LOG_WARN("ProfileStore query battle history failed account=%s error=%s", account.c_str(), result ? PQresultErrorMessage(result) : "unknown");
        if (result) {
            PQclear(result);
        }
        return records;
    }

    const int rows = PQntuples(result);
    records.reserve(static_cast<size_t>(rows));
    for (int row = 0; row < rows; ++row) {
        BattleRecord record;
        record.battleId = PQgetvalue(result, row, 0);
        record.result = PQgetvalue(result, row, 1);
        record.round = std::atoi(PQgetvalue(result, row, 2));
        record.userTotalDamage = std::atoll(PQgetvalue(result, row, 3));
        record.enemyTotalDamage = std::atoll(PQgetvalue(result, row, 4));
        record.rewardExp = std::atoll(PQgetvalue(result, row, 5));
        record.rewardGold = std::atoll(PQgetvalue(result, row, 6));
        record.rewardDiamond = std::atoll(PQgetvalue(result, row, 7));
        record.createdAtMs = static_cast<uint64_t>(std::atoll(PQgetvalue(result, row, 8)));
        records.push_back(std::move(record));
    }

    PQclear(result);
    return records;
}

std::vector<LeaderboardEntry> PlayerProfileStore::GetLeaderboardPgLocked(size_t limit) {
    std::vector<LeaderboardEntry> entries;
    if (limit == 0) {
        return entries;
    }

    const std::string limitStr = std::to_string(limit);
    const char* params[1] = { limitStr.c_str() };

    PGresult* result = PQexecParams(
        conn_,
        "SELECT account, level, exp, gold, diamond FROM user_profiles ORDER BY level DESC, gold DESC, account ASC LIMIT $1::int",
        1,
        nullptr,
        params,
        nullptr,
        nullptr,
        0);

    if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
        LOG_WARN("ProfileStore query leaderboard failed: %s", result ? PQresultErrorMessage(result) : "unknown");
        if (result) {
            PQclear(result);
        }
        return entries;
    }

    const int rows = PQntuples(result);
    entries.reserve(static_cast<size_t>(rows));
    for (int row = 0; row < rows; ++row) {
        LeaderboardEntry entry;
        entry.account = PQgetvalue(result, row, 0);
        entry.level = std::atoi(PQgetvalue(result, row, 1));
        entry.exp = std::atoll(PQgetvalue(result, row, 2));
        entry.gold = std::atoll(PQgetvalue(result, row, 3));
        entry.diamond = std::atoll(PQgetvalue(result, row, 4));
        entries.push_back(std::move(entry));
    }

    PQclear(result);
    return entries;
}

DailyState PlayerProfileStore::GetOrCreateDailyPgLocked(const std::string& account) {
    const uint64_t today = CurrentDayKey();
    const std::string dayStr = std::to_string(today);

    const char* insertParams[2] = { account.c_str(), dayStr.c_str() };
    PGresult* insertResult = PQexecParams(
        conn_,
        "INSERT INTO daily_states(account, day_key, signed_in, battle_count, task_claimed, updated_at) VALUES($1, $2, FALSE, 0, FALSE, NOW()) "
        "ON CONFLICT(account) DO NOTHING",
        2,
        nullptr,
        insertParams,
        nullptr,
        nullptr,
        0);

    if (!insertResult || PQresultStatus(insertResult) != PGRES_COMMAND_OK) {
        LOG_WARN("ProfileStore insert daily state failed account=%s error=%s", account.c_str(), insertResult ? PQresultErrorMessage(insertResult) : "unknown");
        if (insertResult) {
            PQclear(insertResult);
        }
        return GetOrCreateDailyMemoryLocked(account);
    }
    PQclear(insertResult);

    const char* queryParams[1] = { account.c_str() };
    PGresult* queryResult = PQexecParams(
        conn_,
        "SELECT day_key, signed_in, battle_count, task_claimed FROM daily_states WHERE account=$1",
        1,
        nullptr,
        queryParams,
        nullptr,
        nullptr,
        0);

    if (!queryResult || PQresultStatus(queryResult) != PGRES_TUPLES_OK || PQntuples(queryResult) != 1) {
        LOG_WARN("ProfileStore query daily state failed account=%s error=%s", account.c_str(), queryResult ? PQresultErrorMessage(queryResult) : "unknown");
        if (queryResult) {
            PQclear(queryResult);
        }
        return GetOrCreateDailyMemoryLocked(account);
    }

    DailyState state;
    state.dayKey = static_cast<uint64_t>(std::atoll(PQgetvalue(queryResult, 0, 0)));
    {
        const char* signedRaw = PQgetvalue(queryResult, 0, 1);
        state.signedIn = signedRaw && (signedRaw[0] == 't' || signedRaw[0] == 'T' || signedRaw[0] == '1');
    }
    state.battleCount = std::atoi(PQgetvalue(queryResult, 0, 2));
    {
        const char* taskRaw = PQgetvalue(queryResult, 0, 3);
        state.taskClaimed = taskRaw && (taskRaw[0] == 't' || taskRaw[0] == 'T' || taskRaw[0] == '1');
    }
    PQclear(queryResult);

    if (state.dayKey != today) {
        state.dayKey = today;
        state.signedIn = false;
        state.battleCount = 0;
        state.taskClaimed = false;
        if (!UpsertDailyPgLocked(account, state)) {
            return GetOrCreateDailyMemoryLocked(account);
        }
    }

    return state;
}

bool PlayerProfileStore::UpsertDailyPgLocked(const std::string& account, const DailyState& state) {
    const std::string dayStr = std::to_string(state.dayKey);
    const std::string signStr = state.signedIn ? "true" : "false";
    const std::string battleCountStr = std::to_string(state.battleCount);
    const std::string taskClaimedStr = state.taskClaimed ? "true" : "false";

    const char* params[5] = {
        account.c_str(),
        dayStr.c_str(),
        signStr.c_str(),
        battleCountStr.c_str(),
        taskClaimedStr.c_str(),
    };

    PGresult* result = PQexecParams(
        conn_,
        "INSERT INTO daily_states(account, day_key, signed_in, battle_count, task_claimed, updated_at) VALUES($1, $2, $3::boolean, $4::int, $5::boolean, NOW()) "
        "ON CONFLICT(account) DO UPDATE SET day_key=EXCLUDED.day_key, signed_in=EXCLUDED.signed_in, battle_count=EXCLUDED.battle_count, task_claimed=EXCLUDED.task_claimed, updated_at=NOW()",
        5,
        nullptr,
        params,
        nullptr,
        nullptr,
        0);

    if (!result || PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG_WARN("ProfileStore upsert daily state failed account=%s error=%s", account.c_str(), result ? PQresultErrorMessage(result) : "unknown");
        if (result) {
            PQclear(result);
        }
        return false;
    }
    PQclear(result);
    return true;
}
#endif

} // namespace webgame::server
