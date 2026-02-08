#pragma once

#include "ItemTypes.h"
#include "BattleTypes.h"
#include "BattleAttr.h"
#include <string>

// ==================== 装备词缀 ====================
struct EquipmentAffix { // 暂不使用
    AffixType type;
    int64_t value;
    
    EquipmentAffix() : type(AffixType::NONE), value(0) {}
    EquipmentAffix(AffixType t, int64_t v) : type(t), value(v) {}
    
    std::string getDescription() const {return "";}; // 获取词缀描述
};

// ==================== 装备 ====================
struct Equipment {
    int id;
    std::string name;
    EquipmentType type;
    QualityType quality;
    
    // 词缀(主词缀+副词缀-随机生成)
    // std::vector<EquipmentAffix> mainAffixs;
    // std::vector<EquipmentAffix> subAffixes;
    BattleAttr baseAttrs; // 装备基础属性加成
    int skillId;  // 装备携带的技能ID
    
    int setId; // 所属套装ID，0表示不属于任何套装
    
    Equipment() = default;
    
    Equipment(int id_, const std::string& name_, EquipmentType type_, 
              QualityType quality_, int skill = 0, int setId_ = 0)
        : id(id_), name(name_), type(type_), quality(quality_),
            skillId(skill), setId(setId_) {}

    bool hasSkill() const { return skillId != 0; }

    std::string GetDescription() const {return name;} // NTODO 完善描述;
};

// ==================== 套装效果 ====================
struct SetBonus {
    int setId;
    std::string name;
    
    struct Bonus {
        int pieceCount;     // 需要件数
        std::string desc;
        int skillId;        // 套装技能ID（可选）
        BattleAttr applyAttr; // 套装属性加成
    };
    
    std::vector<Bonus> bonuses;  // 2件套、4件套、6件套等
};
