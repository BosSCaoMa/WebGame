#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <array>
// ==================== 目标类型 ====================
enum class TargetType {
    // 基础阵营类型（保留原有核心类型，统一命名风格）
    SELF = 0,               // 自己
    ALLY_ALL = 1,           // 己方全体（替代原ALLY）
    ENEMY_SINGLE = 2,       // 敌方单个目标（同位置）
    ENEMY_COL = 3,          // 敌方一列
    ENEMY_ALL = 4,          // 敌方全体（保留原ALL_ENEMY，统一命名）

    // 按站位区分（前/后排）
    ALLY_FRONT_ROW = 5,     // 己方前排
    ALLY_BACK_ROW = 6,      // 己方后排
    ENEMY_FRONT_ROW = 7,    // 敌方前排
    ENEMY_BACK_ROW = 8,     // 敌方后排

    // 按攻击属性排序（攻击最高1/2/3人）
    ALLY_ATK_TOP1 = 9,      // 己方攻击最高1人
    ALLY_ATK_TOP2 = 10,     // 己方攻击最高2人
    ALLY_ATK_TOP3 = 11,     // 己方攻击最高3人
    ENEMY_ATK_TOP1 = 12,    // 敌方攻击最高1人
    ENEMY_ATK_TOP2 = 13,    // 敌方攻击最高2人
    ENEMY_ATK_TOP3 = 14,    // 敌方攻击最高3人

    // 按血量属性排序（血量最低1/2/3人）
    ALLY_HP_LOW1 = 15,      // 己方血量最低1人
    ALLY_HP_LOW2 = 16,      // 己方血量最低2人
    ALLY_HP_LOW3 = 17,      // 己方血量最低3人
    ENEMY_HP_LOW1 = 18,     // 敌方血量最低1人
    ENEMY_HP_LOW2 = 19,     // 敌方血量最低2人
    ENEMY_HP_LOW3 = 20,     // 敌方血量最低3人

    // 随机
    ALLY_RANDOM_1 = 21,   // 己方随机1人
    ALLY_RANDOM_2 = 22,   // 己方随机2人
    ALLY_RANDOM_3 = 23,   // 己方随机3人
    ENEMY_RANDOM_1 = 24,  // 敌方随机1人
    ENEMY_RANDOM_2 = 25,  // 敌方随机2人
    ENEMY_RANDOM_3 = 26   // 敌方随机3人
};

// ==================== 数值计算类型 ====================
enum class ValueType {
    FIXED = 0,          // 固定值
    PERCENT_ATK,        // 攻击力百分比
    PERCENT_DEF,        // 防御力百分比
    PERCENT_MAX_HP,     // 最大生命值百分比
    PERCENT_CUR_HP,     // 当前生命值百分比
    PERCENT_LOST_HP,    // 已损失生命值
    PERCENT_TARGET_MAXHP,  // 目标最大生命值百分比
};

enum class ValueSource {
    FIXED,      // 固定值
    ATK,        // 攻击力
    DEF,        // 防御力
    MAX_HP,     // 最大生命
    CUR_HP,     // 当前生命
    LOST_HP,    // 已损失生命
};

enum class ValueOwner {
    CASTER,     // 技能释放者
    TARGET,     // 技能目标
};

enum class ValueScale {
    ABSOLUTE,   // 绝对值
    PERCENT,    // 万分比
};

// ==================== 效果类型 ====================
// 控制类型和负面效果额外添加到kIntrinsicDebuffEffects和kControlEffects中
enum class EffectType {
    NONE = 0,
    
    // 伤害/治疗/怒气等即时效果
    DAMAGE,             // 伤害
    PIERCE,             // 穿透（伤害无视护盾/部分减伤）
    TRUE_DAMAGE,        // 真实伤害（无视防御/减伤/抗性，直接生效）
    HEAL,               // 治疗
    RAGE_CHANGE,           // 增加怒气
    DIVINE_POWER,          // 神威值(更高级的怒气机制，提供额外效果)

    // 护盾
    SHIELD,             // 护盾
    BARRIER,            // 屏障（免疫单次控制效果，新增）
    
    // 属性Buff
    BUFF_MAX_HP,        // 最大生命值加成（%）
    BUFF_ATK,           // 攻击力加成（%）
    BUFF_DEF,           // 防御力加成（%）
    BUFF_SPEED,         // 速度加成（%）
    BUFF_CRIT_RATE,     // 暴击率加成（%）
    BUFF_CRIT_RESIST,   // 抗暴率（%）
    BUFF_HIT_RATE,      // 命中率加成（%）
    BUFF_DODGE_RATE,    // 闪避率加成（%）

    // 持续BUFF
    BUFF_REGEN,         // 持续回血

    // ================负面效果开始====================
    // 控制
    STUN,               // 眩晕（无法行动）
    FREEZE,             // 冰冻（无法行动，受到伤害解除）
    SILENCE,            // 沉默（无法释放技能，只能普攻）
    TAUNT,              // 嘲讽（强制攻击释放者）
    INJURY,             // 受伤（无法回血）
    
    // DOT（持续伤害）
    POISON,             // 中毒(越来越深、不断增加)
    BURN,               // 灼烧(减缓速度)
    BLEED,              // 流血(直接损失体力值)
    CURSE,              // 诅咒（持续损失怒气+少量生命值）
    // ================负面效果结束=====================

    // 特殊
    LOCK_BLEED,         // 锁血（生命值不会低于1）
    REVIVE,             // 复活
    DISPEL,             // 驱散（移除敌方Buff）
    CLEANSE,            // 净化（移除己方Debuff）
    IMMUNITY,           // 免疫控制
    INVINCIBLE,         // 无敌
    TRANSFER_DEBUFF,    // Debuff转移（将自身Debuff转给敌方）

    // 标记类（高级机制）
    MARK_HEAL,          // 治疗标记（受到治疗时额外治疗）
    MARK_DAMAGE,        // 伤害标记（受到伤害时额外伤害）
    MARK_KILL,          // 斩杀标记（血量低于阈值时被秒杀）
    MARK_PROTECT,       // 保护标记（友方攻击时，优先攻击标记持有者）
};

template <std::size_t N>
constexpr bool ContainsEffect(const std::array<EffectType, N>& bucket, EffectType type)
{
    for (const auto entry : bucket) {
        if (entry == type) {
            return true;
        }
    }
    return false;
}

constexpr auto kIntrinsicDebuffEffects = std::array{
    EffectType::STUN,
    EffectType::FREEZE,
    EffectType::SILENCE,
    EffectType::TAUNT,
    EffectType::INJURY,
    EffectType::POISON,
    EffectType::BURN,
    EffectType::BLEED,
    EffectType::CURSE,
};

constexpr auto kControlEffects = std::array{
    EffectType::STUN,
    EffectType::FREEZE,
    EffectType::SILENCE,
    EffectType::TAUNT,
    EffectType::INJURY,
};

inline bool IsIntrinsicDebuff(EffectType type)
{
    return ContainsEffect(kIntrinsicDebuffEffects, type);
}

inline bool IsDebuff(EffectType type, int64_t value)
{
    if (IsIntrinsicDebuff(type)) {
        return true;
    }
    return value < 0;
}

inline bool IsControlEffect(EffectType type)
{
    return ContainsEffect(kControlEffects, type);
}

// ==================== 技能触发时机 ====================
enum class SkillTrigger
{
    NORMAL_ATTACK, // 普通攻击
    RAGE_SKILL,    // 怒气技能
    ON_SAME_CAMP, // 合击技能

    // 战斗阶段触发
    BATTLE_START,   // 开局技能
    ROUND_START,    // 回合开始【高品质武将技能】
    ROUND_END,      // 回合结束【高品质武将技能】
    TURN_START,     // 自身行动开始
    TURN_END,       // 自身行动结束
    ROUND_ODD,      // 奇数回合
    ROUND_EVEN,     // 偶数回合
    ROUND_MULTIPLE_OF_FIVE,     // 每5回合
    ROUND_TEN,      // 第10回合

    // 受击/伤害相关触发
    ON_HIT,          // 受到攻击时
    ON_CONTROL,      // 受到控制时
    ON_KILL,         // 击杀敌人
    ON_DEATH,  // 阵亡技能

    None          // 无触发
};

enum class QualityType {
    WHITE = 1,      // 白色 - 普通
    GREEN = 2,      // 绿色 - 优秀
    BLUE = 3,       // 蓝色 - 精良
    PURPLE = 4,     // 紫色 - 史诗
    ORANGE = 5,     // 橙色 - 传说
    RED = 6,        // 红色 - 神话
    GOLD = 7,       // 金色 - 传世
};

enum class Position {
    WARRIOR,    // 战士
    MAGE,   // 法师
    TANK,   // 坦克
    HEALER,  // 治疗
    ASSASSIN // 刺客
};






// 暂未用到
enum class PlayerAttrType {
    STAMINA = 1, // 体力
    ENERGY = 2, // 精力
    EXP = 3    // 经验
};