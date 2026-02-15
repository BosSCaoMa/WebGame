#pragma once

#include "BattleTypes.h"
#include "Character.h"
#include "Inventory.h"
#include "equip.h"
#include "ShopTypes.h"
#include <string>
#include <map>
#include <vector>
#include <unordered_map>

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
    Inventory inventory;
    
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
    void addCurrency(CurrencyType type, int64_t amount);
    bool costCurrency(CurrencyType type, int64_t amount);
    int64_t getCurrency(CurrencyType type) const;
    
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

    // ==================== 商城交互 ====================
    int getShopPurchaseCount(int offerId) const;
    void recordShopPurchase(int offerId, int quantity);
    bool purchaseFromShop(ShopType type, int offerId, int quantity, std::string* errMsg = nullptr);
    
private:
    uint64_t calculateCharacterPower(const Character* ch) const;

    struct CurrencyHash {
        size_t operator()(CurrencyType type) const noexcept
        {
            return static_cast<size_t>(type);
        }
    };

    std::unordered_map<CurrencyType, int64_t, CurrencyHash> currencyWallet_;
    std::unordered_map<int, int> shopPurchaseHistory_;
};
