#include <gtest/gtest.h>

#include "ItemConfig.h"
#include "Player.h"
#include "ShopService.h"

namespace {

ItemTemplate BuildPotionTemplate(int itemId)
{
	ItemTemplate potion;
	potion.meta.id = itemId;
	potion.meta.name = "中瓶治疗药水";
	potion.meta.description = "恢复适量生命";
	potion.meta.type = ItemType::CONSUMABLE;
	potion.meta.stack.maxStack = 50;
	potion.meta.quality = QualityType::GREEN;

	ConsumableProfile profile;
	profile.category = ConsumableType::HP_POTION;
	profile.cooldownSec = 5;
	profile.destroyOnUse = true;
	potion.payload = profile;
	return potion;
}

ShopOffer BuildPotionOffer(int offerId, int itemId)
{
	ShopOffer offer;
	offer.offerId = offerId;
	offer.itemId = itemId;
	offer.itemCount = 5;
	offer.type = ShopType::PERMANENT;
	offer.price = ShopPrice {CurrencyType::SOFT, 120};
	offer.maxPurchasePerPlayer = 5;
	offer.bindOnPurchase = false;
	return offer;
}

class ShopServiceTest : public ::testing::Test {
protected:
	void SetUp() override
	{
		ItemConfig::instance().clear();
		ShopService::instance().clear();
	}

	void TearDown() override
	{
		ShopService::instance().clear();
		ItemConfig::instance().clear();
	}

	void SeedPotionItem(int itemId)
	{
		ItemConfig::instance().regItem(BuildPotionTemplate(itemId));
	}

	ShopOffer RegisterOffer(int offerId, int itemId)
	{
		auto offer = BuildPotionOffer(offerId, itemId);
		ShopService::instance().registerOffer(offer);
		return offer;
	}
};

TEST_F(ShopServiceTest, PurchaseSucceedsWhenPlayerHasCurrency)
{
	constexpr int kItemId = 10001;
	constexpr int kOfferId = 5001;
	SeedPotionItem(kItemId);
	const ShopOffer offer = RegisterOffer(kOfferId, kItemId);

	Player tester(999, "ShopTester");
	tester.addCurrency(CurrencyType::SOFT, 2000);

	std::string error;
	ASSERT_TRUE(tester.purchaseFromShop(offer.type, offer.offerId, 2, &error)) << error;
	EXPECT_EQ(tester.inventory.countItem(kItemId), offer.itemCount * 2);
	EXPECT_EQ(tester.getCurrency(CurrencyType::SOFT), 2000 - offer.price.amount * 2);
}

TEST_F(ShopServiceTest, PurchaseFailsWhenCurrencyIsInsufficient)
{
	constexpr int kItemId = 10002;
	constexpr int kOfferId = 5002;
	SeedPotionItem(kItemId);
	const ShopOffer offer = RegisterOffer(kOfferId, kItemId);

	Player tester(1000, "LowBudgetTester");
	tester.addCurrency(CurrencyType::SOFT, 50);

	std::string error;
	EXPECT_FALSE(tester.purchaseFromShop(offer.type, offer.offerId, 1, &error));
	EXPECT_EQ(error, "货币不足");
	EXPECT_EQ(tester.inventory.countItem(kItemId), 0);
	EXPECT_EQ(tester.getCurrency(CurrencyType::SOFT), 50);
}

} // namespace
