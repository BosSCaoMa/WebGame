#pragma once

#include <vector>
#include <random>
#include <functional>
#include <string>
#include <array>
#include <unordered_map>
#include "BattleCharacter.h"

class Player;

class BattleManager {
public:
    // ==================== 战斗结果枚举 ====================
    enum class Result {
        ONGOING,    // 进行中
        WIN,        // 己方胜利
        LOSE,       // 己方失败
        DRAW        // 平局
    };

    // 日志回调类型
    using LogCallback = std::function<void(const std::string&)>;

public:
    // ==================== 构造与析构 ====================
    BattleManager(Player* user, Player* enemy, LogCallback logCallback = nullptr);
    ~BattleManager() = default;
    bool debugging = false;
    void SetDebugging();

    // ==================== 公共接口 ====================
    
    // 运行完整战斗，返回结果
    Result runBattle();
    
    // 执行单个回合，返回当前结果
    Result executeRound();
    
    // 获取当前状态
    Result getResult() const;
    int getRound() const;
    int getMaxRounds() const;
    bool isOver() const;
    
    // 获取队伍信息
    BattleCharacter* getBatCharById(int battleId);
    const std::vector<BattleCharacter>& getUserTeam() const;
    const std::vector<BattleCharacter>& getEnemyTeam() const;
    const std::unordered_map<int, int64_t>& getDamageStats() const;

private:
    // ==================== 初始化 ====================
    void initBattle();
    void buildUnitMap();
    void createBattleCharacters();
    
    // ==================== 回合流程 ====================
    void onRoundStart();
    void onRoundEnd();
    void calculateActionOrder();
    
    // ==================== 行动执行 ====================
    void executeAction(BattleCharacter* actor);
    void executeSkill(BattleCharacter* caster, Skill* skill);
    void executeEffect(BattleCharacter* caster, const SkillEffect& effect, int skillId);
    void applyEffect(BattleCharacter* caster, BattleCharacter* target, 
                     const SkillEffect& effect, int skillId);
    
    // ==================== 伤害计算 ====================
    int64_t calculateValue(BattleCharacter* caster, const SkillEffect& effect, 
                           BattleCharacter* target = nullptr);
    int64_t calculateDamage(BattleCharacter* caster, BattleCharacter* target, 
                            const SkillEffect& effect);
    int64_t calculateTrueDamage(BattleCharacter* caster, BattleCharacter* target,
                               const SkillEffect& effect);
    int64_t calculateHeal(BattleCharacter* caster, BattleCharacter* target,
                          const SkillEffect& effect);
    
    // ==================== 目标选择 ====================
    std::vector<BattleCharacter*> getTargets(BattleCharacter* caster, TargetType type);
    BattleCharacter* selectEnemyTarget(BattleCharacter* caster, 
                                        std::vector<BattleCharacter>& enemies);
    BattleCharacter* selectRandomAlive(std::vector<BattleCharacter>& team);
    std::vector<BattleCharacter*> selectRandom(std::vector<BattleCharacter>& team, int count);
    std::vector<BattleCharacter*> selectByAttr(std::vector<BattleCharacter>& team, TargetType now,
        TargetType base, bool byAtk, bool highest);
    
    // ==================== 技能触发 ====================
    void triggerSkills(SkillTrigger trigger);
    void triggerSkills(SkillTrigger trigger, BattleCharacter* specificCharacter);
    void triggerOnRoundX();

    // ==================== 状态检查 ====================
    Result checkBattleResult();
    void checkDeaths();
    bool isTeamAlive(const std::vector<BattleCharacter>& team) const;
    int countAlive(const std::vector<BattleCharacter>& team) const;
    
    // ==================== 工具函数 ====================
    void log(const std::string& message);
    void debug(const std::string& message);
    std::string getEffectName(EffectType type) const;
    bool rollChance(int percent);
    std::string formatCharacterState(const BattleCharacter& ch) const;
    std::string formatActionOrder() const;
    void logTeamState(const std::string& label, const std::vector<BattleCharacter>& team);
    void logDamageSummary();
    void logDamageForTeam(const std::vector<BattleCharacter>& team, const std::string& label);
    void resetDamageStats();
    void recordDamage(BattleCharacter* caster, int64_t damage);

    void logLine2(const std::string& s) { log("  - " + s); } // 统一缩进风格（可选）
    // 统一：加 buff + 日志
    void addBuffWithLog(BattleCharacter* target, EffectType type, int64_t value,
        int duration, bool canOverlay, int skillId);

    // 统一：应用伤害 + 统计实际伤害 + 日志 + 触发器 +（可选）连击
    void applyDamageWithStats(BattleCharacter* caster, BattleCharacter* target, int64_t rawDamage,
        bool allowShield, const char* damageLabel, int skillId, int maxExtraHits);

    int overlayKey(bool canOverlay, int skillId) const { return canOverlay ? 0 : skillId; }
private:
    // ==================== 成员变量 ====================
    
    // 玩家引用
    Player* userPlayer_;
    Player* enemyPlayer_;
    
    // 战斗队伍（副本）
    std::vector<BattleCharacter> userTeam_;
    std::vector<BattleCharacter> enemyTeam_;
    std::unordered_map<int, BattleCharacter*> unitMap; // battleId -> BattleCharacter*
    std::unordered_map<int, int64_t> damageStats_;      // battleId -> total damage dealt
    // 行动顺序
    std::vector<BattleCharacter*> actionOrder_;
    
    // 战斗状态
    int round_;
    int maxRounds_;
    Result result_;
    
    // 随机数生成器
    std::mt19937 rng_;
    LogCallback logCallback_;
    LogCallback debugCallback_;
    void setLogCallback(LogCallback callback);

    // 禁止拷贝
    BattleManager(const BattleManager&) = delete;
    BattleManager& operator=(const BattleManager&) = delete;
};