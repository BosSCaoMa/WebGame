#pragma once
#include "BattleTypes.h"

/*
    技能效果：目标类型、效果类型、数值类型、数值、持续时间、触发概率、是否可叠加
*/
struct ValueExpr {
    ValueSource source = ValueSource::FIXED;
    ValueOwner  owner  = ValueOwner::CASTER;
    ValueScale  scale  = ValueScale::ABSOLUTE;
    int64_t     value  = 1;   // ABSOLUTE: 实际值
                              // PERCENT: 万分比
    ValueExpr() = default;
    static ValueExpr Fixed(int64_t v);
    static ValueExpr Percent_Atk(ValueSource src, ValueOwner owner, int64_t percent);
    static ValueExpr Percent(ValueSource src, ValueOwner owner, int64_t percent);
};

struct SkillEffect {
    TargetType target;
    EffectType effect;
    ValueExpr valueExpr;

    int duration;       // 持续时间: 0 = 即时
    int chance;         // 触发概率: 100 = 100%
    bool canOverlay;     // 是否可叠加（同类型Buff是否覆盖或叠加）
    
    SkillEffect();
    SkillEffect(TargetType t, EffectType e, ValueExpr ve, int dur = 0, int ch = 100, bool overlay = false);

    // 常用效果工厂


private:
    static SkillEffect Make(TargetType target, EffectType effect, ValueExpr expr, int duration, int chance, bool overlay);
};