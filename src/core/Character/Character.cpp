#include "Character.h"
#include "ItemConfig.h"
#include <algorithm>
#include "CharacterConfig.h"
#include <cmath>

namespace {
bool isExclusiveTrigger(SkillTrigger trigger)
{
    switch (trigger) {
        case SkillTrigger::NORMAL_ATTACK:
        case SkillTrigger::RAGE_SKILL:
        case SkillTrigger::ON_SAME_CAMP:
            return true;
        default:
            return false;
    }
}
std::string GetDescByBreakthrough(int breakthrough)
{
    switch (breakthrough) {
        case 0: return "";
        case 1: return "超凡";
        case 2: return "超凡";
        case 3: return "绝世";
        case 4: return "绝世";
        case 5: return "无双";
        case 6: return "无双";
        case 7: return "至尊";
        case 8: return "至尊";
        case 9: return "神威";
        default: return "神尊";
    }
}
}

// ==================== 构造函数 ====================
Character::Character()
    : id(0)
    , relid(0)
    , name()
    , quality(QualityType::WHITE)
    , position(Position::WARRIOR)
    , baseName()
    , exp(0)
    , expMax(100)
    , breakthrough(0)
    , level(1)
    , star(1) {}

Character::Character(int id_, const std::string& name_, QualityType quality_, Position position_)
    : id(id_)
    , relid(0)
    , name(name_)
    , quality(quality_)
    , position(position_)
    , baseName(name_)
    , exp(0)
    , expMax(0)
    , breakthrough(0)
    , level(1)
    , star(1)
{
    initExpMax();
};

// ==================== 技能管理 ====================
void Character::setSkill(const Skill& skill)
{
    if (skill.id == 0) {
        return;
    }
    // 获取武将技能列表
    auto& skillList = skills[skill.trigger];
    if (isExclusiveTrigger(skill.trigger)) {
        skillList.clear();
        skillList.push_back(skill);
        return;
    }
    auto it = std::find_if(skillList.begin(), skillList.end(), [&](const Skill& s) {
        return s.id == skill.id;
    });
    if (it != skillList.end()) {
        *it = skill;
    } else {
        skillList.push_back(skill);
    }
}

const Skill* Character::getSkill(SkillTrigger trigger) const {
    auto it = skills.find(trigger);
    if (it == skills.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.front();
}

const std::vector<Skill>* Character::getSkills(SkillTrigger trigger) const {
    auto it = skills.find(trigger);
    return it != skills.end() ? &it->second : nullptr;
}

bool Character::hasSkill(SkillTrigger trigger) const {
    auto it = skills.find(trigger);
    return it != skills.end() && !it->second.empty();
}

// ==================== 装备管理 ====================
void Character::equipItem(const Equipment& equip)
{
    // 卸下旧装备
    unequipItem(equip.type);
    
    // 装备新装备
    equipments[equip.type] = equip;
    setSkill(GET_SKILL(equip.skillId));
    // 重新计算属性
    recalculateAttr();
}

void Character::unequipItem(EquipmentType type) {
    auto equipIt = equipments.find(type);
    if (equipIt == equipments.end()) {
        return;
    }
    int removedSkillId = equipIt->second.skillId;
    equipments.erase(equipIt);

    if (removedSkillId != 0) {
        for (auto it = skills.begin(); it != skills.end(); ) {
            auto& skillList = it->second;
            skillList.erase(std::remove_if(skillList.begin(), skillList.end(),
                [removedSkillId](const Skill& s) { return s.id == removedSkillId; }),
                skillList.end());
            if (skillList.empty()) {
                it = skills.erase(it);
            } else {
                ++it;
            }
        }
    }

    recalculateAttr();
}

const Equipment* Character::getEquipment(EquipmentType type) const {
    auto it = equipments.find(type);
    return it != equipments.end() ? &it->second : nullptr;
}


// ==================== 属性计算 ====================
void Character::recalculateAttr() {
    // 1. 从基础属性开始
    baseAttr = originAttr;
    
    // 2. 应用所有装备
    for (const auto& [type, equip] : equipments) {
        baseAttr += equip.baseAttrs;
    }
    
    // 3. 应用套装加成
    applySetBonuses();
    
    // 4. 确保 HP 不超过最大值
    if (baseAttr.hp > baseAttr.maxHp) {
        baseAttr.hp = baseAttr.maxHp;
    }
}

void Character::applySetBonuses()
{
    // 统计套装件数<套装id, 件数>
    std::unordered_map<int, int> setCounts;
    for (const auto& [type, equip] : equipments) {
        if (equip.setId > 0) {
            setCounts[equip.setId]++;
        }
    }

    // 应用套装效果
    for (const auto& [setId, count] : setCounts) {
        const SetBonus* setBonus = ItemConfig::instance().getSetBonus(setId);
        if (!setBonus) continue;

        for (const auto& bonus : setBonus->bonuses) {
            if (count >= bonus.pieceCount) {
                setSkill(GET_SKILL(bonus.skillId));
                baseAttr += bonus.applyAttr;
            }
        }
    }
}

uint64_t Character::calculateCombatPower() const {
    // 战力计算公式（可根据需求调整）
    uint64_t power = 0;
    power += baseAttr.calculateCombatPower();
    // todo: 技能战力加成
    return power;
}

// ==================== 升级/进阶 ====================
bool Character::addExp(int64_t amount) {
    exp += amount;
    
    bool leveledUp = false;
    while (exp >= expMax && canLevelUp()) {
        levelUp();
        leveledUp = true;
    }
    
    return leveledUp;
}

bool Character::canLevelUp() const {
    // 等级上限：基础80级 + 突破等级*10, 最大等级120级
    int maxLevel = 80 + (breakthrough - 1) * 10;
    if (maxLevel > 120) {
        maxLevel = 120;
    }
    return level < maxLevel;
}

void Character::levelUp() {
    if (!canLevelUp()) return;
    
    level++;
    exp = 0;
    
    // 更新经验上限
    initExpMax();
    
    // 重新计算属性（CharacterConfig 会根据等级计算基础属性）
    // 基础属性->等级加固定值-->突破/星级加成
    originAttr.upgradeByLevel(level);
    recalculateAttr();
}

void Character::initExpMax() {
    // 经验需求公式：1000 * level^1.5
    expMax = static_cast<int64_t>(1000 * std::pow(level, 1.5));
}

bool Character::canBreakthrough() const {
    return breakthrough < 10;
}

void Character::DoBreakthrough() {
    if (!canBreakthrough()) return;
    
    breakthrough++;
    // 处理 baseName / name 可能为空的情况，避免生成类似 "超凡·" 的非法名称
    std::string suffix = baseName.empty() ? name : baseName;
    if (suffix.empty()) {
        // 没有可用的基础名称时，只使用突破描述
        name = GetDescByBreakthrough(breakthrough);
    } else {
        name = GetDescByBreakthrough(breakthrough) + "·" + suffix;
    }
    // 突破后属性额外加成（可选）
    originAttr.hp += originAttr.maxHp * 5 / 100;
    originAttr.maxHp += originAttr.maxHp * 5 / 100;
    originAttr.atk += originAttr.atk * 5 / 100;
    originAttr.def += originAttr.def * 5 / 100;
    originAttr.dodgeRate += 2; // 闪避率+2%
    originAttr.damageReduction += 5; // 伤害减免+5%
    recalculateAttr();
}

bool Character::canUpgradeStar() const {
    // 升星条件（需要消耗材料，这里只判断星级上限）
    return star < 5;
}

void Character::upgradeStar()
{
    if (!canUpgradeStar()) return;
    
    star++;
    
    // 升星属性加成
    originAttr.hp += originAttr.maxHp * 10 / 100;
    originAttr.maxHp += originAttr.maxHp * 10 / 100;
    originAttr.atk += originAttr.atk * 10 / 100;
    originAttr.def += originAttr.def * 10 / 100;
    originAttr.critRate += 3; // 暴击率+3%
    originAttr.critDamage += 30; // 暴击伤害+30%
    recalculateAttr();
}
