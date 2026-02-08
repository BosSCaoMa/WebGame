#include "CharacterConfig.h"
#include "SkillConfig.h"
#include <cmath>
#include <algorithm>

// ==================== 创建武将实例 ====================
Character CharacterConfig::create(int charId) const
{
    const auto* tmpl = get(charId);
    if (!tmpl) {
        return Character();  // 返回空武将
    }
    
    // 1. 创建基础角色
    Character ch(charId, tmpl->name, tmpl->quality, tmpl->position);
    
    // 2. 计算属性
    ch.originAttr.InitAttr(ch.position, ch.quality); // 根据定位和品质初始化基础属性
    ch.baseAttr = ch.originAttr;  // 初始时，当前属性 = 基础属性
    
    // 3. 初始化技能
    for (size_t i = 0; i < tmpl->skillIds.size(); ++i) {
        int skillId = tmpl->skillIds[i];
        Skill aSkill = GET_SKILL(skillId);
        ch.setSkill(aSkill);
    }
    
    return ch;
}

// ==================== 获取所有武将ID ====================
std::vector<int> CharacterConfig::getAllIds() const {
    std::vector<int> ids;
    ids.reserve(templates_.size());
    for (const auto& [id, _] : templates_) {
        ids.push_back(id);
    }
    return ids;
}

// ==================== 预定义属性模板 ====================

struct BaseCharDef {
    int id;
    const char* name;
    QualityType quality;
    Position position;
    std::initializer_list<int> skills;
};

// void CharacterConfig::initCharacters()
// {
//     initBaseCharacters(); // 初始化基础武将
//     InitShuCharacters(); // 初始化蜀国武将
//     InitWuCharacters(); // 初始化吴国武将
//     InitWeiCharacters(); // 初始化魏国武将
//     InitQunCharacters(); // 初始化群雄武将
//     InitShenCharacters(); // 初始化神将
// }

// void CharacterConfig::InitShuCharacters()
// {
//     InitShuRedCharacters();
//     InitShuOrangeCharacters();
//     InitShuPurpleCharacters();
//     InitShuBlueCharacters();
//     InitShuGreenCharacters();
// }

// void CharacterConfig::addHero(int id, const char* name, QualityType quality, Position pos,
//     std::initializer_list<int> skills)
// {
//     reg(CharacterTemplate(id, name, quality, pos, skills));
// }

// void CharacterConfig::InitWuCharacters()
// {
//     InitWuRedCharacters();
//     InitWuOrangeCharacters();
//     InitWuPurpleCharacters();
//     InitWuBlueCharacters();
//     InitWuGreenCharacters();
// }

// void CharacterConfig::InitWeiCharacters()
// {
//     InitWeiRedCharacters();
//     InitWeiOrangeCharacters();
//     InitWeiPurpleCharacters();
//     InitWeiBlueCharacters();
//     InitWeiGreenCharacters();
// }

// void CharacterConfig::InitQunCharacters()
// {
//     InitQunRedCharacters();
//     InitQunOrangeCharacters();
//     InitQunPurpleCharacters();
//     InitQunBlueCharacters();
//     InitQunGreenCharacters();
// }

// void CharacterConfig::InitShenCharacters()
// {

// }



