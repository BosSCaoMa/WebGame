#include "ShopService.h"

#include "Player.h"
#include <algorithm>
#include <utility>

namespace {
constexpr int kUnlimitedStock = -1;
}

ShopService& ShopService::instance()
{
    static ShopService inst;
    return inst;
}

void ShopService::registerOffer(const ShopOffer& offer)
{
    OfferRuntime runtime;
    runtime.data = offer;
    runtime.remainingStock = offer.globalStock > 0 ? offer.globalStock : kUnlimitedStock;
    offers_[offer.type][offer.offerId] = std::move(runtime);
}

bool ShopService::removeOffer(ShopType type, int offerId)
{
    auto bucketIt = offers_.find(type);
    if (bucketIt == offers_.end()) {
        return false;
    }
    return bucketIt->second.erase(offerId) > 0;
}

const ShopOffer* ShopService::getOffer(ShopType type, int offerId) const
{
    auto bucketIt = offers_.find(type);
    if (bucketIt == offers_.end()) {
        return nullptr;
    }
    auto offerIt = bucketIt->second.find(offerId);
    if (offerIt == bucketIt->second.end()) {
        return nullptr;
    }
    return &offerIt->second.data;
}

std::vector<ShopOffer> ShopService::listActiveOffers(ShopType type, std::chrono::system_clock::time_point now) const
{
    std::vector<ShopOffer> result;
    auto bucketIt = offers_.find(type);
    if (bucketIt == offers_.end()) {
        return result;
    }
    for (const auto& [offerId, runtime] : bucketIt->second) {
        (void)offerId;
        if (!runtime.data.isAvailable(now)) {
            continue;
        }
        if (runtime.remainingStock == 0) {
            continue;
        }
        result.push_back(runtime.data);
    }
    return result;
}

bool ShopService::purchase(Player& player, ShopType type, int offerId, int quantity, std::string* errMsg)
{
    if (quantity <= 0) {
        if (errMsg) {
            *errMsg = "购买数量必须大于0";
        }
        return false;
    }

    auto bucketIt = offers_.find(type);
    if (bucketIt == offers_.end()) {
        if (errMsg) {
            *errMsg = "商城不存在";
        }
        return false;
    }
    auto offerIt = bucketIt->second.find(offerId);
    if (offerIt == bucketIt->second.end()) {
        if (errMsg) {
            *errMsg = "商品不存在";
        }
        return false;
    }

    OfferRuntime& runtime = offerIt->second;
    const ShopOffer& offer = runtime.data;
    const auto now = std::chrono::system_clock::now();
    if (!offer.isAvailable(now)) {
        if (errMsg) {
            *errMsg = "商品未在售";
        }
        return false;
    }

    if (runtime.remainingStock != kUnlimitedStock && runtime.remainingStock < quantity) {
        if (errMsg) {
            *errMsg = "库存不足";
        }
        return false;
    }

    const int alreadyPurchased = player.getShopPurchaseCount(offer.offerId);
    if (offer.maxPurchasePerPlayer > 0 && alreadyPurchased + quantity > offer.maxPurchasePerPlayer) {
        if (errMsg) {
            *errMsg = "超出限购次数";
        }
        return false;
    }

    if (offer.price.amount < 0) {
        if (errMsg) {
            *errMsg = "价格配置错误";
        }
        return false;
    }

    const int64_t totalPrice = offer.price.amount * static_cast<int64_t>(quantity);
    if (!player.costCurrency(offer.price.currency, totalPrice)) {
        if (errMsg) {
            *errMsg = "货币不足";
        }
        return false;
    }

    const int totalItems = offer.itemCount * quantity;
    if (!player.inventory.hasSpaceFor(offer.itemId, totalItems, offer.bindOnPurchase)) {
        player.addCurrency(offer.price.currency, totalPrice);
        if (errMsg) {
            *errMsg = "背包空间不足";
        }
        return false;
    }

    if (!player.inventory.addItem(offer.itemId, totalItems, offer.bindOnPurchase)) {
        player.addCurrency(offer.price.currency, totalPrice);
        if (errMsg) {
            *errMsg = "添加物品失败";
        }
        return false;
    }

    if (runtime.remainingStock != kUnlimitedStock) {
        runtime.remainingStock -= quantity;
    }
    player.recordShopPurchase(offer.offerId, quantity);
    return true;
}
