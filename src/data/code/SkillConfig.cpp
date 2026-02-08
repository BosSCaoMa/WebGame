#include "SkillConfig.h"
#include <fstream>
#include "json.hpp"
#include "LogM.h"

using nlohmann::json;
namespace {
const std::unordered_map<std::string, SkillTrigger> kSkillTriggerMap = {
	{"NORMAL_ATTACK", SkillTrigger::NORMAL_ATTACK},
	{"RAGE_SKILL", SkillTrigger::RAGE_SKILL},
	{"ON_SAME_CAMP", SkillTrigger::ON_SAME_CAMP},
	{"BATTLE_START", SkillTrigger::BATTLE_START},
	{"ROUND_START", SkillTrigger::ROUND_START},
	{"ROUND_END", SkillTrigger::ROUND_END},
	{"TURN_START", SkillTrigger::TURN_START},
	{"TURN_END", SkillTrigger::TURN_END},
	{"ROUND_ODD", SkillTrigger::ROUND_ODD},
	{"ROUND_EVEN", SkillTrigger::ROUND_EVEN},
	{"ROUND_MULTIPLE_OF_FIVE", SkillTrigger::ROUND_MULTIPLE_OF_FIVE},
	{"ROUND_TEN", SkillTrigger::ROUND_TEN},
	{"ON_HIT", SkillTrigger::ON_HIT},
	{"ON_CONTROL", SkillTrigger::ON_CONTROL},
	{"ON_KILL", SkillTrigger::ON_KILL},
	{"ON_DEATH", SkillTrigger::ON_DEATH},
	{"None", SkillTrigger::None}
};

const std::unordered_map<std::string, TargetType> kTargetTypeMap = {
	{"SELF", TargetType::SELF},
	{"ALLY_ALL", TargetType::ALLY_ALL},
	{"ENEMY_SINGLE", TargetType::ENEMY_SINGLE},
	{"ENEMY_COL", TargetType::ENEMY_COL},
	{"ENEMY_ALL", TargetType::ENEMY_ALL},
	{"ALLY_FRONT_ROW", TargetType::ALLY_FRONT_ROW},
	{"ALLY_BACK_ROW", TargetType::ALLY_BACK_ROW},
	{"ENEMY_FRONT_ROW", TargetType::ENEMY_FRONT_ROW},
	{"ENEMY_BACK_ROW", TargetType::ENEMY_BACK_ROW},
	{"ALLY_ATK_TOP1", TargetType::ALLY_ATK_TOP1},
	{"ALLY_ATK_TOP2", TargetType::ALLY_ATK_TOP2},
	{"ALLY_ATK_TOP3", TargetType::ALLY_ATK_TOP3},
	{"ENEMY_ATK_TOP1", TargetType::ENEMY_ATK_TOP1},
	{"ENEMY_ATK_TOP2", TargetType::ENEMY_ATK_TOP2},
	{"ENEMY_ATK_TOP3", TargetType::ENEMY_ATK_TOP3},
	{"ALLY_HP_LOW1", TargetType::ALLY_HP_LOW1},
	{"ALLY_HP_LOW2", TargetType::ALLY_HP_LOW2},
	{"ALLY_HP_LOW3", TargetType::ALLY_HP_LOW3},
	{"ENEMY_HP_LOW1", TargetType::ENEMY_HP_LOW1},
	{"ENEMY_HP_LOW2", TargetType::ENEMY_HP_LOW2},
	{"ENEMY_HP_LOW3", TargetType::ENEMY_HP_LOW3},
	{"ALLY_RANDOM_1", TargetType::ALLY_RANDOM_1},
	{"ALLY_RANDOM_2", TargetType::ALLY_RANDOM_2},
	{"ALLY_RANDOM_3", TargetType::ALLY_RANDOM_3},
	{"ENEMY_RANDOM_1", TargetType::ENEMY_RANDOM_1},
	{"ENEMY_RANDOM_2", TargetType::ENEMY_RANDOM_2},
	{"ENEMY_RANDOM_3", TargetType::ENEMY_RANDOM_3}
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

const std::unordered_map<std::string, ValueSource> kValueSourceMap = {
	{"FIXED", ValueSource::FIXED},
	{"ATK", ValueSource::ATK},
	{"DEF", ValueSource::DEF},
	{"MAX_HP", ValueSource::MAX_HP},
	{"CUR_HP", ValueSource::CUR_HP},
	{"LOST_HP", ValueSource::LOST_HP}
};

const std::unordered_map<std::string, ValueOwner> kValueOwnerMap = {
	{"CASTER", ValueOwner::CASTER},
	{"TARGET", ValueOwner::TARGET}
};

const std::unordered_map<std::string, ValueScale> kValueScaleMap = {
	{"ABSOLUTE", ValueScale::ABSOLUTE},
	{"PERCENT", ValueScale::PERCENT}
};

template <typename Enum>
Enum LookupEnum(const std::unordered_map<std::string, Enum>& table,
	const std::string& key, Enum fallback, const char* enumName) {
	auto it = table.find(key);
	if (it != table.end()) {
		return it->second;
	}
	LOG_ERROR("%s: 未知取值 '%s'，使用默认值。", enumName, key.c_str());
	return fallback;
}

bool loadFromJson(const json& root)
{
	if (!root.contains("skills")) {
		LOG_ERROR("SkillConfig: JSON 中缺少 skills 字段。");
		return false;
	}

	const json& skillsJson = root.at("skills");
	if (!skillsJson.is_array()) {
		LOG_ERROR("SkillConfig: skills 字段不是数组。");
		return false;
	}

	for (const auto& skillNode : skillsJson) {
		if (!skillNode.is_object()) {
			continue;
		}

		Skill skill;
		skill.id = skillNode.value("id", 0);
		if (skill.id == 0) { // 忽略 id=0 的技能记录
			continue;
		}

		skill.name = skillNode.value("name", std::string("未命名技能"));
		skill.description = skillNode.value("description", std::string());
		const std::string triggerStr = skillNode.value("trigger", std::string("None"));
		skill.trigger = LookupEnum(kSkillTriggerMap, triggerStr, SkillTrigger::None, "SkillTrigger");

		skill.effects.clear();
		const auto effectsIt = skillNode.find("effects");
		if (effectsIt != skillNode.end() && effectsIt->is_array()) {
			for (const auto& eff : *effectsIt) {
				if (!eff.is_object()) {
					continue;
				}
				SkillEffect effect;
				const std::string targetStr = eff.value("target", std::string("SELF"));
				effect.target = LookupEnum(kTargetTypeMap, targetStr, TargetType::SELF, "TargetType");

				const std::string effectStr = eff.value("effect", std::string("NONE"));
				effect.effect = LookupEnum(kEffectTypeMap, effectStr, EffectType::NONE, "EffectType");

				const json valueExprJson = eff.value("valueExpr", json::object());
				const std::string sourceStr = valueExprJson.value("source", std::string("FIXED"));
				const std::string ownerStr = valueExprJson.value("owner", std::string("CASTER"));
				const std::string scaleStr = valueExprJson.value("scale", std::string("ABSOLUTE"));
				effect.valueExpr.source = LookupEnum(kValueSourceMap, sourceStr, ValueSource::FIXED, "ValueSource");
				effect.valueExpr.owner = LookupEnum(kValueOwnerMap, ownerStr, ValueOwner::CASTER, "ValueOwner");
				effect.valueExpr.scale = LookupEnum(kValueScaleMap, scaleStr, ValueScale::ABSOLUTE, "ValueScale");
				effect.valueExpr.value = valueExprJson.value("value", static_cast<int64_t>(0));

				effect.duration = eff.value("duration", 0);
				effect.chance = eff.value("chance", 100);
				effect.canOverlay = eff.value("canOverlay", false);
				skill.effects.push_back(effect);
			}
		}

		REG_SKILL(skill);
	}
	return true;
}
}


bool SkillConfig::loadFromFile(const std::string& path)
{
	std::ifstream in(path);
	if (!in.is_open()) {
		LOG_ERROR("SkillConfig: 无法打开文件 %s", path.c_str());
		return false;
	}

	json root;
	try {
		in >> root;
	} catch (const std::exception& ex) {
		LOG_ERROR("SkillConfig: 解析 JSON 失败: %s", ex.what());
		return false;
	}
    clear();
	return loadFromJson(root);
}