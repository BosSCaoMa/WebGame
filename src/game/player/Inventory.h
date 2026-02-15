#pragma once

#include "item.h"
#include "equip.h"
#include <cstdint>
#include <vector>

class Inventory {
public:
    explicit Inventory(unsigned int maxItemSlots = 100, unsigned int maxEquipSlots = 100);

    bool addItem(int itemId, int count, bool forceBind = false);
    bool removeItem(int itemId, int count);
    int countItem(int itemId) const;
    bool hasSpaceFor(int itemId, int count, bool forceBind = false) const;

    bool addEquipment(const Equipment& equip);
    bool removeEquipment(int index);
    const Equipment* getEquipment(int index) const;

    const std::vector<Item>& items() const { return itemStacks_; }
    const std::vector<Equipment>& equipments() const { return equipments_; }

    void setMaxItemSlots(unsigned int slots) { maxItemSlots_ = slots; }
    void setMaxEquipSlots(unsigned int slots) { maxEquipSlots_ = slots; }
    unsigned int maxItemSlots() const { return maxItemSlots_; }
    unsigned int maxEquipSlots() const { return maxEquipSlots_; }

private:
    Item createStack(const ItemTemplate* tmpl, int count, bool shouldBind);
    bool validateUniqueRule(const ItemTemplate* tmpl, int incomingCount) const;
    bool ensureCapacityFor(const ItemTemplate* tmpl, int count, bool shouldBind) const;

    unsigned int maxItemSlots_;
    unsigned int maxEquipSlots_;
    int64_t nextGuid_;
    std::vector<Item> itemStacks_;
    std::vector<Equipment> equipments_;
};
