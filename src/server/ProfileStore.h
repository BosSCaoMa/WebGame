#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>

namespace webgame::server {

struct PlayerProfile {
    int level = 1;
    int64_t exp = 0;
    int64_t gold = 1000;
    int64_t diamond = 0;
};

struct BattleReward {
    int64_t exp = 0;
    int64_t gold = 0;
    int64_t diamond = 0;
};

struct BattleRecord {
    std::string battleId;
    std::string result;
    int round = 0;
    int64_t userTotalDamage = 0;
    int64_t enemyTotalDamage = 0;
    int64_t rewardExp = 0;
    int64_t rewardGold = 0;
    int64_t rewardDiamond = 0;
    uint64_t createdAtMs = 0;
};

struct LeaderboardEntry {
    std::string account;
    int level = 1;
    int64_t exp = 0;
    int64_t gold = 0;
    int64_t diamond = 0;
};

struct DailyState {
    uint64_t dayKey = 0;
    bool signedIn = false;
    int battleCount = 0;
    bool taskClaimed = false;
};

struct RewardDelta {
    int64_t exp = 0;
    int64_t gold = 0;
    int64_t diamond = 0;
};

struct DailyActionResult {
    bool success = false;
    std::string message;
    RewardDelta reward;
    PlayerProfile profile;
    DailyState state;
};

class PlayerProfileStore {
public:
    static PlayerProfileStore& Instance();

    PlayerProfile GetOrCreate(const std::string& account);
    PlayerProfile ApplyBattleReward(const std::string& account, const BattleReward& reward);
    bool AppendBattleRecord(const std::string& account, const BattleRecord& record);
    std::vector<BattleRecord> GetRecentBattleRecords(const std::string& account, size_t limit);
    std::vector<LeaderboardEntry> GetLeaderboard(size_t limit);
    DailyState GetDailyState(const std::string& account);
    void AddDailyBattleProgress(const std::string& account, int count);
    DailyActionResult SignInDaily(const std::string& account);
    DailyActionResult ClaimDailyTask(const std::string& account);

    std::string BackendName() const;
    bool IsPersistent() const;

private:
    PlayerProfileStore() = default;
    ~PlayerProfileStore();

    PlayerProfileStore(const PlayerProfileStore&) = delete;
    PlayerProfileStore& operator=(const PlayerProfileStore&) = delete;

    void EnsureInitializedLocked();
    PlayerProfile GetOrCreateMemoryLocked(const std::string& account);
    bool AppendBattleRecordMemoryLocked(const std::string& account, const BattleRecord& record);
    std::vector<BattleRecord> GetRecentBattleRecordsMemoryLocked(const std::string& account, size_t limit);
    std::vector<LeaderboardEntry> GetLeaderboardMemoryLocked(size_t limit);
    DailyState GetOrCreateDailyMemoryLocked(const std::string& account);
    void UpsertDailyMemoryLocked(const std::string& account, const DailyState& state);

#if WEBGAME_HAS_LIBPQ
    struct PGconn* conn_ = nullptr;
    bool pgReady_ = false;
    bool EnsurePgConnectedLocked();
    bool EnsureSchemaLocked();
    PlayerProfile GetOrCreatePgLocked(const std::string& account);
    bool UpsertPgLocked(const std::string& account, const PlayerProfile& profile);
    bool AppendBattleRecordPgLocked(const std::string& account, const BattleRecord& record);
    std::vector<BattleRecord> GetRecentBattleRecordsPgLocked(const std::string& account, size_t limit);
    std::vector<LeaderboardEntry> GetLeaderboardPgLocked(size_t limit);
    DailyState GetOrCreateDailyPgLocked(const std::string& account);
    bool UpsertDailyPgLocked(const std::string& account, const DailyState& state);
#endif

    bool initialized_ = false;
    bool persistent_ = false;
    std::string backendName_ = "memory";
    std::unordered_map<std::string, PlayerProfile> memoryProfiles_;
    std::unordered_map<std::string, std::vector<BattleRecord>> memoryBattleRecords_;
    std::unordered_map<std::string, DailyState> memoryDailyStates_;
    mutable std::mutex mutex_;
};

} // namespace webgame::server
