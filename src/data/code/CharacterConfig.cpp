#include "CharacterConfig.h"
#include "SkillConfig.h"
#include "json.hpp"
#include "LogM.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>

using nlohmann::json;

namespace {

template <typename Enum>
Enum LookupEnum(const std::unordered_map<std::string, Enum>& table,
                const std::string& key,
                Enum fallback,
                const char* enumName,
                std::string* err) {
    auto it = table.find(key);
    if (it != table.end()) {
        return it->second;
    }
    LOG_ERROR("%s: 未知取值 '%s'，使用默认值。", enumName, key.c_str());
    if (err) {
        if (!err->empty()) err->append("\n");
        err->append(enumName).append(": 未知取值 ").append(key);
    }
    return fallback;
}

const std::unordered_map<std::string, QualityType> kQualityMap = {
    {"WHITE", QualityType::WHITE},
    {"GREEN", QualityType::GREEN},
    {"BLUE", QualityType::BLUE},
    {"PURPLE", QualityType::PURPLE},
    {"ORANGE", QualityType::ORANGE},
    {"RED", QualityType::RED},
    {"GOLD", QualityType::GOLD}
};

const std::unordered_map<std::string, Position> kPositionMap = {
    {"WARRIOR", Position::WARRIOR},
    {"MAGE", Position::MAGE},
    {"TANK", Position::TANK},
    {"HEALER", Position::HEALER},
    {"ASSASSIN", Position::ASSASSIN}
};

std::vector<int> ParseSkillIds(const json& node) {
    std::vector<int> ids;
    if (node.is_array()) {
        for (const auto& v : node) {
            if (v.is_number_integer()) {
                ids.push_back(v.get<int>());
            }
        }
    } else if (node.is_string()) {
        const std::string text = node.get<std::string>();
        std::string token;
        std::istringstream ss(text);
        while (std::getline(ss, token, ',')) {
            try {
                ids.push_back(std::stoi(token));
            } catch (...) {
            }
        }
    }
    return ids;
}

} // namespace

// ==================== 创建武将实例 ====================
Character CharacterConfig::create(int charId) const
{
    const auto* tmpl = get(charId);
    if (!tmpl) {
        return Character();  // 返回空武将
    }
    
    // 1. 创建基础角色
    Character ch(charId, tmpl->name, tmpl->quality, tmpl->position);
    ch.relid = tmpl->relId;
    if (!tmpl->baseName.empty()) {
        ch.baseName = tmpl->baseName;
    }
    
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

bool CharacterConfig::loadFromJson(const json& root, std::string* err)
{
    clear();
    const auto charactersIt = root.find("characters");
    if (charactersIt == root.end()) {
        LOG_ERROR("CharacterConfig: JSON 缺少 characters 字段。");
        if (err) {
            *err += "CharacterConfig: missing characters";
        }
        return false;
    }
    if (!charactersIt->is_array()) {
        LOG_ERROR("CharacterConfig: characters 字段不是数组。");
        if (err) {
            if (!err->empty()) err->append("\n");
            err->append("CharacterConfig: characters is not array");
        }
        return false;
    }

    bool ok = true;
    for (const auto& node : *charactersIt) {
        if (!node.is_object()) {
            continue;
        }
        CharacterTemplate tmpl;
        tmpl.id = node.value("id", 0);
        if (tmpl.id == 0) {
            LOG_ERROR("CharacterConfig: 忽略 id=0 的角色记录。");
            ok = false;
            continue;
        }
        tmpl.name = node.value("name", std::string("未命名角色"));
        tmpl.baseName = node.value("baseName", std::string());
        tmpl.relId = node.value("relId", 0);

        const std::string qualityStr = node.value("quality", std::string("WHITE"));
        tmpl.quality = LookupEnum(kQualityMap, qualityStr, QualityType::WHITE, "QualityType", err);

        const std::string posStr = node.value("position", std::string("WARRIOR"));
        tmpl.position = LookupEnum(kPositionMap, posStr, Position::WARRIOR, "Position", err);

        const auto skillIdsIt = node.find("skillIds");
        if (skillIdsIt != node.end()) {
            tmpl.skillIds = ParseSkillIds(*skillIdsIt);
        }

        reg(tmpl);
    }
    return ok;
}

bool CharacterConfig::loadFromFile(const std::string& path, std::string* err)
{
    std::ifstream in(path);
    if (!in.is_open()) {
        LOG_ERROR("CharacterConfig: 无法打开文件 %s", path.c_str());
        if (err) {
            *err += "CharacterConfig: failed to open file";
        }
        return false;
    }

    json root;
    try {
        in >> root;
    } catch (const std::exception& ex) {
        LOG_ERROR("CharacterConfig: 解析 JSON 失败: %s", ex.what());
        if (err) {
            if (!err->empty()) err->append("\n");
            err->append("CharacterConfig: invalid json");
        }
        return false;
    }

    return loadFromJson(root, err);
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



