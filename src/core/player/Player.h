#pragma once

#include "BattleTypes.h"
#include "Character.h"
#include "Item.h"
#include <string>
#include <map>
#include <vector>
#include <unordered_map>

// ==================== 背包系统 ====================
struct ItemSlot {
    int itemId;
    int count;
    
    ItemSlot() : itemId(0), count(0) {}
    ItemSlot(int id, int cnt) : itemId(id), count(cnt) {}
};

class Package {
public:
    std::unordered_map<int, ItemSlot> itemsMap; // 物品背包 <itemId, ItemSlot>
    std::vector<Equipment> equipments;  // 装备背包
    
    unsigned int maxItemSlots = 100;
    unsigned int maxEquipSlots = 100;
    
    // 添加物品
    bool addItem(int itemId, int count);
    
    // 移除物品
    bool removeItem(int itemId, int count);
    
    // 获取物品数量
    int getItemCount(int itemId) const;
    
    // 添加装备
    bool addEquipment(const Equipment& equip);
    
    // 移除装备
    bool removeEquipment(int index);
    
    // 获取装备
    const Equipment* getEquipment(int index) const;
};

// ==================== 玩家类 ====================
class Player {
public:
    // ==================== 基础信息 ====================
    int id = 0;
    std::string name;
    int level = 1;
    int vipLevel = 0;
    
    // ==================== 资源属性 ====================
    std::map<PlayerAttrType, int> attributes;  // 体力、精力等
    std::map<int, int64_t> resources;          // 货币资源（金币、钻石等）
    
    // ==================== 角色系统 ====================
    Character* mainCharacter = nullptr;        // 主角（可选）
    std::map<int, Character> characters;       // 拥有的武将 <武将ID, 武将对象>
    std::vector<int> battleTeam;             // 最多6个位置,id = 0代表空位
    uint64_t combatPower = 0;                  // 总战力
    
    // ==================== 背包系统 ====================
    Package package;
    
    // ==================== 构造函数 ====================
    Player();
    Player(int id_, const std::string& name_);
    
    // ==================== 属性管理 ====================
    void modifyAttribute(PlayerAttrType type, int value);
    int getAttribute(PlayerAttrType type) const;
    
    // ==================== 资源管理 ====================
    void addResource(int resourceId, int64_t amount);
    bool costResource(int resourceId, int64_t amount);
    int64_t getResource(int resourceId) const;
    
    // ==================== 武将管理 ====================
    bool addCharacter(const Character& ch);
    bool removeCharacter(int charId);
    Character* getCharacter(int charId);
    const Character* getCharacter(int charId) const;
    bool hasCharacter(int charId) const;
    
    // ==================== 阵容管理 ====================
    bool addToBattleTeam(int charId);
    bool removeFromBattleTeam(int charId);
    std::vector<Character*> getBattleTeam();
    std::vector<const Character*> getBattleTeam() const;
    bool isBattleTeamFull() const;
    
    // ==================== 战力计算 ====================
    void updateCombatPower();
    
    // ==================== 数据持久化 ====================
    bool saveDataToServer();
    bool loadDataFromServer();
    
private:
    uint64_t calculateCharacterPower(const Character* ch) const;
};
