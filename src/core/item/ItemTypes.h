#pragma once
// ==================== 物品类型 ====================
enum class ItemType {
    NONE = 0,
    CONSUMABLE,     // 消耗品
    MATERIAL,       // 材料
    TREASURE,       // 宝物
    EQUIPMENT,      // 装备
    CURRENCY,       // 货币类
};

enum class ConsumableType {
    NONE,          // 无
    HP_POTION,      // 生命药水
    RAGE_POTION,    // 怒气药水
    EXP_POTION,     // 经验药水
    BUFF_POTION,    // 属性药水
};

// ==================== 材料类型 ====================
enum class MaterialType {
    NONE = 0,
    CRAFT,          // 通用打造材料
    UPGRADE,        // 升阶/强化材料
    QUEST,          // 任务道具
    EVENT           // 活动限定材料
};

// ==================== 货币类型 ====================
enum class CurrencyType {
    UNKNOWN = 0,
    SOFT,           // 常规金币/铜钱
    PREMIUM,        // 付费钻石
    BOUND_PREMIUM,  // 绑定钻石
    TOKEN           // 活动代币/积分
};

// ==================== 装备类型 ====================
enum class EquipmentType {
    WEAPON = 0,     // 武器
    ARMOR,          // 盔甲
    HELMET,         // 头盔
    BOOTS,          // 战靴

    STEED,         // 坐骑
    TALLY,         // 兵符
    TREASURE,      // 法宝
    FAMOUS,        // 名将
    COUNT           // 装备栏数量
};

// ==================== 装备词缀类型 ====================
enum class AffixType {
    NONE = 0,
    
    // 基础属性
    ATK_FLAT,           // 攻击力 +X
    ATK_PERCENT,        // 攻击力 +X%
    DEF_FLAT,           // 防御力 +X
    DEF_PERCENT,        // 防御力 +X%
    HP_FLAT,            // 生命值 +X
    HP_PERCENT,         // 生命值 +X%
    SPEED_FLAT,         // 速度 +X
    
    // 战斗属性
    CRIT_RATE,          // 暴击率 +X%
    CRIT_DAMAGE,        // 暴击伤害 +X%
    HIT_RATE,           // 命中率 +X%
    DODGE_RATE,         // 闪避率 +X%
    
    // 伤害加成
    DAMAGE_BONUS,       // 伤害加成 +X%
    DAMAGE_REDUCTION,   // 伤害减免 +X%
    SKILL_DAMAGE,       // 技能伤害 +X%
    
    // 特殊效果 todo实现到战斗
    LIFESTEAL,          // 吸血 +X%
    COUNTER_RATE,       // 反击率 +X%
    HEAL_BONUS,         // 治疗加成 +X%
    MULTI_HIT_RATE,      // 连击率 +X%
    
    // 抗性
    STUN_RESIST,        // 眩晕抗性 +X%
    FREEZE_RESIST,      // 冰冻抗性 +X%
    SILENCE_RESIST,     // 沉默抗性 +X%
    TAUNT_RESIST,       // 嘲讽抗性 +X%
    INJURY_RESIST,      // 受伤抗性 +X%
    BLEED_RESIST,       // 流血抗性 +X%
    CURSE_RESIST,       // 诅咒抗性 +X%
    BURN_RESIST,        // 灼烧抗性 +X%
    POISON_RESIST,      // 中毒抗性 +X%
};
