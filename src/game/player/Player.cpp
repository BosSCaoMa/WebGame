#include "Player.h"
#include "ItemConfig.h"
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



// 物品资源等=====后续实现

// ==================== Package 实现 ====================
bool Package::addItem(int itemId, int count)
{
    // 查找是否已有该物品
    auto it = itemsMap.find(itemId);
    if (it != itemsMap.end()) {
        ItemSlot& slot = it->second;
        const auto* tmpl = ItemConfig::instance().getItem(itemId);
        if (!tmpl) return false;
        
        // 检查堆叠上限
        if (slot.count + count <= tmpl->maxStack) {
            slot.count += count;
            return true;
        } else {
            return false; // 超过堆叠上限
        }
    }
    
    // 新增物品槽
    if (itemsMap.size() >= maxItemSlots) {
        return false;  // 背包已满
    }
    
    itemsMap[itemId] = ItemSlot(itemId, count);
    return true;
}

bool Package::removeItem(int itemId, int count)
{
    auto it = itemsMap.find(itemId);
    if (it == itemsMap.end()) {
        return false;
    }
    ItemSlot& slot = it->second;
    if (slot.count < count) {
        return false;
    }
    slot.count -= count;
    if (slot.count == 0) {
        itemsMap.erase(it);
    }
    return true;
}

int Package::getItemCount(int itemId) const
{
    auto it = itemsMap.find(itemId);
    if (it != itemsMap.end()) {
        return it->second.count;
    }
    return 0;
}

bool Package::addEquipment(const Equipment& equip)
{
    if (equipments.size() >= maxEquipSlots) {
        return false;
    }
    equipments.push_back(equip);
    return true;
}

// 上装备或出售进行移除
bool Package::removeEquipment(int index)
{
    if (index < 0 || index >= static_cast<int>(equipments.size())) {
        return false;
    }
    equipments.erase(equipments.begin() + index);
    return true;
}

const Equipment* Package::getEquipment(int index) const
{
    if (index < 0 || index >= static_cast<int>(equipments.size())) {
        return nullptr;
    }
    return &equipments[index];
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