#include "ItemConfig.h"
#include <random>
#include "LogM.h"

// ==================== 创建装备实例 ====================
Equipment ItemConfig::createEquipment(int equipId) const
{
    const EquipmentTemplate* tmpl = getEquipment(equipId);
    if (!tmpl) {
        LOG_ERROR("Equipment template not found for id: %d", equipId);
        return Equipment();
    }
    
    Equipment equip(tmpl->id, tmpl->name, tmpl->type, tmpl->quality, tmpl->skillId, tmpl->setId);
    equip.baseAttrs = tmpl->baseAttrs;
    return equip;
}

void ItemConfig::init() {
    // initConsumables();
    // initMaterials();
    // initEquipments();
    // initSetBonuses();
}

// ==================== 装备配置 ====================
// void ItemConfig::initEquipments() {

//     // 分离各类初始化函数，便于维护
//     initWeapons();
//     initArmors();
//     initHelmets();
//     initBoots();
//     initSteeds();
//     initTallys();
//     initTreasures();
//     initFamouss();
      
// }

// Equipment ItemConfig::createEquipment(int equipId) const
// {
//     const auto* tmpl = getEquipment(equipId);
//     if (!tmpl) {
//         LOG_ERROR("Equipment template not found for id: {%d}", equipId);
//         return Equipment();
//     }
    
//     // 生成主词缀
//     Equipment equip(tmpl->id, tmpl->name, tmpl->type, tmpl->quality, tmpl->skillId, tmpl->setId);
//     for (const auto& affix : tmpl->mainAffixs) {
//         equip.mainAffixs.push_back(affix);
//     }
//     // 随机器
//     std::random_device rd;  // 随机数种子（硬件随机数）
//     std::mt19937 gen(rd());  // 梅森旋转算法随机数生成器
    
//     // 根据品质生成副词缀
//     int subAffixCount = static_cast<int>(tmpl->quality) - 2;  // 蓝装1个, 紫装2个, 橙装3个, 红装4个
//     subAffixCount = std::min(subAffixCount, 4);
    
//     static const AffixType possibleAffixes[] = {
//         AffixType::CRIT_RATE, AffixType::CRIT_DAMAGE, AffixType::HIT_RATE,
//         AffixType::COUNTER_RATE, AffixType::HEAL_BONUS, AffixType::MULTI_HIT_RATE,
//         AffixType::STUN_RESIST, AffixType::SILENCE_RESIST, AffixType::POISON_RESIST
//     }; // 随机词缀池,全部是百分比加成

//     // 生成模板指定范围内的随机数
//     int minvalue = 3, maxvalue = 5;
//     minvalue += static_cast<int>(tmpl->quality);
//     maxvalue += static_cast<int>(tmpl->quality) * 2;
//     int numPossible = sizeof(possibleAffixes) / sizeof(possibleAffixes[0]);
//     std::uniform_int_distribution<> affixDis(0, numPossible - 1); // possibleAffixes数组索引
//     std::uniform_int_distribution<> valueDis(minvalue, maxvalue); // 词缀数值范围
    
//     for (int i = 0; i < subAffixCount; ++i) {
//         AffixType type = possibleAffixes[affixDis(gen)];
//         int64_t value = valueDis(gen);
//         equip.subAffixes.push_back({type, value});
//     }
    
//     return equip;
// }