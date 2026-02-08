#pragma once

#include "equip.h"
#include <unordered_map>
#include <vector>

// ==================== 物品模板 ====================
struct ItemTemplate {
    int id;
    std::string name;
    ItemType type;
    ConsumableType subType;
    int quality;
    std::string description;
    int maxStack;
    
    // 消耗品效果
    struct Effect {
        EffectType type;
        int64_t value;
    };
    std::vector<Effect> effects;
    
    ItemTemplate() = default;
    ItemTemplate(int id, const std::string& name_, ItemType type_, 
        ConsumableType subType_, int quality_, int maxStack_ = 1)
        : id(id), name(name_), type(type_), subType(subType_)
        , quality(quality_), maxStack(maxStack_) {}
};

// ==================== 装备模板 ====================
// 当前与Equipment基本相同，但是后续可能添加一些培养属性
struct EquipmentTemplate {
    int id;
    std::string name;
    EquipmentType type;
    QualityType quality;
    BattleAttr baseAttrs;

    int skillId;
    int setId;
    
    EquipmentTemplate() = default;
    EquipmentTemplate(int id, const std::string& name_, EquipmentType type_, 
        QualityType quality_, int skillId_ = 0, int setId_ = 0)
        : id(id), name(name_), type(type_), quality(quality_)
        , skillId(skillId_), setId(setId_) {}
};

// ==================== 物品配置管理器 ====================
class ItemConfig {
private:
    std::unordered_map<int, ItemTemplate> items_;
    std::unordered_map<int, EquipmentTemplate> equipments_;
    std::unordered_map<int, SetBonus> setBonuses_;
public:
    using ET = EquipmentType;
    using EQ = QualityType;
    using AT = AffixType;

    static ItemConfig& instance() {
        static ItemConfig inst;
        return inst;
    }
    
    // 获取物品模板
    const ItemTemplate* getItem(int itemId) const {
        auto it = items_.find(itemId);
        return it != items_.end() ? &it->second : nullptr;
    }
    
    // 获取装备模板
    const EquipmentTemplate* getEquipment(int equipId) const {
        auto it = equipments_.find(equipId);
        return it != equipments_.end() ? &it->second : nullptr;
    }
    
    // 创建装备实例（随机词缀）
    Equipment createEquipment(int equipId) const;
    
    // 注册
    void regItem(const ItemTemplate& tmpl) { items_[tmpl.id] = tmpl; }
    void regEquipment(const EquipmentTemplate& tmpl) { equipments_[tmpl.id] = tmpl; }
    void regSetBonus(const SetBonus& bonus) { setBonuses_[bonus.setId] = bonus; }
    
    // 获取套装加成
    const SetBonus* getSetBonus(int setId) const {
        auto it = setBonuses_.find(setId);
        return it != setBonuses_.end() ? &it->second : nullptr;
    }

private:
    ItemConfig() { init(); }
    ItemConfig(const ItemConfig&) = delete;
    ItemConfig& operator=(const ItemConfig&) = delete;

    void init();
    // void initConsumables();
    // void initMaterials();
    // void initEquipments();
    // void initSetBonuses();

    // // 初始化装备
    // void initWeapons();
    // void initArmors();
    // void initHelmets();
    // void initBoots();
    // void initSteeds();
    // void initTallys();
    // void initTreasures();
    // void initFamouss();
};

#define GET_ITEM(id) ItemConfig::instance().getItem(id)
#define CREATE_EQUIP(id) ItemConfig::instance().createEquipment(id)
