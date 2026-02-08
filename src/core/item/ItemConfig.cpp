#include "ItemConfig.h"
#include <random>
#include "LogM.h"
#include "json.hpp"
#include <fstream>z
using json = nlohmann::json;

namespace {

template <typename Enum>
Enum LookupEnum(const std::unordered_map<std::string, Enum>& table, const std::string& key,
    Enum fallback, const char* enumName)
{
    auto it = table.find(key);
    if (it != table.end()) {
        return it->second;
    }
    LOG_INFO("%s: 未知取值 '%s'，使用默认值。", enumName, key.c_str());
    return fallback;
}

const std::unordered_map<std::string, EquipmentType> kEquipTypeMap = {
    {"WEAPON", EquipmentType::WEAPON},
    {"ARMOR", EquipmentType::ARMOR},
    {"HELMET", EquipmentType::HELMET},
    {"BOOTS", EquipmentType::BOOTS},
    {"STEED", EquipmentType::STEED},
    {"TALLY", EquipmentType::TALLY},
    {"TREASURE", EquipmentType::TREASURE},
    {"FAMOUS", EquipmentType::FAMOUS}
};

const std::unordered_map<std::string, QualityType> kQualityMap = {
    {"WHITE", QualityType::WHITE},
    {"GREEN", QualityType::GREEN},
    {"BLUE", QualityType::BLUE},
    {"PURPLE", QualityType::PURPLE},
    {"ORANGE", QualityType::ORANGE},
    {"RED", QualityType::RED},
    {"GOLD", QualityType::GOLD}
};

const std::unordered_map<std::string, ItemType> kItemTypeMap = {
    {"NONE", ItemType::NONE},
    {"CONSUMABLE", ItemType::CONSUMABLE},
    {"MATERIAL", ItemType::MATERIAL},
    {"TREASURE", ItemType::TREASURE},
    {"EQUIPMENT", ItemType::EQUIPMENT},
    {"CURRENCY", ItemType::CURRENCY}
};

const std::unordered_map<std::string, ConsumableType> kConsumableMap = {
    {"NONE", ConsumableType::NONE},
    {"HP_POTION", ConsumableType::HP_POTION},
    {"RAGE_POTION", ConsumableType::RAGE_POTION},
    {"EXP_POTION", ConsumableType::EXP_POTION},
    {"BUFF_POTION", ConsumableType::BUFF_POTION}
};

const std::unordered_map<std::string, EffectType> kEffectTypeMap = {
    {"NONE", EffectType::NONE},
    {"DAMAGE", EffectType::DAMAGE},
    {"PIERCE", EffectType::PIERCE},
    {"TRUE_DAMAGE", EffectType::TRUE_DAMAGE},
    {"HEAL", EffectType::HEAL},
    {"RAGE_CHANGE", EffectType::RAGE_CHANGE},
    {"DIVINE_POWER", EffectType::DIVINE_POWER},
    {"SHIELD", EffectType::SHIELD},
    {"BARRIER", EffectType::BARRIER},
    {"BUFF_MAX_HP", EffectType::BUFF_MAX_HP},
    {"BUFF_ATK", EffectType::BUFF_ATK},
    {"BUFF_DEF", EffectType::BUFF_DEF},
    {"BUFF_SPEED", EffectType::BUFF_SPEED},
    {"BUFF_CRIT_RATE", EffectType::BUFF_CRIT_RATE},
    {"BUFF_CRIT_RESIST", EffectType::BUFF_CRIT_RESIST},
    {"BUFF_HIT_RATE", EffectType::BUFF_HIT_RATE},
    {"BUFF_DODGE_RATE", EffectType::BUFF_DODGE_RATE},
    {"BUFF_REGEN", EffectType::BUFF_REGEN},
    {"STUN", EffectType::STUN},
    {"FREEZE", EffectType::FREEZE},
    {"SILENCE", EffectType::SILENCE},
    {"TAUNT", EffectType::TAUNT},
    {"INJURY", EffectType::INJURY},
    {"POISON", EffectType::POISON},
    {"BURN", EffectType::BURN},
    {"BLEED", EffectType::BLEED},
    {"CURSE", EffectType::CURSE},
    {"LOCK_BLEED", EffectType::LOCK_BLEED},
    {"REVIVE", EffectType::REVIVE},
    {"DISPEL", EffectType::DISPEL},
    {"CLEANSE", EffectType::CLEANSE},
    {"IMMUNITY", EffectType::IMMUNITY},
    {"INVINCIBLE", EffectType::INVINCIBLE},
    {"TRANSFER_DEBUFF", EffectType::TRANSFER_DEBUFF},
    {"MARK_HEAL", EffectType::MARK_HEAL},
    {"MARK_DAMAGE", EffectType::MARK_DAMAGE},
    {"MARK_KILL", EffectType::MARK_KILL},
    {"MARK_PROTECT", EffectType::MARK_PROTECT}
};

BattleAttr ParseBattleAttr(const json& node)
{
    BattleAttr attr;
    attr.hp = node.value("hp", static_cast<int64_t>(attr.hp));
    attr.maxHp = node.value("maxHp", static_cast<int64_t>(attr.maxHp));
    attr.atk = node.value("atk", static_cast<int64_t>(attr.atk));
    attr.def = node.value("def", static_cast<int64_t>(attr.def));
    attr.speed = node.value("speed", static_cast<int64_t>(attr.speed));
    attr.critRate = static_cast<BattleAttr::Rate>(node.value("critRate", attr.critRate));
    attr.critDamage = static_cast<BattleAttr::Rate>(node.value("critDamage", attr.critDamage));
    attr.hitRate = static_cast<BattleAttr::Rate>(node.value("hitRate", attr.hitRate));
    attr.dodgeRate = static_cast<BattleAttr::Rate>(node.value("dodgeRate", attr.dodgeRate));
    return attr;
}

} // namespace

// ==================== 创建装备实例 ====================
Equipment ItemConfig::createEquipment(int equipId) const
{
    const EquipmentTemplate* tmpl = getEquipment(equipId);
    if (!tmpl) {
        LOG_ERROR("Equipment template not found for id: %d", equipId);
        return Equipment();
    }
    
    Equipment equip(tmpl->id, tmpl->name, tmpl->type, tmpl->quality, tmpl->skillId, tmpl->setId);
    equip.baseAttrs = tmpl->baseAttrs;
    return equip;
}

void ItemConfig::clear()
{
    items_.clear();
    equipments_.clear();
    setBonuses_.clear();
}

bool loadFromJson(const json& root)
{
    const auto itemsIt = root.find("items");
    if (itemsIt != root.end() && itemsIt->is_array()) {
        for (const auto& node : *itemsIt) {
            if (!node.is_object()) {
                continue;
            }
            ItemTemplate item;
            item.id = node.value("id", 0);
            if (item.id == 0) {
                LOG_ERROR("ItemConfig: 忽略 id=0 的物品。");
                continue;
            }
            item.name = node.value("name", std::string("未命名物品"));
            const std::string typeStr = node.value("type", std::string("NONE"));
            item.type = LookupEnum(kItemTypeMap, typeStr, ItemType::NONE, "ItemType");
            const std::string subTypeStr = node.value("subType", std::string("NONE"));
            item.subType = LookupEnum(kConsumableMap, subTypeStr, ConsumableType::NONE, "ConsumableType");
            item.quality = node.value("quality", 0);
            item.description = node.value("description", std::string());
            item.maxStack = node.value("maxStack", 1);

            const auto effectsIt = node.find("effects");
            if (effectsIt != node.end() && effectsIt->is_array()) {
                for (const auto& eff : *effectsIt) {
                    if (!eff.is_object()) continue;
                    ItemTemplate::Effect effect;
                    const std::string effStr = eff.value("type", std::string("NONE"));
                    effect.type = LookupEnum(kEffectTypeMap, effStr, EffectType::NONE, "EffectType");
                    effect.value = eff.value("value", static_cast<int64_t>(0));
                    item.effects.push_back(effect);
                }
            }

            REG_ITEM(item);
        }
    }

    const auto equipsIt = root.find("equipments");
    if (equipsIt != root.end() && equipsIt->is_array()) {
        for (const auto& node : *equipsIt) {
            if (!node.is_object()) {
                continue;
            }
            EquipmentTemplate tmpl;
            tmpl.id = node.value("id", 0);
            if (tmpl.id == 0) {
                LOG_ERROR("ItemConfig: 忽略 id=0 的装备。");
                continue;
            }
            tmpl.name = node.value("name", std::string("未命名装备"));
            const std::string typeStr = node.value("type", std::string("WEAPON"));
            tmpl.type = LookupEnum(kEquipTypeMap, typeStr, EquipmentType::WEAPON, "EquipmentType");
            const std::string qualityStr = node.value("quality", std::string("WHITE"));
            tmpl.quality = LookupEnum(kQualityMap, qualityStr, QualityType::WHITE, "QualityType");
            tmpl.skillId = node.value("skillId", 0);
            tmpl.setId = node.value("setId", 0);
            const auto attrIt = node.find("baseAttrs");
            if (attrIt != node.end() && attrIt->is_object()) {
                tmpl.baseAttrs = ParseBattleAttr(*attrIt);
            }
            REG_EQUIP(tmpl);
        }
    }

    const auto setIt = root.find("setBonuses");
    if (setIt != root.end() && setIt->is_array()) {
        for (const auto& node : *setIt) {
            if (!node.is_object()) continue;
            SetBonus bonus;
            bonus.setId = node.value("setId", 0);
            if (bonus.setId == 0) continue;
            bonus.name = node.value("name", std::string("套装"));
            const auto bonusesIt = node.find("bonuses");
            if (bonusesIt != node.end() && bonusesIt->is_array()) {
                for (const auto& b : *bonusesIt) {
                    if (!b.is_object()) continue;
                    SetBonus::Bonus entry;
                    entry.pieceCount = b.value("pieceCount", 2);
                    entry.desc = b.value("desc", std::string());
                    entry.skillId = b.value("skillId", 0);
                    const auto applyAttrIt = b.find("applyAttr");
                    if (applyAttrIt != b.end() && applyAttrIt->is_object()) {
                        entry.applyAttr = ParseBattleAttr(*applyAttrIt);
                    }
                    bonus.bonuses.push_back(entry);
                }
            }
            REG_SET_BONUS(bonus);
        }
    }

    return true;
}

bool ItemConfig::loadFromFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in.is_open()) {
        LOG_ERROR("ItemConfig: 无法打开文件 %s", path.c_str());
        return false;
    }

    json root;
    try {
        in >> root;
    } catch (const std::exception& ex) {
        LOG_ERROR("ItemConfig: 解析 JSON 失败: %s", ex.what());
        return false;
    }
    clear();
    return loadFromJson(root);
}