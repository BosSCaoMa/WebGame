#include "gtest/gtest.h"

#include "Player.h"

#include <filesystem>

namespace {

TEST(PlayerSerializationTest, SaveAndLoadRoundTrip)
{
    Player original(4242, "serialize_user");
    original.level = 23;
    original.vipLevel = 3;
    original.modifyAttribute(PlayerAttrType::STAMINA, 55);
    original.modifyAttribute(PlayerAttrType::ENERGY, 12);
    original.addResource(1001, 900000);
    original.addCurrency(CurrencyType::SOFT, 12345);
    original.addCurrency(CurrencyType::PREMIUM, 77);
    original.recordShopPurchase(5001, 2);

    Character hero;
    hero.id = 10001;
    hero.name = "hero";
    hero.level = 30;
    hero.quality = QualityType::PURPLE;
    hero.position = Position::WARRIOR;
    original.addCharacter(hero);
    original.mainCharacter = original.getCharacter(hero.id);
    original.addToBattleTeam(hero.id);
    original.combatPower = 54321;

    ASSERT_TRUE(original.saveDataToServer());

    Player loaded(4242, "placeholder");
    ASSERT_TRUE(loaded.loadDataFromServer());

    EXPECT_EQ(loaded.id, original.id);
    EXPECT_EQ(loaded.name, original.name);
    EXPECT_EQ(loaded.level, original.level);
    EXPECT_EQ(loaded.vipLevel, original.vipLevel);
    EXPECT_EQ(loaded.getAttribute(PlayerAttrType::STAMINA), original.getAttribute(PlayerAttrType::STAMINA));
    EXPECT_EQ(loaded.getAttribute(PlayerAttrType::ENERGY), original.getAttribute(PlayerAttrType::ENERGY));
    EXPECT_EQ(loaded.getResource(1001), original.getResource(1001));
    EXPECT_EQ(loaded.getCurrency(CurrencyType::SOFT), original.getCurrency(CurrencyType::SOFT));
    EXPECT_EQ(loaded.getCurrency(CurrencyType::PREMIUM), original.getCurrency(CurrencyType::PREMIUM));
    EXPECT_EQ(loaded.getShopPurchaseCount(5001), original.getShopPurchaseCount(5001));

    ASSERT_TRUE(loaded.hasCharacter(hero.id));
    ASSERT_NE(loaded.getCharacter(hero.id), nullptr);
    ASSERT_EQ(loaded.mainCharacter, loaded.getCharacter(hero.id));
    ASSERT_EQ(loaded.battleTeam.size(), 1u);
    EXPECT_EQ(loaded.battleTeam.front(), hero.id);
    EXPECT_EQ(loaded.combatPower, original.combatPower);

    std::filesystem::remove_all(std::filesystem::path("build") / "player_data");
}

} // namespace
