#include "ItemConfig.h"
#include <algorithm>
#include <random>
#include "LogM.h"
#include "json.hpp"
#include <fstream>
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

const std::unordered_map<std::string, MaterialType> kMaterialTypeMap = {
    {"NONE", MaterialType::NONE},
    {"CRAFT", MaterialType::CRAFT},
    {"UPGRADE", MaterialType::UPGRADE},
    {"QUEST", MaterialType::QUEST},
    {"EVENT", MaterialType::EVENT}
};

const std::unordered_map<std::string, CurrencyType> kCurrencyTypeMap = {
    {"UNKNOWN", CurrencyType::UNKNOWN},
    {"SOFT", CurrencyType::SOFT},
    {"PREMIUM", CurrencyType::PREMIUM},
    {"BOUND_PREMIUM", CurrencyType::BOUND_PREMIUM},
    {"TOKEN", CurrencyType::TOKEN}
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

const std::unordered_map<std::string, ValueOwner> kValueOwnerMap = {
    {"CASTER", ValueOwner::CASTER},
    {"TARGET", ValueOwner::TARGET}
};

const std::unordered_map<std::string, ValueScale> kValueScaleMap = {
    {"ABSOLUTE", ValueScale::ABSOLUTE},
    {"PERCENT", ValueScale::PERCENT}
};

// 小工具：如子节点存在并为对象就返回指针，便于可选配置块读取
const json* SelectChildObject(const json& node, const char* key)
{
    const auto it = node.find(key);
    if (it != node.end() && it->is_object()) {
        return &(*it);
    }
    return nullptr;
}

// 统一处理单个效果的解析，兼容缺省字段
ItemEffect ParseItemEffect(const json& node)
{
    ItemEffect effect;
    const std::string typeStr = node.value("type", std::string("NONE"));
    effect.type = LookupEnum(kEffectTypeMap, typeStr, EffectType::NONE, "EffectType");
    effect.magnitude = node.value("value", static_cast<int64_t>(effect.magnitude));
    const std::string ownerStr = node.value("owner", std::string("TARGET"));
    effect.owner = LookupEnum(kValueOwnerMap, ownerStr, ValueOwner::TARGET, "ValueOwner");
    const std::string scaleStr = node.value("scale", std::string("ABSOLUTE"));
    effect.scale = LookupEnum(kValueScaleMap, scaleStr, ValueScale::ABSOLUTE, "ValueScale");
    effect.durationTurn = node.value("duration", effect.durationTurn);
    return effect;
}

// 支持旧版 maxStack 字段，以及新版 stack 对象的更多控制位
void ApplyStackingRule(const json& node, ItemTemplate& item)
{
    item.meta.stack.maxStack = node.value("maxStack", item.meta.stack.maxStack);
    if (const auto* stackObj = SelectChildObject(node, "stack")) {
        item.meta.stack.maxStack = stackObj->value("max", item.meta.stack.maxStack);
        item.meta.stack.unique = stackObj->value("unique", item.meta.stack.unique);
        item.meta.stack.autoSplit = stackObj->value("autoSplit", item.meta.stack.autoSplit);
    }
}

// 扁平字段或 ownership 块都可以配置绑定策略
void ApplyOwnershipRule(const json& node, ItemTemplate& item)
{
    if (const auto* ownershipObj = SelectChildObject(node, "ownership")) {
        item.meta.ownership.tradable = ownershipObj->value("tradable", item.meta.ownership.tradable);
        item.meta.ownership.bindOnPickup = ownershipObj->value("bindOnPickup", item.meta.ownership.bindOnPickup);
        item.meta.ownership.bindOnEquip = ownershipObj->value("bindOnEquip", item.meta.ownership.bindOnEquip);
        return;
    }
    item.meta.ownership.tradable = node.value("tradable", item.meta.ownership.tradable);
    item.meta.ownership.bindOnPickup = node.value("bindOnPickup", item.meta.ownership.bindOnPickup);
    item.meta.ownership.bindOnEquip = node.value("bindOnEquip", item.meta.ownership.bindOnEquip);
}

// 将 tags 数组注入索引用于分类检索
void ApplyTags(const json& node, ItemTemplate& item)
{
    const auto tagsIt = node.find("tags");
    if (tagsIt == node.end() || !tagsIt->is_array()) {
        return;
    }
    for (const auto& tagNode : *tagsIt) {
        if (tagNode.is_string()) {
            item.meta.tags.push_back(tagNode.get<std::string>());
        }
    }
}

// 允许质量值写成字符串（WHITE）或数值（1）
QualityType ParseQuality(const json& node)
{
    const auto it = node.find("quality");
    if (it != node.end()) {
        if (it->is_string()) {
            return LookupEnum(kQualityMap, it->get<std::string>(), QualityType::WHITE, "QualityType");
        }
        if (it->is_number_integer()) {
            const int raw = it->get<int>();
            if (raw >= static_cast<int>(QualityType::WHITE) && raw <= static_cast<int>(QualityType::GOLD)) {
                return static_cast<QualityType>(raw);
            }
        }
    }
    const std::string qualityStr = node.value("quality", std::string("WHITE"));
    return LookupEnum(kQualityMap, qualityStr, QualityType::WHITE, "QualityType");
}

// 消耗品载荷：优先读取 consumable 块，没有则回退到旧字段
void ParseConsumablePayload(const json& node, ItemTemplate& item)
{
    ConsumableProfile profile;
    std::string categoryStr = node.value("subType", std::string("NONE"));
    const json* payloadNode = SelectChildObject(node, "consumable");
    if (payloadNode) {
        categoryStr = payloadNode->value("category", categoryStr);
    } else {
        payloadNode = &node;
    }
    profile.category = LookupEnum(kConsumableMap, categoryStr, ConsumableType::NONE, "ConsumableType");
    profile.cooldownSec = payloadNode->value("cooldownSec", profile.cooldownSec);
    profile.destroyOnUse = payloadNode->value("destroyOnUse", profile.destroyOnUse);

    const auto effectsIt = payloadNode->find("effects");
    if (effectsIt != payloadNode->end() && effectsIt->is_array()) {
        for (const auto& eff : *effectsIt) {
            if (!eff.is_object()) {
                continue;
            }
            profile.effects.push_back(ParseItemEffect(eff));
        }
    } else if (const auto fallbackIt = node.find("effects"); fallbackIt != node.end() && fallbackIt->is_array()) {
        for (const auto& eff : *fallbackIt) {
            if (!eff.is_object()) {
                continue;
            }
            profile.effects.push_back(ParseItemEffect(eff));
        }
    }

    item.payload = std::move(profile);
}

// 材料与宝物共享 MaterialProfile，tier/usage 可选
void ParseMaterialPayload(const json& node, ItemTemplate& item)
{
    MaterialProfile profile;
    const json* payloadNode = SelectChildObject(node, "material");
    if (!payloadNode) {
        payloadNode = &node;
    }
    const std::string typeStr = payloadNode->value("category", node.value("materialType", std::string("NONE")));
    profile.category = LookupEnum(kMaterialTypeMap, typeStr, MaterialType::NONE, "MaterialType");
    profile.tier = payloadNode->value("tier", profile.tier);
    profile.usage = payloadNode->value("usage", profile.usage);
    item.payload = std::move(profile);
}

// 货币载荷提供精度与跨角色共享等开关
void ParseCurrencyPayload(const json& node, ItemTemplate& item)
{
    CurrencyProfile profile;
    const json* payloadNode = SelectChildObject(node, "currency");
    if (!payloadNode) {
        payloadNode = &node;
    }
    const std::string typeStr = payloadNode->value("category", std::string("UNKNOWN"));
    profile.category = LookupEnum(kCurrencyTypeMap, typeStr, CurrencyType::UNKNOWN, "CurrencyType");
    profile.precision = payloadNode->value("precision", profile.precision);
    profile.sharedAcrossRoles = payloadNode->value("sharedAcrossRoles", profile.sharedAcrossRoles);
    profile.allowNegative = payloadNode->value("allowNegative", profile.allowNegative);
    item.payload = std::move(profile);
}

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

bool loadFromJson(const json& root)
{
    const auto itemsIt = root.find("items");
    if (itemsIt != root.end() && itemsIt->is_array()) {
        for (const auto& node : *itemsIt) {
            if (!node.is_object()) {
                continue;
            }
            ItemTemplate item;
            item.meta.id = node.value("id", 0);
            if (item.meta.id == 0) {
                LOG_ERROR("ItemConfig: 忽略 id=0 的物品。");
                continue;
            }
            item.meta.name = node.value("name", std::string("未命名物品"));
            item.meta.description = node.value("description", std::string());
            const std::string typeStr = node.value("type", std::string("NONE"));
            item.meta.type = LookupEnum(kItemTypeMap, typeStr, ItemType::NONE, "ItemType");
            item.meta.quality = ParseQuality(node);
            ApplyStackingRule(node, item);
            ApplyOwnershipRule(node, item);
            ApplyTags(node, item);

            switch (item.meta.type) {
            case ItemType::CONSUMABLE:
                ParseConsumablePayload(node, item);
                break;
            case ItemType::MATERIAL:
            case ItemType::TREASURE:
                ParseMaterialPayload(node, item);
                break;
            case ItemType::CURRENCY:
                ParseCurrencyPayload(node, item);
                break;
            default:
                break;
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

} // namespace

void ItemConfig::init()
{
    // TODO: load JSON config files; keep no-op for now.
}

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
    return loadFromJson(root);
}

void ItemConfig::regItem(const ItemTemplate& tmpl)
{
    const int id = tmpl.meta.id;
    auto it = items_.find(id);
    if (it != items_.end()) {
        removeFromIndex(it->second);
        it->second = tmpl;
        addToIndex(it->second);
        return;
    }
    auto [inserted, _] = items_.emplace(id, tmpl);
    addToIndex(inserted->second);
}

std::vector<const ItemTemplate*> ItemConfig::listItemsByType(ItemType type) const
{
    std::vector<const ItemTemplate*> result;
    auto it = typeIndex_.find(type);
    if (it == typeIndex_.end()) {
        return result;
    }
    result.reserve(it->second.size());
    for (int itemId : it->second) {
        if (const auto* tmpl = getItem(itemId)) {
            result.push_back(tmpl);
        }
    }
    return result;
}

std::vector<const ItemTemplate*> ItemConfig::findItemsByTag(const std::string& tag) const
{
    std::vector<const ItemTemplate*> result;
    auto it = tagIndex_.find(tag);
    if (it == tagIndex_.end()) {
        return result;
    }
    result.reserve(it->second.size());
    for (int itemId : it->second) {
        if (const auto* tmpl = getItem(itemId)) {
            result.push_back(tmpl);
        }
    }
    return result;
}

void ItemConfig::addToIndex(const ItemTemplate& tmpl)
{
    typeIndex_[tmpl.meta.type].push_back(tmpl.meta.id);
    for (const auto& tag : tmpl.meta.tags) {
        tagIndex_[tag].push_back(tmpl.meta.id);
    }
}

void ItemConfig::removeFromIndex(const ItemTemplate& tmpl)
{
    if (auto it = typeIndex_.find(tmpl.meta.type); it != typeIndex_.end()) {
        RemoveId(it->second, tmpl.meta.id);
        if (it->second.empty()) {
            typeIndex_.erase(it);
        }
    }
    for (const auto& tag : tmpl.meta.tags) {
        auto tagIt = tagIndex_.find(tag);
        if (tagIt == tagIndex_.end()) {
            continue;
        }
        RemoveId(tagIt->second, tmpl.meta.id);
        if (tagIt->second.empty()) {
            tagIndex_.erase(tagIt);
        }
    }
}

void ItemConfig::RemoveId(std::vector<int>& bucket, int id)
{
    auto it = std::remove(bucket.begin(), bucket.end(), id);
    bucket.erase(it, bucket.end());
}