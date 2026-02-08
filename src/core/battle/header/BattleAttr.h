#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "BattleTypes.h"

// ==================== 战斗属性（使用明确命名，避免魔法索引） ====================
struct BattleAttr final {   // 相互关联 【 EffectType applyEffect 】【 BattleAttr 】
    using Scalar = std::int64_t;
    using Rate = std::int16_t;

    enum class Resistance : std::uint8_t {
        Burn, // 灼烧
        Freeze, // 冰冻
        Stun, // 眩晕
        Silence, // 沉默
        Poison, // 中毒
        Taunt, // 嘲讽
        Injury, // 受伤
        Bleed, // 流血
        Curse, // 诅咒
        Count
    };

    // 基础属性
    Scalar hp{10000};
    Scalar maxHp{10000};
    Scalar atk{1000};
    Scalar def{100};
    Scalar speed{300};
    
    // 战斗属性
    Rate critRate{0};       // 暴击率 (百分比)
    Rate critDamage{150};   // 暴击伤害 (默认150%)
    Rate critResist{0};     // 暴击抗性 (百分比)
    Rate hitRate{100};      // 命中率 (默认100%)
    Rate dodgeRate{0};      // 闪避率
    Rate rage{2};           // 基础怒气值

    // ========== 扩展属性（可选） ==========
    Rate damageBonus{0};        // 伤害加成 (百分比)
    Rate damageReduction{0};    // 伤害减免 (百分比)
    Rate skillDamage{0};        // 技能伤害加成 (百分比)
    
    Rate lifesteal{0};          // 吸血 (百分比)
    Rate counterRate{0};        // 反击率 (百分比)
    Rate multiHitRate{0};       // 连击概率 (百分比)
    Rate healBonus{0};          // 治疗加成 (百分比)
    
    BattleAttr() = default;
    constexpr BattleAttr(Scalar hp_, Scalar atk_, Scalar def_, Scalar speed_) noexcept
        : hp(hp_), maxHp(hp_), atk(atk_), def(def_), speed(speed_) {} // 编译器常量，如果传入的参数也是常量的话，可以在编译时计算结果
    
    // 属性相加
    BattleAttr& operator+=(const BattleAttr& other);
    
    void upgradeByLevel(int level); // 根据等级提升属性
    std::uint64_t calculateCombatPower() const; // 计算战力值
    void InitAttr(Position pos, QualityType quality); // 根据定位初始化属性

    [[nodiscard]] Rate getResistance(Resistance type) const noexcept {
        return resistances_[resistanceIndex(type)];
    }

    Rate& resistance(Resistance type) noexcept {
        return resistances_[resistanceIndex(type)];
    }

    void setResistance(Resistance type, Rate value) noexcept {
        resistance(type) = value;
    }

private:
    void ExPandByQuality(QualityType quality); // 根据品质扩展属性
    static BattleAttr warriorAttrs(); // 战士属性模板
    static BattleAttr mageAttrs();   // 法师属性模板
    static BattleAttr tankAttrs();  // 坦克属性模板
    static BattleAttr healerAttrs(); // 辅助属性模板
    static BattleAttr assassinAttrs(); // 刺客属性模板

    static constexpr std::size_t resistanceIndex(Resistance type) noexcept {
        return static_cast<std::size_t>(type);
    }

    std::array<Rate, static_cast<std::size_t>(Resistance::Count)> resistances_{}; // 控制抗性集合
};

