#include "BattleCharacter.h"
#include "LogM.h"
#include <limits>
#include <string>

using namespace std;

BattleCharacter::BattleCharacter(Character* ch, int battleId)
: original(ch), battleId(battleId), name(ch->name), 
    isAlive(true), hasActed(false), shieldValue(0)
{
    if (ch == nullptr) {
        LOG_ERROR("BattleCharacter::fromCharacter: null Character pointer");
    } else {
        baseAttr = ch->getTotalAttr();
        currentAttr = baseAttr;
        
        // 复制技能
        for (const auto& pair : ch->skills) {
            skills[pair.first] = pair.second;
        }
    }
}

void BattleCharacter::setLogger(LogCallback callback)
{
    logger_ = std::move(callback);
}

void BattleCharacter::log(const std::string& message) const
{
    if (logger_) {
        logger_(message);
    } else {
        LOG_DEBUG("%s", message.c_str());
    }
}

// ==================== Buff管理 ====================
// 添加buff之后，首先会重新计算属性，过期之后会进行删除，而伤害类Buff会在tickBuffs中处理
void BattleCharacter::addBuff(EffectType type, int64_t value, int duration, int sourceId)
{
    if (IsControlEffect(type)) {
        if (hasControlImmunity()) {
            return;
        }
        if (hasBuffOfType(EffectType::BARRIER)) {
            removeBuffsByType(EffectType::BARRIER);
            return;
        }
    }
    if (BuffIsOffset(type)) {
        return;
    }
    // 同类型同来源刷新, id为0代表可叠加，id不为0但sourceId相同则刷新持续时间和数值
    for (auto& b : buffs) {
        if (sourceId != 0 && b.type == type && b.sourceId == sourceId) {
            b.duration = std::max(b.duration, duration);
            b.value = value;
            recalculateAttr();
            log(name + " 刷新状态 type=" + std::to_string(static_cast<int>(type)) +
                " value=" + std::to_string(value) + " duration=" + std::to_string(duration));
            return;
        }
    }
    buffs.emplace_back(type, value, duration, sourceId);
    recalculateAttr();
    log(name + " 获得状态 type=" + std::to_string(static_cast<int>(type)) +
        " value=" + std::to_string(value) + " duration=" + std::to_string(duration));
}

void BattleCharacter::tickBuffs()
{
    // 处理持续 dot伤害类依据攻击者的攻击力
    for (Buff& b : buffs) {
        switch (b.type) {
            case EffectType::BURN:
            // 灼烧的减速效果在recalculateAttr中处理
                takeDamage(b.value);
                break;
            case EffectType::BLEED:
                takeDamage(b.value, false);
                break;
            case EffectType::POISON:
                b.value = b.value * 110 / 100; // 中毒伤害每回合增加10%
                takeDamage(b.value, false);
                break;
            case EffectType::CURSE:
                takeDamage(b.value, false);
                addRage(-1); // 诅咒：同时扣除怒气
                break;
            case EffectType::BUFF_REGEN:
                heal(b.value);
                break;
            default:
                break;
        }
    }
    
    // 移除过期Buff
    buffs.erase(
        std::remove_if(buffs.begin(), buffs.end(), [](Buff& b) { return b.tick(); }),
        buffs.end()
    );
    recalculateAttr();
}

void BattleCharacter::recalculateAttr()
{
    // 先保存当前HP
    int64_t currentHp = currentAttr.hp;
    int currentRage = currentAttr.rage;
    
    currentAttr = baseAttr;
    currentAttr.hp = std::clamp(currentHp, static_cast<int64_t>(0), currentAttr.maxHp);
    currentAttr.rage = currentRage;
    for (const Buff& buff : buffs) {
        switch (buff.type) {
            case EffectType::BURN:
                // 灼烧降低10%基础速度
                currentAttr.speed -= baseAttr.speed * 10 / 100;
                break;
            case EffectType::BUFF_MAX_HP:
                currentAttr.maxHp += buff.value;
                break;
            case EffectType::BUFF_ATK:
                currentAttr.atk += buff.value;
                break;
            case EffectType::BUFF_DEF:
                currentAttr.def += buff.value;
                break;
            case EffectType::BUFF_SPEED:
                currentAttr.speed += buff.value;
                break;
            case EffectType::BUFF_CRIT_RATE:
                currentAttr.critRate += buff.value;
                break;
            case EffectType::BUFF_CRIT_RESIST:
                currentAttr.critResist += buff.value;
                break;
            case EffectType::BUFF_HIT_RATE:
                currentAttr.hitRate += buff.value;
                break;
            case EffectType::BUFF_DODGE_RATE:
                currentAttr.dodgeRate += buff.value;
                break;
            default:
                break;
        }
    }
}

bool BattleCharacter::BuffIsOffset(EffectType type) const
{
    int resistValue = 0;
    switch (type) {
        case EffectType::STUN:
            resistValue = currentAttr.getResistance(BattleAttr::Resistance::Stun);
            break;
        case EffectType::FREEZE:
            resistValue = currentAttr.getResistance(BattleAttr::Resistance::Freeze);
            break;
        case EffectType::SILENCE:
            resistValue = currentAttr.getResistance(BattleAttr::Resistance::Silence);
            break;
        case EffectType::TAUNT:
            resistValue = currentAttr.getResistance(BattleAttr::Resistance::Taunt);
            break;
        case EffectType::INJURY:
            resistValue = currentAttr.getResistance(BattleAttr::Resistance::Injury);
            break;
        case EffectType::POISON:
            resistValue = currentAttr.getResistance(BattleAttr::Resistance::Poison);
            break;
        case EffectType::BURN:
            resistValue = currentAttr.getResistance(BattleAttr::Resistance::Burn);
            break;
        case EffectType::BLEED:
            resistValue = currentAttr.getResistance(BattleAttr::Resistance::Bleed);
            break;
        case EffectType::CURSE:
            resistValue = currentAttr.getResistance(BattleAttr::Resistance::Curse);
            break;
        default:
            return false;
    }
    // resistValue是百分比，随机生成0-99
    int roll = rand() % 100;
    return roll < resistValue;
}

// ==================== 状态检查 ====================
bool BattleCharacter::isControlled() const
{
    for (const Buff& b : buffs) {
        if (b.type == EffectType::STUN || b.type == EffectType::FREEZE) {
            return true;
        }
    }
    return false;
}

bool BattleCharacter::isSilenced() const {
    for (const Buff& b : buffs) {
        if (b.type == EffectType::SILENCE) return true;
    }
    return false;
}

// ==================== 战斗操作 ====================
void BattleCharacter::takeDamage(int64_t damage, bool canBeShielded)
{
    if (damage <= 0 || isInvincible()) {
        if (damage > 0 && isInvincible()) {
            log(name + " 处于无敌，免疫伤害 " + std::to_string(damage));
        }
        return;
    }

    int64_t incoming = damage;
    int64_t shieldConsumed = 0;
    if (canBeShielded && shieldValue > 0) {
        if (shieldValue >= damage) {
            shieldConsumed = damage;
            shieldValue -= damage;
            damage = 0;
        } else {
            shieldConsumed = shieldValue;
            damage -= shieldValue;
            shieldValue = 0;
        }
    }
    
    currentAttr.hp -= damage;
    if (currentAttr.hp <= 0) {
        if (hasLockBleed()) {
            currentAttr.hp = 1;
            isAlive = true;
        } else {
            currentAttr.hp = 0;
            isAlive = false;
        }
    }
    const int64_t applied = std::max<int64_t>(0, incoming - shieldConsumed);
    log(name + " 受到伤害: 实际=" + std::to_string(applied) +
        " 护盾抵消=" + std::to_string(shieldConsumed) +
        " 剩余HP=" + std::to_string(currentAttr.hp));
}

void BattleCharacter::heal(int64_t amount) {
    if (!isAlive || amount <= 0 || hasInjury()) {
        return;
    }
    const int64_t before = currentAttr.hp;
    currentAttr.hp = std::min(currentAttr.hp + amount, currentAttr.maxHp);
    const int64_t healed = currentAttr.hp - before;
    if (healed > 0) {
        log(name + " 恢复生命 " + std::to_string(healed) +
            " (当前HP " + std::to_string(currentAttr.hp) + ")");
    }
}

void BattleCharacter::addRage(int amount) {
    const int before = currentAttr.rage;
    currentAttr.rage = std::clamp(currentAttr.rage + amount, 0, 6); // 【设定】假设最大怒气为6
    if (currentAttr.rage != before) {
        log(name + " 怒气变动 " + std::to_string(before) + " -> " +
            std::to_string(currentAttr.rage));
    }
}

void BattleCharacter::addShield(int64_t amount) {
    shieldValue += amount;
    log(name + " 增加护盾 " + std::to_string(amount) +
        " (当前护盾 " + std::to_string(shieldValue) + ")");
}

int BattleCharacter::dispelBuffs(int count)
{
    return removeBuffsInternal(false, count);
}

int BattleCharacter::cleanseDebuffs(int count)
{
    return removeBuffsInternal(true, count);
}

int BattleCharacter::transferDebuffsTo(BattleCharacter* target, int count)
{
    if (!target) {
        return 0;
    }
    int limit = count <= 0 ? std::numeric_limits<int>::max() : count; // count为0或负数表示转移所有
    int moved = 0;
    auto it = buffs.begin();
    while (it != buffs.end() && moved < limit) {
        if (it->isDebuff) {
            target->addBuff(it->type, it->value, it->duration, it->sourceId);
            it = buffs.erase(it);
            moved++;
        } else {
            ++it;
        }
    }
    if (moved > 0) {
        recalculateAttr();
        log(name + " 转移 " + std::to_string(moved) + " 个负面状态给 " + target->name);
    }
    return moved;
}

// ==================== 技能相关 ====================
Skill* BattleCharacter::getSkill(SkillTrigger trigger) {
    auto* skillList = getSkills(trigger);
    if (skillList && !skillList->empty()) {
        return &skillList->front();
    }
    return nullptr;
}

std::vector<Skill>* BattleCharacter::getSkills(SkillTrigger trigger) {
    auto it = skills.find(trigger);
    return it != skills.end() ? &it->second : nullptr;
}

const std::vector<Skill>* BattleCharacter::getSkills(SkillTrigger trigger) const {
    auto it = skills.find(trigger);
    return it != skills.end() ? &it->second : nullptr;
}

Skill* BattleCharacter::getNormalAttack() {
    return getSkill(SkillTrigger::NORMAL_ATTACK);
}

Skill* BattleCharacter::GetAction()
{
    // todo: 合击技能等
    if (currentAttr.rage >= 4) {
        if (Skill* skill = getSkill(SkillTrigger::RAGE_SKILL)) {
            log(name + " 选择怒气技能 [" + skill->name + "](ID:" +
                std::to_string(skill->id) + ")");
            return skill;
        }
    }
    Skill* normal = getNormalAttack();
    if (normal) {
        log(name + " 选择普攻 [" + normal->name + "](ID:" +
            std::to_string(normal->id) + ")");
    } else {
        log(name + " 没有可用技能，只能跳过");
    }
    return normal;
}

bool BattleCharacter::hasBuffOfType(EffectType type) const
{
    for (const Buff& b : buffs) {
        if (b.type == type) {
            return true;
        }
    }
    return false;
}

bool BattleCharacter::hasInjury() const
{
    return hasBuffOfType(EffectType::INJURY);
}

bool BattleCharacter::hasLockBleed() const
{
    return hasBuffOfType(EffectType::LOCK_BLEED);
}

bool BattleCharacter::hasControlImmunity() const
{
    return hasBuffOfType(EffectType::IMMUNITY) || hasBuffOfType(EffectType::INVINCIBLE);
}

bool BattleCharacter::isInvincible() const
{
    return hasBuffOfType(EffectType::INVINCIBLE);
}

int BattleCharacter::removeBuffsInternal(bool removeDebuff, int count)
{
    int limit = count <= 0 ? std::numeric_limits<int>::max() : count;
    int removed = 0;
    auto it = buffs.begin();
    while (it != buffs.end() && removed < limit) {
        if (it->isDebuff == removeDebuff) {
            it = buffs.erase(it);
            ++removed;
        } else {
            ++it;
        }
    }
    if (removed > 0) {
        recalculateAttr();
    }
    return removed;
}

int BattleCharacter::removeBuffsByType(EffectType type, int count)
{
    if (count == 0) {
        return 0;
    }
    int limit = count < 0 ? std::numeric_limits<int>::max() : count;
    int removed = 0;
    auto it = buffs.begin();
    while (it != buffs.end() && removed < limit) {
        if (it->type == type) {
            it = buffs.erase(it);
            ++removed;
        } else {
            ++it;
        }
    }
    if (removed > 0) {
        recalculateAttr();
    }
    return removed;
}