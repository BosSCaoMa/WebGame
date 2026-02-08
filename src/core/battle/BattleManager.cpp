#include "BattleManager.h"
#include "Player.h"
#include <algorithm>
#include <cmath>
#include "LogM.h"
#include <iostream>
#include <sstream>
#include <utility>

using namespace std;

#include <thread>
#include <chrono>
namespace {
void waitSeconds(double seconds)
{
    std::this_thread::sleep_for(
        std::chrono::duration<double>(seconds)
    );
}

string GetTargetType(TargetType type) {
    switch (type) {
        case TargetType::SELF: return "敌方前排";
        case TargetType::ALLY_ALL: return "己方全体";
        case TargetType::ENEMY_SINGLE: return "敌方单个目标";
        case TargetType::ENEMY_COL: return "敌方一列";
        case TargetType::ENEMY_ALL: return "敌方全体";
        case TargetType::ALLY_FRONT_ROW: return "己方前排";
        case TargetType::ALLY_BACK_ROW: return "己方后排";
        case TargetType::ENEMY_FRONT_ROW: return "敌方前排";
        case TargetType::ENEMY_BACK_ROW: return "敌方后排";
        case TargetType::ALLY_ATK_TOP1: return "己方攻击最高1人";
        case TargetType::ALLY_ATK_TOP2: return "己方攻击最高2人";
        case TargetType::ALLY_ATK_TOP3: return "己方攻击最高3人";
        case TargetType::ENEMY_ATK_TOP1: return "敌方攻击最高1人";
        case TargetType::ENEMY_ATK_TOP2: return "敌方攻击最高2人";
        case TargetType::ENEMY_ATK_TOP3: return "敌方攻击最高3人";
        case TargetType::ALLY_HP_LOW1: return "己方血量最低1人";
        case TargetType::ALLY_HP_LOW2: return "己方血量最低2人";
        case TargetType::ALLY_HP_LOW3: return "己方血量最低3人";
        case TargetType::ENEMY_HP_LOW1: return "敌方血量最低1人";
        case TargetType::ENEMY_HP_LOW2: return "敌方血量最低2人";
        case TargetType::ENEMY_HP_LOW3: return "敌方血量最低3人";
        case TargetType::ALLY_RANDOM_1: return "己方随机1人";
        case TargetType::ALLY_RANDOM_2: return "己方随机2人";
        case TargetType::ALLY_RANDOM_3: return "己方随机3人";
        case TargetType::ENEMY_RANDOM_1: return "敌方随机1人";
        case TargetType::ENEMY_RANDOM_2: return "敌方随机2人";
        case TargetType::ENEMY_RANDOM_3: return "敌方随机3人";
        default: return "未知目标类型";
    }
}
}


// ==================== 构造函数 ====================
BattleManager::BattleManager(Player* user, Player* enemy, LogCallback logCallback)
    : userPlayer_(user)
    , enemyPlayer_(enemy)
    , round_(0)
    , maxRounds_(50)
    , result_(Result::ONGOING)
    , logCallback_(logCallback)
{
    LOG_INFO("BattleManager initialized, user: %d, enemy: %d", userPlayer_->id, enemyPlayer_->id);
    if (!logCallback_) {
        setLogCallback([](const string& msg) {
            cout << msg << endl;
        });
    }
    rng_.seed(random_device{}());
    initBattle();
}

void  BattleManager::SetDebugging()
{
    debugging = true;
    debugCallback_ = [](const string& msg) {
        cout << msg << endl;
    };
}

// ==================== 初始化 ====================
void BattleManager::initBattle()
{
    userTeam_.clear();
    enemyTeam_.clear();
    actionOrder_.clear();
    round_ = 0;
    result_ = Result::ONGOING;
    
    createBattleCharacters();
    buildUnitMap();
    resetDamageStats();
}

void BattleManager::createBattleCharacters()
{
    int idx = 1;
    for (int i = 0; i < static_cast<int>(userPlayer_->battleTeam.size()); ++i) {
        int charId = userPlayer_->battleTeam[i];
        if (charId == 0) {
            idx++;
            continue;
        }
        Character* ch = userPlayer_->getCharacter(charId);
        if (ch) {
            userTeam_.emplace_back(ch, idx++);
            userTeam_.back().setLogger(logCallback_);
        }
    }
    idx = 1;
    for (int i = 0; i < static_cast<int>(enemyPlayer_->battleTeam.size()); ++i) {
        int charId = enemyPlayer_->battleTeam[i];
        if (charId == 0) {
            idx++;
            continue;
        }
        Character* ch = enemyPlayer_->getCharacter(charId);
        if (ch) {
            enemyTeam_.emplace_back(ch, -idx++);
            enemyTeam_.back().setLogger(logCallback_);
        }
    }
}

void BattleManager::buildUnitMap()
{
    unitMap.clear();
    for (auto& ch : userTeam_) {
        unitMap[ch.battleId] = &ch;
    }
    for (auto& ch : enemyTeam_) {
        unitMap[ch.battleId] = &ch;
    }
}

BattleCharacter* BattleManager::getBatCharById(int battleId)
{
    auto it = unitMap.find(battleId);
    if (it != unitMap.end()) {
        return it->second;
    }
    return nullptr;
}

// ==================== 公共接口.runBattle开始战斗 ====================
BattleManager::Result BattleManager::runBattle()
{
    log("===================战斗开始==================");
    logTeamState("我方初始", userTeam_);
    logTeamState("敌方初始", enemyTeam_);

    // 触发开局技能
    log("触发开局技能...");
    calculateActionOrder();
    triggerSkills(SkillTrigger::BATTLE_START);

    while (result_ == Result::ONGOING) {
        calculateActionOrder();
        executeRound();
    }
    
    switch (result_) {
        case Result::WIN:  LOG_INFO("=== 战斗胜利！ ==="); break;
        case Result::LOSE: LOG_INFO("=== 战斗失败！ ==="); break;
        case Result::DRAW: LOG_INFO("=== 战斗平局！ ==="); break;
        default:
            LOG_ERROR("Unexpected battle result");
            break;
    }
    logTeamState("战后我方", userTeam_);
    logTeamState("战后敌方", enemyTeam_);   // 必要：记录战后状态
    logDamageSummary();
    
    return result_;
}
// todo 优化整体战斗逻辑，可以先不做，等到程序可以运行之后，进行优化，这样可以看效果
BattleManager::Result BattleManager::executeRound()
{
    round_++;
    log("====== 回合 " + to_string(round_) + " 开始 ======");
    // 1. 回合开始阶段
    onRoundStart();

    // 2. 计算行动顺序
    calculateActionOrder();
    
    // 3. 依次行动
    for (BattleCharacter* actor : actionOrder_) {
        if (!actor->isAlive || actor->hasActed) {
            continue;
        }

        triggerSkills(SkillTrigger::TURN_START, actor);
        executeAction(actor);
        triggerSkills(SkillTrigger::TURN_END, actor);
        
        result_ = checkBattleResult();
        if (result_ != Result::ONGOING) {
            return result_;
        }
    }
    
    // 4. 回合结束阶段，处理buff等
    onRoundEnd();
    log("====== 回合 " + to_string(round_) + " 结束 ======");
    logTeamState("我方", userTeam_);
    logTeamState("敌方", enemyTeam_);
    
    // 5. 检查战斗结果
    result_ = checkBattleResult();
    
    // 6. 检查回合上限
    if (round_ >= maxRounds_ && result_ == Result::ONGOING) {
        result_ = Result::DRAW;
    }
    
    return result_;
}

// ==================== 回合流程 ====================
void BattleManager::onRoundStart()
{
    // 重置行动标记
    for (auto& ch : userTeam_) {
        ch.hasActed = false;
    }
    for (auto& ch : enemyTeam_) {
        ch.hasActed = false;
    }
    triggerOnRoundX();
    // 触发回合开始技能
    triggerSkills(SkillTrigger::ROUND_START);
    logTeamState("我方", userTeam_);
    logTeamState("敌方", enemyTeam_);
}

void BattleManager::onRoundEnd()
{
    // 触发回合结束技能
    triggerSkills(SkillTrigger::ROUND_END);
    
    // 检查死亡
    checkDeaths();
}

void BattleManager::calculateActionOrder()
{
    actionOrder_.clear();
    for (int i = 1; i <= 6; ++i) {
        BattleCharacter* userChar = getBatCharById(i);
        BattleCharacter* enemyChar = getBatCharById(-i);
        if (userChar && !enemyChar) {
            actionOrder_.push_back(userChar);
        } else if (!userChar && enemyChar) {
            actionOrder_.push_back(enemyChar);
        } else if (userChar && enemyChar) {
            if (userChar->currentAttr.speed >= enemyChar->currentAttr.speed) {
                actionOrder_.push_back(userChar);
                actionOrder_.push_back(enemyChar);
            } else {
                actionOrder_.push_back(enemyChar);
                actionOrder_.push_back(userChar);
            }
        }
    }
    if (!actionOrder_.empty()) {
        log("  行动顺序: " + formatActionOrder());
    }
}

// ==================== 行动执行（最核心战斗逻辑） ====================
void BattleManager::executeAction(BattleCharacter* actor)
{
    if (!actor || !actor->isAlive) {
        return;
    }
    actor->hasActed = true;

    {
        std::ostringstream oss;
        oss << "[行动] " << actor->name
            << " (HP " << actor->currentAttr.hp << "/" << actor->currentAttr.maxHp
            << ", 怒 " << static_cast<int>(actor->currentAttr.rage) << ")";
        log(oss.str());
    }

    // 被控制
    if (actor->isControlled()) {
        log("  → 被控制，无法行动");
        return;
    }

    Skill* skill = actor->GetAction();
    if (!skill) {
        log("  → 无可用行动，跳过");
        return;
    }

    if (skill->trigger == SkillTrigger::NORMAL_ATTACK) {
        actor->addRage(1);

        std::ostringstream oss;
        oss << "  → 普攻"
            << " | 怒气 +1 → "
            << static_cast<int>(actor->currentAttr.rage);
        log(oss.str());
    } else {
        actor->addRage(-4);

        std::ostringstream oss;
        oss << "  → 技能 [" << skill->name << "]"
            << " | 怒气 -4 → "
            << static_cast<int>(actor->currentAttr.rage);
        log(oss.str());
    }

    executeSkill(actor, skill);

    if (actor->isAlive) {
        actor->tickBuffs();
    } else {
        log("  → 行动中死亡，跳过 Buff");
    }

    checkDeaths();
}

void BattleManager::executeSkill(BattleCharacter* caster, Skill* skill)
{
    if (!skill || !caster) {
        LOG_ERROR("Invalid caster or skill in executeSkill");
        return;
    }
    if (skill->HasEffectHandler()) {
        skill->effectHandler(caster, this);
    }
    for (const SkillEffect& effect : skill->effects) {
        executeEffect(caster, effect, skill->id);
    }
}

// ====================== 效果执行函数，最终都会调用这个函数 =======================
void BattleManager::executeEffect(BattleCharacter* caster, const SkillEffect& effect, int skillId)
{
    // 概率判定
    if (effect.chance < 100 && !rollChance(effect.chance)) {
        log("  效果: " + getEffectName(effect.effect) + " 未触发");
        return;
    }
    
    // 获取目标
    vector<BattleCharacter*> targets = getTargets(caster, effect.target);
    if (targets.empty()) {
        return;
    }
    log("  效果: " + getEffectName(effect.effect) + " -> 目标数: " + to_string(targets.size()));
    
    // 对每个目标应用效果
    for (BattleCharacter* target : targets) {
        if (target && target->isAlive) {
            applyEffect(caster, target, effect, skillId);
        }
    }
}

void BattleManager::applyEffect(BattleCharacter* caster, BattleCharacter* target, const SkillEffect& effect,
    int skillId)
{
    if (!caster || !target) {
        LOG_ERROR("Invalid caster or target in applyEffect");
        return;
    }

    switch (effect.effect) {
        // ===== 即时伤害 =====
        case EffectType::DAMAGE:
        case EffectType::PIERCE: {
            const int64_t damage = calculateDamage(caster, target, effect);
            const bool allowShield = (effect.effect != EffectType::PIERCE); // 保留你原语义：PIERCE 穿盾
            applyDamageWithStats(caster, target, damage, allowShield, "伤害",
                skillId, /*maxExtraHits=*/7);
            break;
        }

        case EffectType::TRUE_DAMAGE: {
            const int64_t damage = calculateTrueDamage(caster, target, effect);
            applyDamageWithStats(caster, target, damage, /*allowShield=*/false, "真实伤害",
                skillId, /*maxExtraHits=*/7);
            break;
        }

        // ===== 治疗 =====
        case EffectType::HEAL: {
            const int64_t heal = calculateHeal(caster, target, effect);
            target->heal(heal);
            log("  - " + target->name + " 恢复 " + to_string(heal) + " 点生命"
                " (当前HP: " + to_string(target->currentAttr.hp) + ")");
            break;
        }

        // ===== 怒气操作 =====
        case EffectType::RAGE_CHANGE: {
            const int amount = static_cast<int>(calculateValue(caster, effect, target));
            target->addRage(amount);
            log("  - " + target->name + (amount >= 0 ? " 获得 " : " 减少 ")
                + to_string(std::abs(amount)) + " 点怒气"
                + " (当前怒气: " + to_string((int)target->currentAttr.rage) + ")");
            break;
        }

        // ===== 护盾 =====
        case EffectType::SHIELD: {
            const int64_t shield = calculateValue(caster, effect);
            target->addShield(shield);
            log("  - " + target->name + " 获得护盾 " + to_string(shield)
                + " (当前护盾: " + to_string(target->shieldValue) + ")");
            break;
        }

        // ===== Buff效果（全部取%） + 持续BUFF =====
        case EffectType::BUFF_MAX_HP:
        case EffectType::BUFF_ATK:
        case EffectType::BUFF_DEF:
        case EffectType::BUFF_SPEED:
        case EffectType::BUFF_CRIT_RATE:
        case EffectType::BUFF_CRIT_RESIST:
        case EffectType::BUFF_HIT_RATE:
        case EffectType::BUFF_DODGE_RATE:
        case EffectType::BUFF_REGEN: {
            const int64_t buffValue = calculateValue(caster, effect, target);
            addBuffWithLog(target, effect.effect, buffValue, effect.duration, effect.canOverlay, skillId);
            break;
        }

        // ===== 控制效果 =====
        case EffectType::STUN:
        case EffectType::SILENCE:
        case EffectType::FREEZE: {
            triggerSkills(SkillTrigger::ON_CONTROL, target);
            const int64_t controlPower = calculateValue(caster, effect, target);
            addBuffWithLog(target, effect.effect, controlPower, effect.duration, effect.canOverlay, skillId);
            break;
        }

        // ===== 特殊状态 =====
        case EffectType::BARRIER:
        case EffectType::INJURY:
        case EffectType::LOCK_BLEED:
        case EffectType::IMMUNITY:
        case EffectType::INVINCIBLE: {
            const int64_t statusValue = calculateValue(caster, effect, target);
            addBuffWithLog(target, effect.effect, statusValue, effect.duration, effect.canOverlay, skillId);
            break;
        }

        // ===== 持续伤害 DOT =====
        case EffectType::POISON:
        case EffectType::BURN:
        case EffectType::BLEED:
        case EffectType::CURSE: {
            const int64_t dotDamage = calculateValue(caster, effect, target);
            target->addBuff(effect.effect, dotDamage, effect.duration, overlayKey(effect.canOverlay, skillId));
            log("  - " + target->name + " 附加持续效果 " + getEffectName(effect.effect)
                + " (强度: " + to_string(dotDamage) + ", 回合: " + to_string(effect.duration) + ")");
            break;
        }

        // ===== 嘲讽 =====
        case EffectType::TAUNT: {
            triggerSkills(SkillTrigger::ON_CONTROL, target);
            target->addBuff(effect.effect, caster->battleId, effect.duration, overlayKey(effect.canOverlay, skillId));
            log("  - " + target->name + " 被嘲讽，目标指向 " + caster->name
                + " (持续: " + to_string(effect.duration) + ")");
            break;
        }

        // ===== 清除/净化 =====
        case EffectType::DISPEL: {
            const int limit = static_cast<int>(calculateValue(caster, effect, target));
            const int removed = target->dispelBuffs(limit);
            log(removed > 0
                ? ("  - " + target->name + " 被驱散 " + to_string(removed) + " 个增益")
                : ("  - " + target->name + " 没有可驱散的增益"));
            break;
        }

        case EffectType::CLEANSE: {
            const int limit = static_cast<int>(calculateValue(caster, effect, target));
            const int removed = target->cleanseDebuffs(limit);
            log(removed > 0
                ? ("  - " + target->name + " 净化 " + to_string(removed) + " 个负面状态")
                : ("  - " + target->name + " 没有可净化的负面状态"));
            break;
        }

        case EffectType::TRANSFER_DEBUFF: {
            // todo
            log("  - " + target->name + " 触发效果：负面转移（TODO）");
            break;
        }

        case EffectType::REVIVE: {
            // todo
            log("  - " + target->name + " 触发效果：复活（TODO）");
            break;
        }

        default:
            break;
    }
}


void BattleManager::addBuffWithLog(BattleCharacter* target, EffectType type, int64_t value,
        int duration, bool canOverlay, int skillId)
{
    target->addBuff(type, value, duration, overlayKey(canOverlay, skillId));
    log("  - " + target->name + " 获得状态 " + getEffectName(type));
}

void BattleManager::applyDamageWithStats(BattleCharacter* caster, BattleCharacter* target, int64_t rawDamage,
        bool allowShield, const char* damageLabel, int skillId, int maxExtraHits)
{
    const int64_t prevHp = target->currentAttr.hp;
    const int64_t prevShield = target->shieldValue;

    target->takeDamage(rawDamage, allowShield);

    const int64_t hpLoss = std::max<int64_t>(0, prevHp - target->currentAttr.hp);
    const int64_t shieldLoss = std::max<int64_t>(0, prevShield - target->shieldValue);
    const int64_t appliedDamage = hpLoss + shieldLoss;

    if (appliedDamage > 0) {
        recordDamage(caster, appliedDamage);
    }

    log("  - " + target->name + " 受到 " + to_string(rawDamage) + " 点" + damageLabel +
        " (剩余HP: " + to_string(target->currentAttr.hp) + ")");

    // 命中触发：命中就触发
    triggerSkills(SkillTrigger::ON_HIT, target);

    // 击杀触发：所有造成击杀的伤害都触发（包含真实伤害）
    if (!target->isAlive) {
        triggerSkills(SkillTrigger::ON_KILL, caster);
        return;
    }

    // 连击：限制次数避免无限递归
    if (maxExtraHits > 0 && rawDamage > 0 && caster->currentAttr.multiHitRate > 0) {
        if (rollChance(caster->currentAttr.multiHitRate)) {
            log("    连击触发，" + caster->name + " 进行额外一次攻击！");
            applyDamageWithStats(caster, target, rawDamage, allowShield, damageLabel,
                skillId, maxExtraHits - 1);
        }
    }
}


// ==================== 数值计算 ====================
int64_t BattleManager::calculateValue(BattleCharacter* caster, const SkillEffect& effect, BattleCharacter* target)
{
    const ValueExpr& expr = effect.valueExpr;

    if (expr.source == ValueSource::FIXED) {
        return expr.value;
    }

    BattleCharacter* owner = expr.owner == ValueOwner::CASTER ? caster : target;
    if (!owner) {
        return 0;
    }

    const auto getSourceValue = [](const BattleCharacter* character, ValueSource source) -> int64_t {
        if (!character) {
            return 0;
        }
        const auto& attr = character->currentAttr;
        switch (source) {
            case ValueSource::ATK:
                return attr.atk;
            case ValueSource::DEF:
                return attr.def;
            case ValueSource::MAX_HP:
                return attr.maxHp;
            case ValueSource::CUR_HP:
                return attr.hp;
            case ValueSource::LOST_HP:
                return std::max<int64_t>(0, attr.maxHp - attr.hp);
            case ValueSource::FIXED:
            default:
                return 0;
        }
    };

    const int64_t base = getSourceValue(owner, expr.source);

    if (expr.scale == ValueScale::ABSOLUTE) {
        return base;
    }

    if (expr.scale == ValueScale::PERCENT) {
        return base * expr.value / 10000;
    }

    return expr.value;
}

int64_t BattleManager::calculateDamage(BattleCharacter* caster, BattleCharacter* target,
    const SkillEffect& effect)
{
    if (!caster || !target) {
        LOG_ERROR("Invalid caster or target in calculateDamage");
        return 0;
    }
    // 1. 闪避判定
    int dodgeRate = target->currentAttr.dodgeRate - caster->currentAttr.hitRate;
    dodgeRate = clamp(dodgeRate, 0, 50);
    if (rollChance(dodgeRate)) {
        log(target->name + " 闪避了攻击！");
        return 0;
    }

    // 2. 基础伤害
    int64_t baseDamage = calculateValue(caster, effect, target);
    
    // 3. 防御减伤 (先扣防御)
    int64_t defense = max(static_cast<int64_t>(0), target->currentAttr.def);
    int64_t damage = max(static_cast<int64_t>(1), baseDamage - defense);
    
    // 4. 暴击判定 (对扣完防御后的伤害暴击)
    int critRate = caster->currentAttr.critRate - target->currentAttr.critResist;
    critRate = clamp(critRate, 0, 100);
    if (rollChance(critRate)) {
        damage = damage * caster->currentAttr.critDamage / 100;
        LOG_INFO("  - 暴击！");
    }
    
    // 5. 伤害加成/减免
    damage = damage * (100 + caster->currentAttr.damageBonus) / 100;
    damage = damage * (100 - target->currentAttr.damageReduction) / 100;
    
    // 6. 保底伤害
    damage = max(static_cast<int64_t>(1), damage);
    
    return damage;
}


int64_t BattleManager::calculateHeal(BattleCharacter* caster, BattleCharacter* target,
     const SkillEffect& effect)
{
    (void)target; // 目前治疗效果不受目标属性影响，预留参数以备后续扩展
    int64_t heal = calculateValue(caster, effect);
    // 治疗加成
    heal = heal * (100 + caster->currentAttr.healBonus) / 100;
    return max(static_cast<int64_t>(0), heal);
}

// ==================== 目标选择 ====================
vector<BattleCharacter*> BattleManager::getTargets(BattleCharacter* caster, TargetType type)
{
    if (!caster) {
        LOG_ERROR("Invalid caster in getTargets");
        return {};
    }
    vector<BattleCharacter*> targets;
    vector<BattleCharacter>& allyTeam = caster->battleId > 0 ? userTeam_ : enemyTeam_;
    vector<BattleCharacter>& enemyTeamRef = caster->battleId > 0 ? enemyTeam_ : userTeam_;

    // 检查嘲讽，如果为真则必须只攻击嘲讽者
    for (const Buff& b : caster->buffs) {
        if (b.type == EffectType::TAUNT) {
            auto it = unitMap.find(static_cast<int>(b.value));
            if (it != unitMap.end() && it->second) {
                targets.push_back(it->second);
                return targets;
            }
        }
    }
    
    switch (type) {
        // ===== 基础目标 =====
        case TargetType::SELF:
            return {caster};
             
        case TargetType::ALLY_ALL:
            for (auto& ch : allyTeam) {
                if (ch.isAlive) targets.push_back(&ch);
            }
            return targets;
            
        case TargetType::ENEMY_SINGLE: {
            BattleCharacter* t = selectEnemyTarget(caster, enemyTeamRef);
            if (t) targets.push_back(t);
            return targets;
        }

        case TargetType::ENEMY_ALL:
            for (auto& ch : enemyTeamRef) {
                if (ch.isAlive) targets.push_back(&ch);
            }
            return targets;
        
        case TargetType::ENEMY_COL: 
            {
                int col = (abs(caster->battleId) - 1) % 3; // 0,1,2
                for (auto& ch : enemyTeamRef) {
                    if (ch.isAlive && (abs(ch.battleId) - 1) % 3 == col) {
                        targets.push_back(&ch);
                    }
                }
                return targets;
            }

        // ===== 前后排 =====
        case TargetType::ALLY_FRONT_ROW:
            for (auto& ch : allyTeam) {
                if (ch.isAlive && ch.isInFrontRow()) targets.push_back(&ch);
            }
            if (!targets.empty()) {
                return targets;
            } // 前排没有目标时，改为选择后排
            [[fallthrough]];
        case TargetType::ALLY_BACK_ROW:
            for (auto& ch : allyTeam) {
                if (ch.isAlive && !ch.isInFrontRow()) targets.push_back(&ch);
            }
            return targets;
            
        case TargetType::ENEMY_FRONT_ROW:
            for (auto& ch : enemyTeamRef) {
                if (ch.isAlive && ch.isInFrontRow()) targets.push_back(&ch);
            }
            return targets;
            
        case TargetType::ENEMY_BACK_ROW:
            for (auto& ch : enemyTeamRef) {
                if (ch.isAlive && !ch.isInFrontRow()) targets.push_back(&ch);
            }
            return targets;
        
        // ===== 按攻击力排序 =====
        case TargetType::ALLY_ATK_TOP1:
        case TargetType::ALLY_ATK_TOP2:
        case TargetType::ALLY_ATK_TOP3:
            return selectByAttr(allyTeam, type, TargetType::ALLY_ATK_TOP1, true, true);
        
        case TargetType::ENEMY_ATK_TOP1:
        case TargetType::ENEMY_ATK_TOP2:
        case TargetType::ENEMY_ATK_TOP3:
            return selectByAttr(enemyTeamRef, type, TargetType::ENEMY_ATK_TOP1, true, true);
        
        // ===== 按血量排序 =====
        case TargetType::ALLY_HP_LOW1:
        case TargetType::ALLY_HP_LOW2:
        case TargetType::ALLY_HP_LOW3:
            return selectByAttr(allyTeam, type, TargetType::ALLY_HP_LOW1, false, false);
        
        case TargetType::ENEMY_HP_LOW1:
        case TargetType::ENEMY_HP_LOW2:
        case TargetType::ENEMY_HP_LOW3:
            return selectByAttr(enemyTeamRef, type, TargetType::ENEMY_HP_LOW1, false, false);
        
        // ===== 随机目标 =====
        case TargetType::ALLY_RANDOM_1:
        case TargetType::ALLY_RANDOM_2:
        case TargetType::ALLY_RANDOM_3: {
            int count = static_cast<int>(type) - static_cast<int>(TargetType::ALLY_RANDOM_1) + 1;
            return selectRandom(allyTeam, count);
        }
        
        case TargetType::ENEMY_RANDOM_1:
        case TargetType::ENEMY_RANDOM_2:
        case TargetType::ENEMY_RANDOM_3: {
            int count = static_cast<int>(type) - static_cast<int>(TargetType::ENEMY_RANDOM_1) + 1;
            return selectRandom(enemyTeamRef, count);
            return {};
        }
        default:
            return {};
    }
    
    return targets;
}

BattleCharacter* BattleManager::selectEnemyTarget(BattleCharacter* caster,
    vector<BattleCharacter>& enemies)
{
    // 优先选择镜像位置（battleId 取反）的敌人，找不到再随机
    const int mirrorId = -caster->battleId;
    auto mirrorIt = unitMap.find(mirrorId);
    if (mirrorIt != unitMap.end()) {
        BattleCharacter* candidate = mirrorIt->second;
        if (candidate && candidate->isAlive &&
            ((caster->battleId > 0 && candidate->battleId < 0) ||
             (caster->battleId < 0 && candidate->battleId > 0))) {
            return candidate;
        }
    }

    return selectRandomAlive(enemies);
}

BattleCharacter* BattleManager::selectRandomAlive(vector<BattleCharacter>& team)
{
    vector<BattleCharacter*> alive;
    for (auto& ch : team) {
        if (ch.isAlive) {
            alive.push_back(&ch);
        }
    }
    
    if (alive.empty()) {
        return nullptr;
    }
    
    uniform_int_distribution<size_t> dist(0, alive.size() - 1);
    return alive[dist(rng_)];
}

vector<BattleCharacter*> BattleManager::selectRandom(vector<BattleCharacter>& team, 
    int count)
{
    vector<BattleCharacter*> alive;
    for (auto& ch : team) {
        if (ch.isAlive) {
            alive.push_back(&ch);
        }
    }
    
    // 打乱顺序
    shuffle(alive.begin(), alive.end(), rng_);
    
    // 截取指定数量
    if (static_cast<int>(alive.size()) > count) {
        alive.resize(count);
    }
    
    return alive;
}

vector<BattleCharacter*> BattleManager::selectByAttr(vector<BattleCharacter>& team,
    TargetType now, TargetType base, bool byAtk, bool highest)
{
    int count = static_cast<int>(now) - static_cast<int>(base) + 1;
    vector<BattleCharacter*> alive;
    for (auto& ch : team) {
        if (ch.isAlive) {
            alive.push_back(&ch);
        }
    }
    
    // 排序
    if (byAtk) {
        sort(alive.begin(), alive.end(),
            [highest](BattleCharacter* a, BattleCharacter* b) {
                return highest ? 
                    (a->currentAttr.atk > b->currentAttr.atk) :
                    (a->currentAttr.atk < b->currentAttr.atk);
            });
    } else {
        // 按当前HP排序
        sort(alive.begin(), alive.end(),
            [highest](BattleCharacter* a, BattleCharacter* b) {
                return highest ? 
                    (a->currentAttr.hp > b->currentAttr.hp) :
                    (a->currentAttr.hp < b->currentAttr.hp);
            });
    }
    
    // 截取指定数量
    if (static_cast<int>(alive.size()) > count) {
        alive.resize(count);
    }
    
    return alive;
}

// ==================== 技能触发 ====================
void BattleManager::triggerSkills(SkillTrigger trigger)
{
    for (BattleCharacter* ch : actionOrder_) {
        if (!debugging) {
            waitSeconds(0.5); // 每个人物行动前等待0.5秒，增加节奏感
        }
        triggerSkills(trigger, ch);
    }
}

void BattleManager::triggerSkills(SkillTrigger trigger, BattleCharacter* specificCharacter)
{
    if (specificCharacter == nullptr) {
        LOG_ERROR("Invalid character in %d", static_cast<int>(trigger));
        return;
    }
    if (!specificCharacter->isAlive && trigger != SkillTrigger::ON_DEATH) {
        return;
    }
    const auto* skillList = specificCharacter->getSkills(trigger);
    if (skillList == nullptr) {
        return;
    }
    for (const Skill& skill : *skillList) {
        if (skill.id == 0) {
            continue;
        }
        if (!debugging) {
            waitSeconds(0.3); // 每个技能触发前等待0.3秒，增加节奏感
        }
        log(specificCharacter->name + " 触发技能: " + skill.name);
        for (const SkillEffect& effect : skill.effects) {
            executeEffect(specificCharacter, effect, skill.id);
        }
    }
}

void BattleManager::triggerOnRoundX()
{
    if (round_ == 10) {
        triggerSkills(SkillTrigger::ROUND_TEN);
    }
    if (round_ % 5 == 0) {
        triggerSkills(SkillTrigger::ROUND_MULTIPLE_OF_FIVE);
    }
    if (round_ % 2 == 0) {
        triggerSkills(SkillTrigger::ROUND_EVEN);
    } else {
        triggerSkills(SkillTrigger::ROUND_ODD);
    }
}

// ==================== 状态检查 ====================
BattleManager::Result BattleManager::checkBattleResult()
{
    bool userAlive = isTeamAlive(userTeam_);
    bool enemyAlive = isTeamAlive(enemyTeam_);
    
    if (!userAlive && !enemyAlive) {
        return Result::DRAW;
    }
    if (!enemyAlive) {
        return Result::WIN;
    }
    if (!userAlive) {
        return Result::LOSE;
    }
    
    return Result::ONGOING;
}

void BattleManager::checkDeaths()
{
    // 检查己方死亡
    for (auto& ch : userTeam_) {
        if (ch.currentAttr.hp <= 0 && ch.isAlive) {
            ch.isAlive = false;
            ch.currentAttr.hp = 0;
            LOG_INFO("%s 阵亡！", ch.name.c_str());
            triggerSkills(SkillTrigger::ON_DEATH, &ch);
        }
    }
    
    // 检查敌方死亡
    for (auto& ch : enemyTeam_) {
        if (ch.currentAttr.hp <= 0 && ch.isAlive) {
            ch.isAlive = false;
            ch.currentAttr.hp = 0;
            LOG_INFO("%s 阵亡！", ch.name.c_str());
            triggerSkills(SkillTrigger::ON_DEATH, &ch);
        }
    }
}

int64_t BattleManager::calculateTrueDamage(BattleCharacter* caster, BattleCharacter* target,
    const SkillEffect& effect)
{
    // 1. 闪避判定
    int dodgeRate = target->currentAttr.dodgeRate - caster->currentAttr.hitRate;
    dodgeRate = clamp(dodgeRate, 0, 50);
    if (rollChance(dodgeRate)) {
        return 0;
    }

    // 2. 基础伤害
    int64_t baseDamage = calculateValue(caster, effect, target);
    int critRate = caster->currentAttr.critRate - target->currentAttr.critResist;
    critRate = clamp(critRate, 0, 100);
    if (rollChance(critRate)) {
        baseDamage = baseDamage * caster->currentAttr.critDamage / 100;
        LOG_INFO("  - 真伤暴击！");
    }

    // 5. 伤害加成/减免
    baseDamage = baseDamage * (100 + caster->currentAttr.damageBonus) / 100;
    baseDamage = baseDamage * (100 - target->currentAttr.damageReduction) / 100;

    return max(static_cast<int64_t>(1), baseDamage);
}

// =================================================================
// ==================== 其他辅助函数,基本不会查看 ====================
// =================================================================

string BattleManager::getEffectName(EffectType type) const {
    switch (type) {
        case EffectType::DAMAGE:        return "伤害";
        case EffectType::PIERCE:       return "穿透伤害";
        case EffectType::TRUE_DAMAGE:   return "真实伤害";
        case EffectType::HEAL:          return "治疗";
        case EffectType::RAGE_CHANGE:      return "怒气增加";
        case EffectType::BUFF_MAX_HP:   return "生命值";
        case EffectType::BUFF_ATK:      return "攻击力";
        case EffectType::BUFF_DEF:      return "防御力";
        case EffectType::BUFF_SPEED:    return "速度";
        case EffectType::BUFF_CRIT_RATE: return "暴击率";
        case EffectType::BUFF_CRIT_RESIST: return "抗暴率";
        case EffectType::BUFF_HIT_RATE: return "命中率";
        case EffectType::BUFF_DODGE_RATE: return "闪避率";
        case EffectType::BARRIER:       return "屏障";
        case EffectType::BUFF_REGEN:    return "持续治疗";
        // 补充于此
        case EffectType::STUN:          return "眩晕";
        case EffectType::SILENCE:       return "沉默";
        case EffectType::FREEZE:        return "冰冻";
        case EffectType::POISON:        return "中毒";
        case EffectType::BURN:          return "灼烧";
        case EffectType::BLEED:         return "流血";
        case EffectType::CURSE:         return "诅咒";
        case EffectType::SHIELD:        return "护盾";
        case EffectType::TAUNT:         return "嘲讽";
        case EffectType::INJURY:        return "重伤";
        case EffectType::LOCK_BLEED:    return "锁血";
        case EffectType::IMMUNITY:      return "免疫控制";
        case EffectType::INVINCIBLE:    return "无敌";
        case EffectType::DISPEL:        return "驱散";
        case EffectType::CLEANSE:       return "净化";
        case EffectType::TRANSFER_DEBUFF: return "负面转移";
        case EffectType::REVIVE:        return "复活";
        default:                        return "未知效果";
    }
}

bool BattleManager::rollChance(int percent) {
    if (percent <= 0) return false;
    if (percent >= 100) return true;
    
    uniform_int_distribution<int> dist(1, 100);
    return dist(rng_) <= percent;
}

std::string BattleManager::formatCharacterState(const BattleCharacter& ch) const
{
    std::ostringstream oss;
    oss << ch.name << "(ID:" << ch.battleId << ", HP " << ch.currentAttr.hp
        << "/" << ch.currentAttr.maxHp << ", 怒" << static_cast<int>(ch.currentAttr.rage);
    if (ch.shieldValue > 0) {
        oss << ", 盾" << ch.shieldValue;
    }
    if (!ch.isAlive) {
        oss << ", 阵亡";
    }
    if (!ch.buffs.empty()) {
        oss << ", Buff" << ch.buffs.size();
    }
    oss << ")";
    return oss.str();
}

std::string BattleManager::formatActionOrder() const
{
    std::ostringstream oss;
    bool first = true;
    for (const auto* ch : actionOrder_) {
        if (!ch) {
            continue;
        }
        if (!first) {
            oss << " -> ";
        }
        oss << ch->name << "(ID:" << ch->battleId << ", 速" << ch->currentAttr.speed << ")";
        first = false;
    }
    if (first) {
        return "无";
    }
    return oss.str();
}

void BattleManager::logTeamState(const std::string& label, const std::vector<BattleCharacter>& team)
{
    std::ostringstream oss;

    oss << "========== " << label << " ==========\n";

    if (team.empty())
    {
        oss << "  (暂无单位)\n";
        oss << "==============================";
        log(oss.str());
        return;
    }

    for (size_t i = 0; i < team.size(); ++i)
    {
        oss << "  [" << (i + 1) << "] " << formatCharacterState(team[i]) << "\n";
    }

    oss << "==============================";

    log(oss.str());
}

void BattleManager::logDamageSummary()
{
    if (userTeam_.empty() && enemyTeam_.empty()) {
        return;
    }
    log("====== 伤害统计 ======");
    logDamageForTeam(userTeam_, "我方");
    logDamageForTeam(enemyTeam_, "敌方");
}

void BattleManager::logDamageForTeam(const std::vector<BattleCharacter>& team, const std::string& label)
{
    std::ostringstream oss;

    oss << "========== " << label << " 伤害统计 ==========\n";

    if (team.empty())
    {
        oss << "  (无参战单位)\n";
        oss << "====================================";
        log(oss.str());
        return;
    }

    std::vector<std::pair<const BattleCharacter*, int64_t>> records;
    records.reserve(team.size());

    for (const auto& ch : team)
    {
        int64_t dmg = 0;
        auto it = damageStats_.find(ch.battleId);
        if (it != damageStats_.end())
        {
            dmg = it->second;
        }
        records.emplace_back(&ch, dmg);
    }

    std::sort(records.begin(), records.end(), [](const auto& lhs, const auto& rhs) {
            if (lhs.second == rhs.second)
                return lhs.first->battleId < rhs.first->battleId;
            return lhs.second > rhs.second;
        });

    for (size_t i = 0; i < records.size(); ++i) {
        const auto& ch  = records[i].first;
        const auto  dmg = records[i].second;

        oss << "  [" << (i + 1) << "] "
            << ch->name
            << "伤害=" << dmg << "\n";
    }

    oss << "====================================";

    log(oss.str());
}

void BattleManager::resetDamageStats()
{
    damageStats_.clear();
    for (const auto& ch : userTeam_) {
        damageStats_[ch.battleId] = 0;
    }
    for (const auto& ch : enemyTeam_) {
        damageStats_[ch.battleId] = 0;
    }
}

void BattleManager::recordDamage(BattleCharacter* caster, int64_t damage)
{
    if (!caster || damage <= 0) {
        return;
    }
    damageStats_[caster->battleId] += damage;
}

void BattleManager::setLogCallback(LogCallback callback)
{
    logCallback_ = callback;
    for (auto& ch : userTeam_) {
        ch.setLogger(logCallback_);
    }
    for (auto& ch : enemyTeam_) {
        ch.setLogger(logCallback_);
    }
}

BattleManager::Result BattleManager::getResult() const {
    return result_;
}

int BattleManager::getRound() const {
    return round_;
}

int BattleManager::getMaxRounds() const {
    return maxRounds_;
}

bool BattleManager::isOver() const {
    return result_ != Result::ONGOING;
}

const vector<BattleCharacter>& BattleManager::getUserTeam() const {
    return userTeam_;
}

const vector<BattleCharacter>& BattleManager::getEnemyTeam() const {
    return enemyTeam_;
}

const unordered_map<int, int64_t>& BattleManager::getDamageStats() const {
    return damageStats_;
}

void BattleManager::log(const string& message) {
    if (logCallback_) {
        logCallback_(message);
    }
}

void BattleManager::debug(const string& message) {
    if (debugging) {
        debugCallback_(message);
    }
}

bool BattleManager::isTeamAlive(const vector<BattleCharacter>& team) const
{
    for (const auto& ch : team) {
        if (ch.isAlive) {
            return true;
        }
    }
    return false;
}

int BattleManager::countAlive(const vector<BattleCharacter>& team) const
{
    int count = 0;
    for (const auto& ch : team) {
        if (ch.isAlive) {
            count++;
        }
    }
    return count;
}