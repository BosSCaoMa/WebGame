#include "Player.h"
#include "ItemConfig.h"
#include "ShopService.h"
#include <iostream>

namespace shop::testing {
namespace {
void SeedTestItems()
{
    ItemConfig::instance().clear();

    ItemTemplate potion;
    potion.meta.id = 10001;
    potion.meta.name = "中瓶治疗药水";
    potion.meta.type = ItemType::CONSUMABLE;
    potion.meta.stack.maxStack = 50;
    potion.payload = ConsumableProfile {
        .category = ConsumableType::HP_POTION,
        .effects = {},
        .cooldownSec = 5,
        .destroyOnUse = true
    };
    ItemConfig::instance().regItem(potion);
}

ShopOffer BuildTestOffer(int itemId)
{
    ShopOffer offer;
    offer.offerId = 5001;
    offer.itemId = itemId;
    offer.itemCount = 5;
    offer.type = ShopType::PERMANENT;
    offer.price = ShopPrice {CurrencyType::SOFT, 120};
    offer.maxPurchasePerPlayer = 5;
    offer.bindOnPurchase = false;
    return offer;
}
}

void RunPurchaseSmokeTest()
{
    SeedTestItems();
    const int itemId = 10001;
    ShopService::instance().registerOffer(BuildTestOffer(itemId));

    Player tester(999, "ShopTester");
    tester.addCurrency(CurrencyType::SOFT, 1000);

    std::string error;
    if (!tester.purchaseFromShop(ShopType::PERMANENT, 5001, 2, &error)) {
        std::cout << "购买失败: " << error << std::endl;
        return;
    }

    std::cout << "购买成功，背包获得道具数量="
              << tester.inventory.countItem(itemId)
              << "，剩余软货币=" << tester.getCurrency(CurrencyType::SOFT)
              << std::endl;
}

} // namespace shop::testing
