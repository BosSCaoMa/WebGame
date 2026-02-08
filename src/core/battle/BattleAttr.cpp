#include "BattleAttr.h"
#include <cstddef>
#include <cstdint>

namespace {
constexpr int kQualityExpandRates[] = {0, 15, 40, 80, 100, 150, 200, 300};
constexpr std::size_t kQualityExpandRatesCount = sizeof(kQualityExpandRates) / sizeof(kQualityExpandRates[0]);
constexpr int kLevelGrowthPercent = 3; // 每级统一成长百分比

constexpr int qualityRate(QualityType quality)
{
    const auto index = static_cast<std::size_t>(quality);
    return index < kQualityExpandRatesCount ? kQualityExpandRates[index] : kQualityExpandRates[0];
}
} // namespace

void BattleAttr::InitAttr(Position pos, QualityType quality)
{
    BattleAttr attrs;
    switch (pos) {
        case Position::WARRIOR:
            attrs = warriorAttrs();
            break;
        case Position::MAGE:
            attrs = mageAttrs();
            break;
        case Position::TANK:
            attrs = tankAttrs();
            break;
        case Position::HEALER:
            attrs = healerAttrs();
            break;
        case Position::ASSASSIN:
            attrs = assassinAttrs();
            break;
        default:
            attrs = BattleAttr(); // 默认属性
            break;
    }
    *this = attrs;
    ExPandByQuality(quality);
}
BattleAttr BattleAttr::warriorAttrs()
{
    BattleAttr attrs(15000, 1200, 200,350);
    return attrs;
}

BattleAttr BattleAttr::mageAttrs()
{
    BattleAttr attrs(10000, 1500, 80, 300);
    return attrs;
}

BattleAttr BattleAttr::tankAttrs()
{
    BattleAttr attrs(20000, 500, 500, 280);
    return attrs;
}

BattleAttr BattleAttr::healerAttrs()
{
    BattleAttr attrs(12000, 800, 100, 320);
    return attrs;
}

BattleAttr BattleAttr::assassinAttrs()
{
    BattleAttr attrs(9000, 1600, 70, 400);
    return attrs;
}

void BattleAttr::ExPandByQuality(QualityType quality)
{
    const int rate = qualityRate(quality);
    auto scale = [rate](Scalar& value) {
        value += value * rate / 100;
    };

    scale(hp);
    scale(maxHp);
    scale(atk);
    scale(def);
    scale(speed);
}

void BattleAttr::upgradeByLevel(int level)
{
    hp += hp * kLevelGrowthPercent / 100 * level;
    maxHp += maxHp * kLevelGrowthPercent / 100 * level;
    atk += atk * kLevelGrowthPercent / 100 * level;
    def += def * kLevelGrowthPercent / 100 * level;
    speed += speed * kLevelGrowthPercent / 100 * level;
}

BattleAttr& BattleAttr::operator+=(const BattleAttr& other)
{
    hp += other.hp;
    maxHp += other.maxHp;
    atk += other.atk;
    def += other.def;
    speed += other.speed;

    critRate += other.critRate;
    critDamage += other.critDamage;
    critResist += other.critResist;
    hitRate += other.hitRate;
    dodgeRate += other.dodgeRate;
    rage += other.rage;

    damageBonus += other.damageBonus;
    damageReduction += other.damageReduction;
    skillDamage += other.skillDamage;

    lifesteal += other.lifesteal;
    counterRate += other.counterRate;
    multiHitRate += other.multiHitRate;
    healBonus += other.healBonus;

    for (std::uint8_t idx = 0; idx < static_cast<std::uint8_t>(Resistance::Count); ++idx) {
        const auto resistanceType = static_cast<Resistance>(idx);
        resistance(resistanceType) += other.getResistance(resistanceType);
    }
    return *this;
}

std::uint64_t BattleAttr::calculateCombatPower() const
{
    // 局部工具函数：将数值稳定在[minValue, maxValue]范围内，防止极端输入破坏整体尺度
    auto clampInt = [](int value, int minValue, int maxValue) {
        if (value < minValue) {
            return minValue;
        }
        if (value > maxValue) {
            return maxValue;
        }
        return value;
    };

    // 当前生命值与最大生命值组合后形成的基础生存体量（避免血线刚被削减时战力骤降）。
    const std::uint64_t averageHp = static_cast<std::uint64_t>(hp + maxHp) / 2;

    // ===== 1. 进攻能力（乘法链路，直接映射至64位空间） =====
    const int totalDamageBonus = clampInt(damageBonus + skillDamage, -80, 600);
    const std::uint64_t damageScale = static_cast<std::uint64_t>(100 + totalDamageBonus); // 20%~700%

    const std::uint64_t critRateEffective = static_cast<std::uint64_t>(clampInt(critRate, 0, 100));
    const std::uint64_t critDamageEffective = static_cast<std::uint64_t>(clampInt(critDamage, 100, 500)); // 100%~500%
    const std::uint64_t critScale = 100 + critRateEffective * (critDamageEffective - 100) / 100; // 加成范围 100~500

    const std::uint64_t speedScale = 100 + static_cast<std::uint64_t>(clampInt(speed, 0, 1500)) * 2; // 100~3100

    const std::uint64_t offensiveScore =
        static_cast<std::uint64_t>(atk > 0 ? atk : 0) * damageScale * critScale * speedScale / 1'000'000ULL;

    // ===== 2. 生存能力（等效生命值 * 减伤 * 续航） =====
    const std::uint64_t mitigationScale = 100 + static_cast<std::uint64_t>(clampInt(damageReduction, 0, 90));
    const std::uint64_t sustainScale = 100 + static_cast<std::uint64_t>(clampInt(lifesteal + healBonus, 0, 300));
    const std::uint64_t defenseScale = 100 + static_cast<std::uint64_t>(clampInt(def, 0, 5000)) / 2; // 防御折算为额外生命

    const std::uint64_t effectiveHp = averageHp * mitigationScale * sustainScale / 10'000ULL;
    const std::uint64_t defensiveScore = effectiveHp + averageHp * defenseScale / 1'000ULL;

    // ===== 3. 抗性（控制对策直接乘以生存体量） =====
    std::uint64_t accumulatedResistance = 0;
    for (std::uint8_t idx = 0; idx < static_cast<std::uint8_t>(Resistance::Count); ++idx) {
        const auto type = static_cast<Resistance>(idx);
        accumulatedResistance += static_cast<std::uint64_t>(clampInt(getResistance(type), 0, 100));
    }
    const std::uint64_t resistanceScore = averageHp * accumulatedResistance / 500ULL; // 每1点抗性按HP折算

    // ===== 4. 功能性（节奏与稳定性奖励） =====
    const std::uint64_t utilityRates = static_cast<std::uint64_t>(clampInt(counterRate + multiHitRate + dodgeRate, 0, 250));
    const std::uint64_t utilityScore =
        utilityRates * (static_cast<std::uint64_t>(speed) + 100) * 4 +
        static_cast<std::uint64_t>(rage) * 2'500ULL +
        static_cast<std::uint64_t>(critResist) * 120 +
        static_cast<std::uint64_t>(hitRate) * 80;

    // ===== 5. 总战力 =====
    return offensiveScore + defensiveScore + resistanceScore + utilityScore;
}