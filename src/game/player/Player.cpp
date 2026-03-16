#include "Player.h"
#include "ShopService.h"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>

#include "player.pb.h"

namespace pb = webgame::player::v1;

namespace {

constexpr uint32_t kPlayerSchemaVersion = 1;

pb::QualityType toPbQuality(QualityType value)
{
    switch (value) {
        case QualityType::WHITE: return pb::QUALITY_TYPE_WHITE;
        case QualityType::GREEN: return pb::QUALITY_TYPE_GREEN;
        case QualityType::BLUE: return pb::QUALITY_TYPE_BLUE;
        case QualityType::PURPLE: return pb::QUALITY_TYPE_PURPLE;
        case QualityType::ORANGE: return pb::QUALITY_TYPE_ORANGE;
        case QualityType::RED: return pb::QUALITY_TYPE_RED;
        case QualityType::GOLD: return pb::QUALITY_TYPE_GOLD;
        default: return pb::QUALITY_TYPE_UNSPECIFIED;
    }
}

QualityType fromPbQuality(pb::QualityType value)
{
    switch (value) {
        case pb::QUALITY_TYPE_GREEN: return QualityType::GREEN;
        case pb::QUALITY_TYPE_BLUE: return QualityType::BLUE;
        case pb::QUALITY_TYPE_PURPLE: return QualityType::PURPLE;
        case pb::QUALITY_TYPE_ORANGE: return QualityType::ORANGE;
        case pb::QUALITY_TYPE_RED: return QualityType::RED;
        case pb::QUALITY_TYPE_GOLD: return QualityType::GOLD;
        case pb::QUALITY_TYPE_WHITE:
        default: return QualityType::WHITE;
    }
}

pb::Position toPbPosition(Position value)
{
    switch (value) {
        case Position::WARRIOR: return pb::POSITION_WARRIOR;
        case Position::MAGE: return pb::POSITION_MAGE;
        case Position::TANK: return pb::POSITION_TANK;
        case Position::HEALER: return pb::POSITION_HEALER;
        case Position::ASSASSIN: return pb::POSITION_ASSASSIN;
        default: return pb::POSITION_UNSPECIFIED;
    }
}

Position fromPbPosition(pb::Position value)
{
    switch (value) {
        case pb::POSITION_MAGE: return Position::MAGE;
        case pb::POSITION_TANK: return Position::TANK;
        case pb::POSITION_HEALER: return Position::HEALER;
        case pb::POSITION_ASSASSIN: return Position::ASSASSIN;
        case pb::POSITION_WARRIOR:
        default: return Position::WARRIOR;
    }
}

pb::SkillTrigger toPbSkillTrigger(SkillTrigger value)
{
    switch (value) {
        case SkillTrigger::NORMAL_ATTACK: return pb::SKILL_TRIGGER_NORMAL_ATTACK;
        case SkillTrigger::RAGE_SKILL: return pb::SKILL_TRIGGER_RAGE_SKILL;
        case SkillTrigger::ON_SAME_CAMP: return pb::SKILL_TRIGGER_ON_SAME_CAMP;
        case SkillTrigger::BATTLE_START: return pb::SKILL_TRIGGER_BATTLE_START;
        case SkillTrigger::ROUND_START: return pb::SKILL_TRIGGER_ROUND_START;
        case SkillTrigger::ROUND_END: return pb::SKILL_TRIGGER_ROUND_END;
        case SkillTrigger::TURN_START: return pb::SKILL_TRIGGER_TURN_START;
        case SkillTrigger::TURN_END: return pb::SKILL_TRIGGER_TURN_END;
        case SkillTrigger::ROUND_ODD: return pb::SKILL_TRIGGER_ROUND_ODD;
        case SkillTrigger::ROUND_EVEN: return pb::SKILL_TRIGGER_ROUND_EVEN;
        case SkillTrigger::ROUND_MULTIPLE_OF_FIVE: return pb::SKILL_TRIGGER_ROUND_MULTIPLE_OF_FIVE;
        case SkillTrigger::ROUND_TEN: return pb::SKILL_TRIGGER_ROUND_TEN;
        case SkillTrigger::ON_HIT: return pb::SKILL_TRIGGER_ON_HIT;
        case SkillTrigger::ON_CONTROL: return pb::SKILL_TRIGGER_ON_CONTROL;
        case SkillTrigger::ON_KILL: return pb::SKILL_TRIGGER_ON_KILL;
        case SkillTrigger::ON_DEATH: return pb::SKILL_TRIGGER_ON_DEATH;
        case SkillTrigger::None:
        default:
            return pb::SKILL_TRIGGER_NONE;
    }
}

SkillTrigger fromPbSkillTrigger(pb::SkillTrigger value)
{
    switch (value) {
        case pb::SKILL_TRIGGER_NORMAL_ATTACK: return SkillTrigger::NORMAL_ATTACK;
        case pb::SKILL_TRIGGER_RAGE_SKILL: return SkillTrigger::RAGE_SKILL;
        case pb::SKILL_TRIGGER_ON_SAME_CAMP: return SkillTrigger::ON_SAME_CAMP;
        case pb::SKILL_TRIGGER_BATTLE_START: return SkillTrigger::BATTLE_START;
        case pb::SKILL_TRIGGER_ROUND_START: return SkillTrigger::ROUND_START;
        case pb::SKILL_TRIGGER_ROUND_END: return SkillTrigger::ROUND_END;
        case pb::SKILL_TRIGGER_TURN_START: return SkillTrigger::TURN_START;
        case pb::SKILL_TRIGGER_TURN_END: return SkillTrigger::TURN_END;
        case pb::SKILL_TRIGGER_ROUND_ODD: return SkillTrigger::ROUND_ODD;
        case pb::SKILL_TRIGGER_ROUND_EVEN: return SkillTrigger::ROUND_EVEN;
        case pb::SKILL_TRIGGER_ROUND_MULTIPLE_OF_FIVE: return SkillTrigger::ROUND_MULTIPLE_OF_FIVE;
        case pb::SKILL_TRIGGER_ROUND_TEN: return SkillTrigger::ROUND_TEN;
        case pb::SKILL_TRIGGER_ON_HIT: return SkillTrigger::ON_HIT;
        case pb::SKILL_TRIGGER_ON_CONTROL: return SkillTrigger::ON_CONTROL;
        case pb::SKILL_TRIGGER_ON_KILL: return SkillTrigger::ON_KILL;
        case pb::SKILL_TRIGGER_ON_DEATH: return SkillTrigger::ON_DEATH;
        case pb::SKILL_TRIGGER_NONE:
        default: return SkillTrigger::None;
    }
}

pb::EquipmentType toPbEquipmentType(EquipmentType value)
{
    switch (value) {
        case EquipmentType::WEAPON: return pb::EQUIPMENT_TYPE_WEAPON;
        case EquipmentType::ARMOR: return pb::EQUIPMENT_TYPE_ARMOR;
        case EquipmentType::HELMET: return pb::EQUIPMENT_TYPE_HELMET;
        case EquipmentType::BOOTS: return pb::EQUIPMENT_TYPE_BOOTS;
        case EquipmentType::STEED: return pb::EQUIPMENT_TYPE_STEED;
        case EquipmentType::TALLY: return pb::EQUIPMENT_TYPE_TALLY;
        case EquipmentType::TREASURE: return pb::EQUIPMENT_TYPE_TREASURE;
        case EquipmentType::FAMOUS: return pb::EQUIPMENT_TYPE_FAMOUS;
        default: return pb::EQUIPMENT_TYPE_UNSPECIFIED;
    }
}

EquipmentType fromPbEquipmentType(pb::EquipmentType value)
{
    switch (value) {
        case pb::EQUIPMENT_TYPE_ARMOR: return EquipmentType::ARMOR;
        case pb::EQUIPMENT_TYPE_HELMET: return EquipmentType::HELMET;
        case pb::EQUIPMENT_TYPE_BOOTS: return EquipmentType::BOOTS;
        case pb::EQUIPMENT_TYPE_STEED: return EquipmentType::STEED;
        case pb::EQUIPMENT_TYPE_TALLY: return EquipmentType::TALLY;
        case pb::EQUIPMENT_TYPE_TREASURE: return EquipmentType::TREASURE;
        case pb::EQUIPMENT_TYPE_FAMOUS: return EquipmentType::FAMOUS;
        case pb::EQUIPMENT_TYPE_WEAPON:
        default:
            return EquipmentType::WEAPON;
    }
}

pb::TargetType toPbTargetType(TargetType value)
{
    switch (value) {
        case TargetType::SELF: return pb::TARGET_TYPE_SELF;
        case TargetType::ALLY_ALL: return pb::TARGET_TYPE_ALLY_ALL;
        case TargetType::ENEMY_SINGLE: return pb::TARGET_TYPE_ENEMY_SINGLE;
        case TargetType::ENEMY_COL: return pb::TARGET_TYPE_ENEMY_COL;
        case TargetType::ENEMY_ALL: return pb::TARGET_TYPE_ENEMY_ALL;
        case TargetType::ALLY_FRONT_ROW: return pb::TARGET_TYPE_ALLY_FRONT_ROW;
        case TargetType::ALLY_BACK_ROW: return pb::TARGET_TYPE_ALLY_BACK_ROW;
        case TargetType::ENEMY_FRONT_ROW: return pb::TARGET_TYPE_ENEMY_FRONT_ROW;
        case TargetType::ENEMY_BACK_ROW: return pb::TARGET_TYPE_ENEMY_BACK_ROW;
        case TargetType::ALLY_ATK_TOP1: return pb::TARGET_TYPE_ALLY_ATK_TOP1;
        case TargetType::ALLY_ATK_TOP2: return pb::TARGET_TYPE_ALLY_ATK_TOP2;
        case TargetType::ALLY_ATK_TOP3: return pb::TARGET_TYPE_ALLY_ATK_TOP3;
        case TargetType::ENEMY_ATK_TOP1: return pb::TARGET_TYPE_ENEMY_ATK_TOP1;
        case TargetType::ENEMY_ATK_TOP2: return pb::TARGET_TYPE_ENEMY_ATK_TOP2;
        case TargetType::ENEMY_ATK_TOP3: return pb::TARGET_TYPE_ENEMY_ATK_TOP3;
        case TargetType::ALLY_HP_LOW1: return pb::TARGET_TYPE_ALLY_HP_LOW1;
        case TargetType::ALLY_HP_LOW2: return pb::TARGET_TYPE_ALLY_HP_LOW2;
        case TargetType::ALLY_HP_LOW3: return pb::TARGET_TYPE_ALLY_HP_LOW3;
        case TargetType::ENEMY_HP_LOW1: return pb::TARGET_TYPE_ENEMY_HP_LOW1;
        case TargetType::ENEMY_HP_LOW2: return pb::TARGET_TYPE_ENEMY_HP_LOW2;
        case TargetType::ENEMY_HP_LOW3: return pb::TARGET_TYPE_ENEMY_HP_LOW3;
        case TargetType::ALLY_RANDOM_1: return pb::TARGET_TYPE_ALLY_RANDOM_1;
        case TargetType::ALLY_RANDOM_2: return pb::TARGET_TYPE_ALLY_RANDOM_2;
        case TargetType::ALLY_RANDOM_3: return pb::TARGET_TYPE_ALLY_RANDOM_3;
        case TargetType::ENEMY_RANDOM_1: return pb::TARGET_TYPE_ENEMY_RANDOM_1;
        case TargetType::ENEMY_RANDOM_2: return pb::TARGET_TYPE_ENEMY_RANDOM_2;
        case TargetType::ENEMY_RANDOM_3: return pb::TARGET_TYPE_ENEMY_RANDOM_3;
        default: return pb::TARGET_TYPE_UNSPECIFIED;
    }
}

TargetType fromPbTargetType(pb::TargetType value)
{
    switch (value) {
        case pb::TARGET_TYPE_ALLY_ALL: return TargetType::ALLY_ALL;
        case pb::TARGET_TYPE_ENEMY_SINGLE: return TargetType::ENEMY_SINGLE;
        case pb::TARGET_TYPE_ENEMY_COL: return TargetType::ENEMY_COL;
        case pb::TARGET_TYPE_ENEMY_ALL: return TargetType::ENEMY_ALL;
        case pb::TARGET_TYPE_ALLY_FRONT_ROW: return TargetType::ALLY_FRONT_ROW;
        case pb::TARGET_TYPE_ALLY_BACK_ROW: return TargetType::ALLY_BACK_ROW;
        case pb::TARGET_TYPE_ENEMY_FRONT_ROW: return TargetType::ENEMY_FRONT_ROW;
        case pb::TARGET_TYPE_ENEMY_BACK_ROW: return TargetType::ENEMY_BACK_ROW;
        case pb::TARGET_TYPE_ALLY_ATK_TOP1: return TargetType::ALLY_ATK_TOP1;
        case pb::TARGET_TYPE_ALLY_ATK_TOP2: return TargetType::ALLY_ATK_TOP2;
        case pb::TARGET_TYPE_ALLY_ATK_TOP3: return TargetType::ALLY_ATK_TOP3;
        case pb::TARGET_TYPE_ENEMY_ATK_TOP1: return TargetType::ENEMY_ATK_TOP1;
        case pb::TARGET_TYPE_ENEMY_ATK_TOP2: return TargetType::ENEMY_ATK_TOP2;
        case pb::TARGET_TYPE_ENEMY_ATK_TOP3: return TargetType::ENEMY_ATK_TOP3;
        case pb::TARGET_TYPE_ALLY_HP_LOW1: return TargetType::ALLY_HP_LOW1;
        case pb::TARGET_TYPE_ALLY_HP_LOW2: return TargetType::ALLY_HP_LOW2;
        case pb::TARGET_TYPE_ALLY_HP_LOW3: return TargetType::ALLY_HP_LOW3;
        case pb::TARGET_TYPE_ENEMY_HP_LOW1: return TargetType::ENEMY_HP_LOW1;
        case pb::TARGET_TYPE_ENEMY_HP_LOW2: return TargetType::ENEMY_HP_LOW2;
        case pb::TARGET_TYPE_ENEMY_HP_LOW3: return TargetType::ENEMY_HP_LOW3;
        case pb::TARGET_TYPE_ALLY_RANDOM_1: return TargetType::ALLY_RANDOM_1;
        case pb::TARGET_TYPE_ALLY_RANDOM_2: return TargetType::ALLY_RANDOM_2;
        case pb::TARGET_TYPE_ALLY_RANDOM_3: return TargetType::ALLY_RANDOM_3;
        case pb::TARGET_TYPE_ENEMY_RANDOM_1: return TargetType::ENEMY_RANDOM_1;
        case pb::TARGET_TYPE_ENEMY_RANDOM_2: return TargetType::ENEMY_RANDOM_2;
        case pb::TARGET_TYPE_ENEMY_RANDOM_3: return TargetType::ENEMY_RANDOM_3;
        case pb::TARGET_TYPE_SELF:
        default: return TargetType::SELF;
    }
}

pb::ValueSource toPbValueSource(ValueSource value)
{
    switch (value) {
        case ValueSource::FIXED: return pb::VALUE_SOURCE_FIXED;
        case ValueSource::ATK: return pb::VALUE_SOURCE_ATK;
        case ValueSource::DEF: return pb::VALUE_SOURCE_DEF;
        case ValueSource::MAX_HP: return pb::VALUE_SOURCE_MAX_HP;
        case ValueSource::CUR_HP: return pb::VALUE_SOURCE_CUR_HP;
        case ValueSource::LOST_HP: return pb::VALUE_SOURCE_LOST_HP;
        default: return pb::VALUE_SOURCE_UNSPECIFIED;
    }
}

ValueSource fromPbValueSource(pb::ValueSource value)
{
    switch (value) {
        case pb::VALUE_SOURCE_ATK: return ValueSource::ATK;
        case pb::VALUE_SOURCE_DEF: return ValueSource::DEF;
        case pb::VALUE_SOURCE_MAX_HP: return ValueSource::MAX_HP;
        case pb::VALUE_SOURCE_CUR_HP: return ValueSource::CUR_HP;
        case pb::VALUE_SOURCE_LOST_HP: return ValueSource::LOST_HP;
        case pb::VALUE_SOURCE_FIXED:
        default: return ValueSource::FIXED;
    }
}

pb::ValueOwner toPbValueOwner(ValueOwner value)
{
    switch (value) {
        case ValueOwner::CASTER: return pb::VALUE_OWNER_CASTER;
        case ValueOwner::TARGET: return pb::VALUE_OWNER_TARGET;
        default: return pb::VALUE_OWNER_UNSPECIFIED;
    }
}

ValueOwner fromPbValueOwner(pb::ValueOwner value)
{
    switch (value) {
        case pb::VALUE_OWNER_TARGET: return ValueOwner::TARGET;
        case pb::VALUE_OWNER_CASTER:
        default: return ValueOwner::CASTER;
    }
}

pb::ValueScale toPbValueScale(ValueScale value)
{
    switch (value) {
        case ValueScale::ABSOLUTE: return pb::VALUE_SCALE_ABSOLUTE;
        case ValueScale::PERCENT: return pb::VALUE_SCALE_PERCENT;
        default: return pb::VALUE_SCALE_UNSPECIFIED;
    }
}

ValueScale fromPbValueScale(pb::ValueScale value)
{
    switch (value) {
        case pb::VALUE_SCALE_PERCENT: return ValueScale::PERCENT;
        case pb::VALUE_SCALE_ABSOLUTE:
        default: return ValueScale::ABSOLUTE;
    }
}

pb::EffectType toPbEffectType(EffectType value)
{
    switch (value) {
        case EffectType::NONE: return pb::EFFECT_TYPE_UNSPECIFIED;
        default:
            return static_cast<pb::EffectType>(static_cast<int>(value));
    }
}

EffectType fromPbEffectType(pb::EffectType value)
{
    switch (value) {
        case pb::EFFECT_TYPE_UNSPECIFIED: return EffectType::NONE;
        default:
            return static_cast<EffectType>(static_cast<int>(value));
    }
}

pb::BattleAttr toPbBattleAttr(const BattleAttr& attr)
{
    pb::BattleAttr out;
    out.set_hp(attr.hp);
    out.set_max_hp(attr.maxHp);
    out.set_atk(attr.atk);
    out.set_def(attr.def);
    out.set_speed(attr.speed);
    out.set_crit_rate(attr.critRate);
    out.set_crit_damage(attr.critDamage);
    out.set_crit_resist(attr.critResist);
    out.set_hit_rate(attr.hitRate);
    out.set_dodge_rate(attr.dodgeRate);
    out.set_rage(attr.rage);
    out.set_damage_bonus(attr.damageBonus);
    out.set_damage_reduction(attr.damageReduction);
    out.set_skill_damage(attr.skillDamage);
    out.set_lifesteal(attr.lifesteal);
    out.set_counter_rate(attr.counterRate);
    out.set_multi_hit_rate(attr.multiHitRate);
    out.set_heal_bonus(attr.healBonus);
    out.set_burn_resist(attr.getResistance(BattleAttr::Resistance::Burn));
    out.set_freeze_resist(attr.getResistance(BattleAttr::Resistance::Freeze));
    out.set_stun_resist(attr.getResistance(BattleAttr::Resistance::Stun));
    out.set_silence_resist(attr.getResistance(BattleAttr::Resistance::Silence));
    out.set_poison_resist(attr.getResistance(BattleAttr::Resistance::Poison));
    out.set_taunt_resist(attr.getResistance(BattleAttr::Resistance::Taunt));
    out.set_injury_resist(attr.getResistance(BattleAttr::Resistance::Injury));
    out.set_bleed_resist(attr.getResistance(BattleAttr::Resistance::Bleed));
    out.set_curse_resist(attr.getResistance(BattleAttr::Resistance::Curse));
    return out;
}

BattleAttr fromPbBattleAttr(const pb::BattleAttr& in)
{
    BattleAttr out;
    out.hp = in.hp();
    out.maxHp = in.max_hp();
    out.atk = in.atk();
    out.def = in.def();
    out.speed = in.speed();
    out.critRate = static_cast<BattleAttr::Rate>(in.crit_rate());
    out.critDamage = static_cast<BattleAttr::Rate>(in.crit_damage());
    out.critResist = static_cast<BattleAttr::Rate>(in.crit_resist());
    out.hitRate = static_cast<BattleAttr::Rate>(in.hit_rate());
    out.dodgeRate = static_cast<BattleAttr::Rate>(in.dodge_rate());
    out.rage = static_cast<BattleAttr::Rate>(in.rage());
    out.damageBonus = static_cast<BattleAttr::Rate>(in.damage_bonus());
    out.damageReduction = static_cast<BattleAttr::Rate>(in.damage_reduction());
    out.skillDamage = static_cast<BattleAttr::Rate>(in.skill_damage());
    out.lifesteal = static_cast<BattleAttr::Rate>(in.lifesteal());
    out.counterRate = static_cast<BattleAttr::Rate>(in.counter_rate());
    out.multiHitRate = static_cast<BattleAttr::Rate>(in.multi_hit_rate());
    out.healBonus = static_cast<BattleAttr::Rate>(in.heal_bonus());
    out.setResistance(BattleAttr::Resistance::Burn, static_cast<BattleAttr::Rate>(in.burn_resist()));
    out.setResistance(BattleAttr::Resistance::Freeze, static_cast<BattleAttr::Rate>(in.freeze_resist()));
    out.setResistance(BattleAttr::Resistance::Stun, static_cast<BattleAttr::Rate>(in.stun_resist()));
    out.setResistance(BattleAttr::Resistance::Silence, static_cast<BattleAttr::Rate>(in.silence_resist()));
    out.setResistance(BattleAttr::Resistance::Poison, static_cast<BattleAttr::Rate>(in.poison_resist()));
    out.setResistance(BattleAttr::Resistance::Taunt, static_cast<BattleAttr::Rate>(in.taunt_resist()));
    out.setResistance(BattleAttr::Resistance::Injury, static_cast<BattleAttr::Rate>(in.injury_resist()));
    out.setResistance(BattleAttr::Resistance::Bleed, static_cast<BattleAttr::Rate>(in.bleed_resist()));
    out.setResistance(BattleAttr::Resistance::Curse, static_cast<BattleAttr::Rate>(in.curse_resist()));
    return out;
}

pb::ValueExpr toPbValueExpr(const ValueExpr& expr)
{
    pb::ValueExpr out;
    out.set_source(toPbValueSource(expr.source));
    out.set_owner(toPbValueOwner(expr.owner));
    out.set_scale(toPbValueScale(expr.scale));
    out.set_value(expr.value);
    return out;
}

ValueExpr fromPbValueExpr(const pb::ValueExpr& in)
{
    ValueExpr out;
    out.source = fromPbValueSource(in.source());
    out.owner = fromPbValueOwner(in.owner());
    out.scale = fromPbValueScale(in.scale());
    out.value = in.value();
    return out;
}

pb::SkillEffect toPbSkillEffect(const SkillEffect& effect)
{
    pb::SkillEffect out;
    out.set_target(toPbTargetType(effect.target));
    out.set_effect(toPbEffectType(effect.effect));
    *out.mutable_value_expr() = toPbValueExpr(effect.valueExpr);
    out.set_duration(effect.duration);
    out.set_chance(effect.chance);
    out.set_can_overlay(effect.canOverlay);
    return out;
}

SkillEffect fromPbSkillEffect(const pb::SkillEffect& in)
{
    return SkillEffect(
        fromPbTargetType(in.target()),
        fromPbEffectType(in.effect()),
        fromPbValueExpr(in.value_expr()),
        in.duration(),
        in.chance(),
        in.can_overlay());
}

pb::Skill toPbSkill(const Skill& skill)
{
    pb::Skill out;
    out.set_id(skill.id);
    out.set_name(skill.name);
    out.set_description(skill.description);
    out.set_trigger(toPbSkillTrigger(skill.trigger));
    for (const auto& effect : skill.effects) {
        *out.add_effects() = toPbSkillEffect(effect);
    }
    return out;
}

Skill fromPbSkill(const pb::Skill& in)
{
    Skill out;
    out.id = in.id();
    out.name = in.name();
    out.description = in.description();
    out.trigger = fromPbSkillTrigger(in.trigger());
    out.effects.reserve(static_cast<size_t>(in.effects_size()));
    for (const auto& effect : in.effects()) {
        out.effects.push_back(fromPbSkillEffect(effect));
    }
    return out;
}

pb::Equipment toPbEquipment(const Equipment& equip)
{
    pb::Equipment out;
    out.set_id(equip.id);
    out.set_name(equip.name);
    out.set_type(toPbEquipmentType(equip.type));
    out.set_quality(toPbQuality(equip.quality));
    *out.mutable_base_attrs() = toPbBattleAttr(equip.baseAttrs);
    out.set_skill_id(equip.skillId);
    out.set_set_id(equip.setId);
    return out;
}

Equipment fromPbEquipment(const pb::Equipment& in)
{
    Equipment out;
    out.id = in.id();
    out.name = in.name();
    out.type = fromPbEquipmentType(in.type());
    out.quality = fromPbQuality(in.quality());
    out.baseAttrs = fromPbBattleAttr(in.base_attrs());
    out.skillId = in.skill_id();
    out.setId = in.set_id();
    return out;
}

pb::Inventory toPbInventory(const Inventory& inventory)
{
    pb::Inventory out;
    out.set_max_item_slots(inventory.maxItemSlots());
    out.set_max_equip_slots(inventory.maxEquipSlots());
    for (const auto& item : inventory.items()) {
        const ItemTemplate* definition = item.definition();
        if (!definition) {
            continue;
        }
        pb::ItemStack* stack = out.add_items();
        stack->set_guid(item.guid());
        stack->set_item_template_id(definition->meta.id);
        stack->set_count(item.count());
        stack->set_bound(item.IsBound());
        stack->set_locked(item.IsLocked());
    }
    for (const auto& equip : inventory.equipments()) {
        *out.add_equipments() = toPbEquipment(equip);
    }
    return out;
}

Inventory fromPbInventory(const pb::Inventory& in)
{
    Inventory out;
    if (in.max_item_slots() > 0) {
        out.setMaxItemSlots(in.max_item_slots());
    }
    if (in.max_equip_slots() > 0) {
        out.setMaxEquipSlots(in.max_equip_slots());
    }
    for (const auto& stack : in.items()) {
        out.addItem(stack.item_template_id(), stack.count(), stack.bound());
    }
    for (const auto& equip : in.equipments()) {
        out.addEquipment(fromPbEquipment(equip));
    }
    return out;
}

pb::Character toPbCharacter(const Character& ch)
{
    pb::Character out;
    out.set_id(ch.id);
    out.set_relid(ch.relid);
    out.set_name(ch.name);
    out.set_quality(toPbQuality(ch.quality));
    out.set_position(toPbPosition(ch.position));
    out.set_base_name(ch.baseName);
    out.set_exp(ch.exp);
    out.set_exp_max(ch.expMax);
    out.set_breakthrough(ch.breakthrough);
    out.set_level(ch.level);
    out.set_star(ch.star);
    *out.mutable_origin_attr() = toPbBattleAttr(ch.originAttr);
    *out.mutable_base_attr() = toPbBattleAttr(ch.baseAttr);
    for (const auto& [trigger, skillList] : ch.skills) {
        pb::SkillList& bucket = (*out.mutable_skills_by_trigger())[static_cast<int32_t>(trigger)];
        for (const auto& skill : skillList) {
            *bucket.add_skills() = toPbSkill(skill);
        }
    }
    for (const auto& [type, equip] : ch.equipments) {
        (*out.mutable_equipments_by_type())[static_cast<int32_t>(type)] = toPbEquipment(equip);
    }
    return out;
}

Character fromPbCharacter(const pb::Character& in)
{
    Character out;
    out.id = in.id();
    out.relid = in.relid();
    out.name = in.name();
    out.quality = fromPbQuality(in.quality());
    out.position = fromPbPosition(in.position());
    out.baseName = in.base_name();
    out.exp = in.exp();
    out.expMax = in.exp_max();
    out.breakthrough = in.breakthrough();
    out.level = in.level();
    out.star = in.star();
    out.originAttr = fromPbBattleAttr(in.origin_attr());
    out.baseAttr = fromPbBattleAttr(in.base_attr());

    for (const auto& [triggerValue, list] : in.skills_by_trigger()) {
        SkillTrigger trigger = static_cast<SkillTrigger>(triggerValue);
        auto& targetList = out.skills[trigger];
        targetList.reserve(static_cast<size_t>(list.skills_size()));
        for (const auto& skill : list.skills()) {
            targetList.push_back(fromPbSkill(skill));
        }
    }

    for (const auto& [equipType, equip] : in.equipments_by_type()) {
        out.equipments[static_cast<EquipmentType>(equipType)] = fromPbEquipment(equip);
    }

    return out;
}

pb::Player toPbPlayer(const Player& player)
{
    pb::Player out;
    out.set_schema_version(kPlayerSchemaVersion);
    out.set_id(player.id);
    out.set_name(player.name);
    out.set_level(player.level);
    out.set_vip_level(player.vipLevel);

    for (const auto& [attrType, value] : player.attributes) {
        (*out.mutable_attributes())[static_cast<int32_t>(attrType)] = value;
    }
    for (const auto& [resourceId, value] : player.resources) {
        (*out.mutable_resources())[resourceId] = value;
    }

    out.set_main_character_id(player.mainCharacter ? player.mainCharacter->id : 0);

    for (const auto& [charId, character] : player.characters) {
        (*out.mutable_characters())[charId] = toPbCharacter(character);
    }
    for (int charId : player.battleTeam) {
        out.add_battle_team(charId);
    }
    out.set_combat_power(player.combatPower);

    *out.mutable_inventory() = toPbInventory(player.inventory);

    for (CurrencyType type : {CurrencyType::SOFT, CurrencyType::PREMIUM, CurrencyType::BOUND_PREMIUM, CurrencyType::TOKEN}) {
        const int64_t value = player.getCurrency(type);
        if (value != 0) {
            (*out.mutable_currency_wallet())[static_cast<int32_t>(type)] = value;
        }
    }

    return out;
}

bool applyPbPlayerToRuntime(const pb::Player& in, Player& player)
{
    player.id = in.id();
    player.name = in.name();
    player.level = in.level();
    player.vipLevel = in.vip_level();

    player.attributes.clear();
    for (const auto& [key, value] : in.attributes()) {
        player.attributes[static_cast<PlayerAttrType>(key)] = value;
    }

    player.resources.clear();
    for (const auto& [key, value] : in.resources()) {
        player.resources[key] = value;
    }

    player.characters.clear();
    for (const auto& [charId, pbCharacter] : in.characters()) {
        player.characters[charId] = fromPbCharacter(pbCharacter);
    }

    player.battleTeam.clear();
    player.battleTeam.reserve(static_cast<size_t>(in.battle_team_size()));
    for (int charId : in.battle_team()) {
        player.battleTeam.push_back(charId);
    }

    player.combatPower = in.combat_power();
    player.inventory = fromPbInventory(in.inventory());

    for (CurrencyType type : {CurrencyType::SOFT, CurrencyType::PREMIUM, CurrencyType::BOUND_PREMIUM, CurrencyType::TOKEN}) {
        const auto it = in.currency_wallet().find(static_cast<int32_t>(type));
        if (it != in.currency_wallet().end()) {
            player.addCurrency(type, it->second);
        }
    }

    player.mainCharacter = nullptr;
    if (in.main_character_id() != 0) {
        auto it = player.characters.find(in.main_character_id());
        if (it != player.characters.end()) {
            player.mainCharacter = &it->second;
        }
    }

    return true;
}

std::filesystem::path buildPlayerDataPath(int playerId)
{
    const int safeId = (playerId > 0) ? playerId : 0;
    return std::filesystem::path("build") / "player_data" / ("player_" + std::to_string(safeId) + ".pb");
}

}


// ==================== Player 实现 ====================
Player::Player() : id(0), level(1), vipLevel(0), combatPower(0) {}

Player::Player(int id_, const std::string& name_) 
    : id(id_), name(name_), level(1), vipLevel(0), combatPower(0) {}


// ==================== 武将管理 ====================
bool Player::addCharacter(const Character& ch) {
    if (hasCharacter(ch.id)) {
        return false;  // 已拥有
    }
    characters[ch.id] = ch;
    return true;
}

bool Player::removeCharacter(int charId) {
    if (!hasCharacter(charId)) {
        return false;
    }
    
    // 如果在阵容中，先移除
    removeFromBattleTeam(charId);
    
    characters.erase(charId);
    return true;
}

Character* Player::getCharacter(int charId) {
    auto it = characters.find(charId);
    return it != characters.end() ? &it->second : nullptr;
}

const Character* Player::getCharacter(int charId) const {
    auto it = characters.find(charId);
    return it != characters.end() ? &it->second : nullptr;
}

bool Player::hasCharacter(int charId) const {
    return characters.find(charId) != characters.end();
}

// ==================== 阵容管理 ====================
bool Player::addToBattleTeam(int charId) {
    if (!hasCharacter(charId)) {
        return false;
    }
    
    if (isBattleTeamFull()) {
        return false;
    }
    
    // 检查是否已在阵容
    auto it = std::find(battleTeam.begin(), battleTeam.end(), charId);
    if (it != battleTeam.end()) {
        return false;
    }
    
    battleTeam.push_back(charId);
    updateCombatPower(); // todo : 优化为增量更新
    return true;
}

bool Player::removeFromBattleTeam(int charId) {
    auto it = std::find(battleTeam.begin(), battleTeam.end(), charId);
    if (it == battleTeam.end()) {
        return false;
    }
    
    battleTeam.erase(it);
    updateCombatPower(); // todo : 优化为增量更新
    return true;
}

std::vector<Character*> Player::getBattleTeam() {
    std::vector<Character*> team;
    for (int charId : battleTeam) {
        auto* ch = getCharacter(charId);
        if (ch) {
            team.push_back(ch);
        }
    }
    return team;
}

std::vector<const Character*> Player::getBattleTeam() const {
    std::vector<const Character*> team;
    for (int charId : battleTeam) {
        auto* ch = getCharacter(charId);
        if (ch) {
            team.push_back(ch);
        }
    }
    return team;
}

bool Player::isBattleTeamFull() const {
    return battleTeam.size() >= 6;
}

// ==================== 战力计算 ====================
void Player::updateCombatPower() {
    // 考虑武将等等
}

uint64_t Player::calculateCharacterPower(const Character* ch) const{
    (void)ch; // 目前未实现具体计算逻辑，预留参数以备后续扩展
    uint64_t power = 0;

    return power;
}

// ==================== 数据持久化 ====================
bool Player::saveDataToServer() {
    pb::Player payload = toPbPlayer(*this);
    for (const auto& [currencyType, amount] : currencyWallet_) {
        if (amount != 0) {
            (*payload.mutable_currency_wallet())[static_cast<int32_t>(currencyType)] = amount;
        }
    }
    for (const auto& [offerId, count] : shopPurchaseHistory_) {
        (*payload.mutable_shop_purchase_history())[offerId] = count;
    }
    std::string buffer;
    if (!payload.SerializeToString(&buffer)) {
        return false;
    }

    const std::filesystem::path dataPath = buildPlayerDataPath(id);
    std::error_code ec;
    std::filesystem::create_directories(dataPath.parent_path(), ec);
    if (ec) {
        return false;
    }

    std::ofstream output(dataPath, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        return false;
    }

    output.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    return output.good();
}

bool Player::loadDataFromServer()
{
    const std::filesystem::path dataPath = buildPlayerDataPath(id);
    if (!std::filesystem::exists(dataPath)) {
        return false;
    }

    std::ifstream input(dataPath, std::ios::binary);
    if (!input.is_open()) {
        return false;
    }

    std::string buffer{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()};

    pb::Player payload;
    if (!payload.ParseFromString(buffer)) {
        return false;
    }
    currencyWallet_.clear();
    shopPurchaseHistory_.clear();
    if (!applyPbPlayerToRuntime(payload, *this)) {
        return false;
    }
    for (const auto& [key, value] : payload.currency_wallet()) {
        currencyWallet_[static_cast<CurrencyType>(key)] = value;
    }
    for (const auto& [offerId, count] : payload.shop_purchase_history()) {
        shopPurchaseHistory_[offerId] = count;
    }
    return true;
}

// ==================== 资源属性管理 ====================
void Player::modifyAttribute(PlayerAttrType type, int value)
{
    attributes[type] += value;
    
    // 防止负数
    if (attributes[type] < 0) {
        attributes[type] = 0;
    }
}

int Player::getAttribute(PlayerAttrType type) const
{
    auto it = attributes.find(type);
    return it != attributes.end() ? it->second : 0;
}

// ==================== 资源管理 ====================
void Player::addResource(int resourceId, int64_t amount) {
    resources[resourceId] += amount;
}

bool Player::costResource(int resourceId, int64_t amount) {
    int64_t current = getResource(resourceId);
    if (current < amount) {
        return false;
    }
    resources[resourceId] -= amount;
    return true;
}

int64_t Player::getResource(int resourceId) const {
    auto it = resources.find(resourceId);
    return it != resources.end() ? it->second : 0;
}

// ==================== 货币与商城辅助 ====================
void Player::addCurrency(CurrencyType type, int64_t amount)
{
    if (amount == 0) {
        return;
    }
    currencyWallet_[type] += amount;
}

bool Player::costCurrency(CurrencyType type, int64_t amount)
{
    if (amount < 0) {
        return false;
    }
    const int64_t current = getCurrency(type);
    if (current < amount) {
        return false;
    }
    currencyWallet_[type] = current - amount;
    return true;
}

int64_t Player::getCurrency(CurrencyType type) const
{
    auto it = currencyWallet_.find(type);
    return it != currencyWallet_.end() ? it->second : 0;
}

int Player::getShopPurchaseCount(int offerId) const
{
    auto it = shopPurchaseHistory_.find(offerId);
    return it != shopPurchaseHistory_.end() ? it->second : 0;
}

void Player::recordShopPurchase(int offerId, int quantity)
{
    if (quantity <= 0) {
        return;
    }
    shopPurchaseHistory_[offerId] += quantity;
}

bool Player::purchaseFromShop(ShopType type, int offerId, int quantity, std::string* errMsg)
{
    return ShopService::instance().purchase(*this, type, offerId, quantity, errMsg);
}