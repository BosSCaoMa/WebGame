#include "SkillEffect.h"


ValueExpr ValueExpr::Fixed(int64_t v) {
    ValueExpr expr;
    expr.value = v;
    return expr;
}

ValueExpr ValueExpr::Percent_Atk(ValueSource src, ValueOwner owner, int64_t percent) {
    ValueExpr expr;
    expr.source = src;
    expr.owner = owner;
    expr.scale = ValueScale::PERCENT;
    expr.value = percent;
    return expr;
}

ValueExpr ValueExpr::Percent(ValueSource src, ValueOwner owner, int64_t percent) {
    ValueExpr expr;
    expr.source = src;
    expr.owner = owner;
    expr.scale = ValueScale::PERCENT;
    expr.value = percent;
    return expr;
}

SkillEffect::SkillEffect()
    : target(TargetType::SELF),
      effect(EffectType::NONE),
      valueExpr(ValueExpr::Fixed(0)),
      duration(0),
      chance(100),
      canOverlay(false) {}

SkillEffect::SkillEffect(TargetType t, EffectType e, ValueExpr ve, int dur, int ch, bool overlay)
    : SkillEffect() {
    target = t;
    effect = e;
    duration = dur;
    chance = ch;
    canOverlay = overlay;
    valueExpr = ve;
}