#include "Player.h"
#include "ShopService.h"
#include <algorithm>


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
    // TODO: 实现与服务器的数据同步
    // 序列化玩家数据并发送
    return true;
}

bool Player::loadDataFromServer()
{
    // 后续todo : 实现从服务器加载数据
    // 反序列化玩家数据
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