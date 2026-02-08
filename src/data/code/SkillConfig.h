#pragma once

#include "Skill.h"
#include <unordered_map>
#include <functional>

// ==================== 技能配置管理器 ====================
class SkillConfig {
public:
    // 单例访问
    static SkillConfig& instance() {
        static SkillConfig inst;
        return inst;
        
    }
    
    // 获取技能（返回副本）
    Skill get(int skillId) const {
        auto it = skills_.find(skillId);
        if (it != skills_.end()) {
            return it->second;
        }
        return Skill(0, "未知技能", SkillTrigger::None);
    }
    
    // 检查技能是否存在
    bool has(int skillId) const {
        return skills_.find(skillId) != skills_.end();
    }
    
    // 注册技能
    void reg(const Skill& skill) {
        skills_[skill.id] = skill;
    }

private:
    SkillConfig() { /* initSkills(); */ }
    SkillConfig(const SkillConfig&) = delete;
    SkillConfig& operator=(const SkillConfig&) = delete;
    
    using T = TargetType;
    using E = EffectType;
    using Tr = SkillTrigger;
    using SE = SkillEffect;
    // void initSkills();
    // void initQunSkills();
    // void initWeiSkills();
    // void initShuSkills();
    // void initWuSkills();
    // void initShenSkills();
    // void initBaseSkills();
    // void initEquipSkills();
    // void initFourEquipSkills();
    // void initMountAndGeneralSkills();
    // void initTreasureSkills();
    std::unordered_map<int, Skill> skills_;
};

// ==================== 便捷访问宏 ====================
#define GET_SKILL(id) SkillConfig::instance().get(id)
#define REG_SKILL(skill) SkillConfig::instance().reg(skill)
/*
0 - 无技能
1 ~ 999 基础技能
1001 ~ 1999 吴国技能
2001 ~ 2999 蜀国技能
3001 ~ 3999 魏国技能
4001 ~ 4999 群雄技能
5001 ~ 5999 神将技能
7001 ~ 7499 武器技能
7500 ~ 7999 防具技能
8001 ~ 8499 头盔技能
8500 ~ 8999 鞋子技能
9001 ~ 9499 坐骑技能
9500 ~ 9999 名将技能
10001 ~ 10499 宝物技能
10500 ~ 10999 兵符技能
*/