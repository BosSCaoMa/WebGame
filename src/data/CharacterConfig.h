#pragma once

#include "Character.h"
#include "SkillConfig.h"
#include "BattleAttr.h"
#include "BattleTypes.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <initializer_list>

// ==================== 武将模板 ====================
struct CharacterTemplate {
    int id;
    std::string name;
    QualityType quality; 
    Position position;
    // ===== 技能 =====
    std::vector<int> skillIds;      // 技能ID列表
    
    CharacterTemplate() : id(0), quality(QualityType::WHITE) {}
    
    CharacterTemplate(int id_, const std::string& name_, QualityType quality_,
        Position position, std::initializer_list<int> skills_)
        : id(id_), name(name_), quality(quality_)
        , position(position), skillIds(skills_) {}
};

// ==================== 武将配置管理器 ====================
class CharacterConfig {
public:
    static CharacterConfig& instance() {
        static CharacterConfig inst;
        return inst;
    }
    
    // 获取武将模板
    const CharacterTemplate* get(int charId) const {
        auto it = templates_.find(charId);
        return it != templates_.end() ? &it->second : nullptr;
    }
    
    
    // ===== 创建武将实例 ===== ⭐ 更新签名
    Character create(int charId) const;
    
    // 注册武将模板
    void reg(const CharacterTemplate& tmpl) {
        templates_[tmpl.id] = tmpl;
    }
    
    // 获取所有武将ID
    std::vector<int> getAllIds() const;

private:
    CharacterConfig() { /* initCharacters(); */ }
    CharacterConfig(const CharacterConfig&) = delete;
    CharacterConfig& operator=(const CharacterConfig&) = delete;
    
    std::unordered_map<int, CharacterTemplate> templates_;

    // void initCharacters();
    // void initBaseCharacters();
    // void InitShuCharacters();
    // void InitWuCharacters();
    // void InitWeiCharacters();
    // void InitQunCharacters();
    // void InitShenCharacters();
    // void addHero(int id, const char* name, QualityType quality, Position pos,
    //     std::initializer_list<int> skills);
    // void InitShuRedCharacters();
    // void InitShuOrangeCharacters();
    // void InitShuPurpleCharacters();
    // void InitShuBlueCharacters();
    // void InitShuGreenCharacters();
    // void InitWuRedCharacters();
    // void InitWuOrangeCharacters();
    // void InitWuPurpleCharacters();
    // void InitWuBlueCharacters();
    // void InitWuGreenCharacters();
    // void InitWeiRedCharacters();
    // void InitWeiOrangeCharacters();
    // void InitWeiPurpleCharacters();
    // void InitWeiBlueCharacters();
    // void InitWeiGreenCharacters();
    // void InitQunRedCharacters();
    // void InitQunOrangeCharacters();
    // void InitQunPurpleCharacters();
    // void InitQunBlueCharacters();
    // void InitQunGreenCharacters();
};

// ==================== 便捷访问宏 ====================
#define GET_CHAR_TMPL(id) CharacterConfig::instance().get(id)
#define CREATE_CHAR(id, lv, star) CharacterConfig::instance().create(id, lv, star)

// 约定定义
/*
0 - 999 基础：小兵等等
1000 - 1999 蜀国
2000 - 2999 吴国
3000 - 3999 魏国
4000 - 4999 群雄
5000 - 5999 神将
*/