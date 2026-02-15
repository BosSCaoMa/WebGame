#include "Inventory.h"

#include "ItemConfig.h"
#include <algorithm>

namespace {
constexpr int64_t kInitialGuid = 1;
}

Inventory::Inventory(unsigned int maxItemSlots, unsigned int maxEquipSlots)
    : maxItemSlots_(maxItemSlots)
    , maxEquipSlots_(maxEquipSlots)
    , nextGuid_(kInitialGuid)
{
}

bool Inventory::addItem(int itemId, int count, bool forceBind)
{
    if (count <= 0) {
        return false;
    }
    const ItemTemplate* tmpl = ItemConfig::instance().getItem(itemId);
    if (!tmpl) {
        return false;
    }

    const bool shouldBind = forceBind || tmpl->meta.ownership.bindOnPickup;
    if (!ensureCapacityFor(tmpl, count, shouldBind)) {
        return false;
    }

    const int maxStack = std::max(1, tmpl->meta.stack.maxStack);
    int remaining = count;

    Item incoming(0, tmpl);
    if (shouldBind) {
        incoming.Bind();
    }

    for (auto& stack : itemStacks_) {
        if (remaining == 0) {
            break;
        }
        if (!stack.definition() || stack.definition()->meta.id != itemId) {
            continue;
        }
        if (stack.IsLocked()) {
            continue;
        }
        if (!stack.CanStackWith(incoming)) {
            continue;
        }
        const int space = maxStack - stack.count();
        if (space <= 0) {
            continue;
        }
        const int delta = std::min(space, remaining);
        stack.setCount(stack.count() + delta);
        remaining -= delta;
    }

    while (remaining > 0) {
        if (itemStacks_.size() >= maxItemSlots_) {
            return false;
        }
        const int delta = std::min(remaining, maxStack);
        itemStacks_.push_back(createStack(tmpl, delta, shouldBind));
        remaining -= delta;
    }

    return true;
}

bool Inventory::removeItem(int itemId, int count)
{
    if (countItem(itemId) < count) {
        return false;
    }

    int remaining = count;
    for (auto it = itemStacks_.begin(); it != itemStacks_.end() && remaining > 0; ) {
        const ItemTemplate* tmpl = it->definition();
        if (!tmpl || tmpl->meta.id != itemId || it->IsLocked()) {
            ++it;
            continue;
        }
        const int delta = std::min(it->count(), remaining);
        it->setCount(it->count() - delta);
        remaining -= delta;
        if (it->count() == 0) {
            it = itemStacks_.erase(it);
        } else {
            ++it;
        }
    }

    return remaining == 0;
}

int Inventory::countItem(int itemId) const
{
    int total = 0;
    for (const auto& stack : itemStacks_) {
        const ItemTemplate* tmpl = stack.definition();
        if (!tmpl || tmpl->meta.id != itemId) {
            continue;
        }
        total += stack.count();
    }
    return total;
}

bool Inventory::hasSpaceFor(int itemId, int count, bool forceBind) const
{
    if (count <= 0) {
        return false;
    }
    const ItemTemplate* tmpl = ItemConfig::instance().getItem(itemId);
    if (!tmpl) {
        return false;
    }
    const bool shouldBind = forceBind || tmpl->meta.ownership.bindOnPickup;
    return ensureCapacityFor(tmpl, count, shouldBind);
}

bool Inventory::addEquipment(const Equipment& equip)
{
    if (equipments_.size() >= maxEquipSlots_) {
        return false;
    }
    equipments_.push_back(equip);
    return true;
}

bool Inventory::removeEquipment(int index)
{
    if (index < 0 || index >= static_cast<int>(equipments_.size())) {
        return false;
    }
    equipments_.erase(equipments_.begin() + index);
    return true;
}

const Equipment* Inventory::getEquipment(int index) const
{
    if (index < 0 || index >= static_cast<int>(equipments_.size())) {
        return nullptr;
    }
    return &equipments_[index];
}

Item Inventory::createStack(const ItemTemplate* tmpl, int count, bool shouldBind)
{
    Item stack(nextGuid_++, tmpl, count);
    if (shouldBind) {
        stack.Bind();
    }
    return stack;
}

bool Inventory::validateUniqueRule(const ItemTemplate* tmpl, int incomingCount) const
{
    if (!tmpl->meta.stack.unique) {
        return true;
    }

    const int maxStack = std::max(1, tmpl->meta.stack.maxStack);
    for (const auto& stack : itemStacks_) {
        if (!stack.definition() || stack.definition()->meta.id != tmpl->meta.id) {
            continue;
        }
        return stack.count() + incomingCount <= maxStack;
    }
    return incomingCount <= maxStack;
}

bool Inventory::ensureCapacityFor(const ItemTemplate* tmpl, int count, bool shouldBind) const
{
    if (!tmpl || count <= 0) {
        return false;
    }
    if (!validateUniqueRule(tmpl, count)) {
        return false;
    }

    const int maxStack = std::max(1, tmpl->meta.stack.maxStack);
    int remaining = count;

    Item incoming(0, tmpl);
    if (shouldBind) {
        incoming.Bind();
    }

    for (const auto& stack : itemStacks_) {
        if (!stack.definition() || stack.definition()->meta.id != tmpl->meta.id) {
            continue;
        }
        if (stack.IsLocked()) {
            continue;
        }
        if (!stack.CanStackWith(incoming)) {
            continue;
        }
        const int space = maxStack - stack.count();
        if (space <= 0) {
            continue;
        }
        remaining -= space;
        if (remaining <= 0) {
            return true;
        }
    }

    if (remaining <= 0) {
        return true;
    }

    const size_t freeSlots = (maxItemSlots_ > itemStacks_.size())
        ? (maxItemSlots_ - itemStacks_.size())
        : 0;

    if (!tmpl->meta.stack.autoSplit) {
        if (remaining > maxStack) {
            return false;
        }
        return freeSlots >= 1;
    }

    const size_t slotsNeeded = static_cast<size_t>((remaining + maxStack - 1) / maxStack);
    return freeSlots >= slotsNeeded;
}
