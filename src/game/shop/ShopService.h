#pragma once

#include "ShopTypes.h"
#include "ItemTypes.h"
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Player;

struct ShopPrice {
    CurrencyType currency {CurrencyType::UNKNOWN};
    int64_t amount {0};
};

struct ShopRotationWindow {
    std::chrono::system_clock::time_point start {};
    std::chrono::system_clock::time_point end {};

    bool contains(std::chrono::system_clock::time_point now) const
    {
        return now >= start && now <= end;
    }
};

struct ShopOffer {
    int offerId {0};
    int itemId {0};
    int itemCount {1};
    ShopType type {ShopType::PERMANENT};
    ShopPrice price;
    bool bindOnPurchase {false};
    int maxPurchasePerPlayer {0};
    int globalStock {0};
    std::optional<ShopRotationWindow> rotation;
    std::string tag;

    bool isAvailable(std::chrono::system_clock::time_point now) const
    {
        if (!rotation.has_value()) {
            return true;
        }
        return rotation->contains(now);
    }
};

class ShopService {
public:
    static ShopService& instance();

    void registerOffer(const ShopOffer& offer);
    bool removeOffer(ShopType type, int offerId);
    const ShopOffer* getOffer(ShopType type, int offerId) const;
    std::vector<ShopOffer> listActiveOffers(ShopType type, std::chrono::system_clock::time_point now) const;
    bool purchase(Player& player, ShopType type, int offerId, int quantity, std::string* errMsg = nullptr);

private:
    ShopService() = default;

    struct OfferRuntime {
        ShopOffer data;
        int remainingStock {0};
    };

    struct EnumClassHash {
        template <typename T>
        size_t operator()(T value) const noexcept
        {
            return static_cast<size_t>(value);
        }
    };

    using OfferBucket = std::unordered_map<int, OfferRuntime>;
    std::unordered_map<ShopType, OfferBucket, EnumClassHash> offers_;
};
