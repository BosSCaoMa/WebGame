#pragma once

#include "ItemTypes.h"
#include "BattleTypes.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

// ==================== 通用物品效果 ====================
// 描述消耗品或技能类条目产生的单个效果，支持数值来源扩展
struct ItemEffect {
    EffectType type {EffectType::NONE};
    int64_t magnitude {0};
    ValueScale scale {ValueScale::ABSOLUTE};
    ValueOwner owner {ValueOwner::TARGET};
    int durationTurn {0};
};

// 控制堆叠方式（默认可堆叠且遇到上限会自动拆分）
struct ItemStackingRule {
    int maxStack {1};
    bool unique {false};
    bool autoSplit {true};
};

// 控制物品的绑定与交易限制
struct ItemOwnershipRule {
    bool tradable {true};
    bool bindOnPickup {false};
    bool bindOnEquip {false};
};

// Item 的轻量级元信息，供索引、显示与通用逻辑使用
struct ItemMeta {
    int id {0};
    std::string name;
    std::string description;
    ItemType type {ItemType::NONE};
    QualityType quality {QualityType::WHITE};
    ItemStackingRule stack;
    ItemOwnershipRule ownership;
    std::vector<std::string> tags;
};

// 针对消耗品补充额外属性，例如冷却和效果列表
struct ConsumableProfile {
    ConsumableType category {ConsumableType::NONE};
    std::vector<ItemEffect> effects;
    int cooldownSec {0};
    bool destroyOnUse {true};
};

// 材料类物品的分类与使用说明
struct MaterialProfile {
    MaterialType category {MaterialType::NONE};
    int tier {0};
    std::string usage;
};

// 货币类定义（是否跨角色共享、是否允许负数）
struct CurrencyProfile {
    CurrencyType category {CurrencyType::UNKNOWN};
    int precision {1};
    bool sharedAcrossRoles {true};
    bool allowNegative {false};
};

using ItemPayload = std::variant<std::monostate, ConsumableProfile, MaterialProfile, CurrencyProfile>;

// ItemTemplate 聚合通用元信息与具体载荷，供数据配置层使用
struct ItemTemplate {
    ItemMeta meta;
    ItemPayload payload;

    bool IsStackable() const { return meta.stack.maxStack > 1 && !meta.stack.unique; }
    bool HasTag(const std::string& tag) const {
        return std::find(meta.tags.begin(), meta.tags.end(), tag) != meta.tags.end();
    }
    bool IsConsumable() const { return std::holds_alternative<ConsumableProfile>(payload); }
    bool IsMaterial() const { return std::holds_alternative<MaterialProfile>(payload); }
    bool IsCurrency() const { return std::holds_alternative<CurrencyProfile>(payload); }

    template <typename T>
    const T* TryGet() const
    {
        return std::get_if<T>(&payload);
    }
};

// ==================== 背包实例 ====================
// ==================== 背包实例 ====================
// Item 运行时实例，引用 ItemTemplate 并记录绑定状态
class Item {
public:
    Item() = default;
    Item(int64_t guid, const ItemTemplate* definition, int count = 1)
        : guid_(guid), definition_(definition), count_(count) {}

    int64_t guid() const { return guid_; }
    int count() const { return count_; }
    const ItemTemplate* definition() const { return definition_; }

    void setCount(int count) { count_ = count; }
    void Bind() { bound_ = true; }
    bool IsBound() const { return bound_; }
    void Lock() { locked_ = true; }
    void Unlock() { locked_ = false; }
    bool IsLocked() const { return locked_; }

    bool CanStackWith(const Item& other) const
    {
        if (!definition_ || !other.definition_) {
            return false;
        }
        if (definition_->meta.id != other.definition_->meta.id) {
            return false;
        }
        if (!definition_->IsStackable()) {
            return false;
        }
        return !bound_ && !other.bound_;
    }

private:
    int64_t guid_ {0};
    const ItemTemplate* definition_ {nullptr};
    int count_ {1};
    bool bound_ {false};
    bool locked_ {false};
};