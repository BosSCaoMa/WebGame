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
    int relId = 0;  // 关联角色ID，用于合击技能
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
    bool loadFromFile(const std::string& path);
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
};

// ==================== 便捷访问宏 ====================
#define GET_CHAR_TMPL(id) CharacterConfig::instance().get(id)
#define CREATE_CHAR(id, lv, star) CharacterConfig::instance().create(id, lv, star)
#define REG_CHAR(tmpl) CharacterConfig::instance().reg(tmpl)
// 约定定义
/*
0 - 999 基础：小兵等等
1000 - 1999 蜀国
2000 - 2999 吴国
3000 - 3999 魏国
4000 - 4999 群雄
5000 - 5999 神将
*/