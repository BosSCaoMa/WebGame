const enums = {
    skillTriggers: [
        { value: "NORMAL_ATTACK", label: "普通攻击" },
        { value: "RAGE_SKILL", label: "怒气技能" },
        { value: "ON_SAME_CAMP", label: "合击技能" },
        { value: "BATTLE_START", label: "战斗开始" },
        { value: "ROUND_START", label: "回合开始" },
        { value: "ROUND_END", label: "回合结束" },
        { value: "TURN_START", label: "行动开始" },
        { value: "TURN_END", label: "行动结束" },
        { value: "ROUND_ODD", label: "奇数回合" },
        { value: "ROUND_EVEN", label: "偶数回合" },
        { value: "ROUND_MULTIPLE_OF_FIVE", label: "每 5 回合" },
        { value: "ROUND_TEN", label: "第 10 回合" },
        { value: "ON_HIT", label: "受到攻击" },
        { value: "ON_CONTROL", label: "受到控制" },
        { value: "ON_KILL", label: "造成击杀" },
        { value: "ON_DEATH", label: "阵亡触发" },
        { value: "None", label: "无" }
    ],
    targetTypes: [
        { value: "SELF", label: "自身" },
        { value: "ALLY_ALL", label: "己方全体" },
        { value: "ENEMY_SINGLE", label: "敌方单体" },
        { value: "ENEMY_COL", label: "敌方同列" },
        { value: "ENEMY_ALL", label: "敌方全体" },
        { value: "ALLY_FRONT_ROW", label: "己方前排" },
        { value: "ALLY_BACK_ROW", label: "己方后排" },
        { value: "ENEMY_FRONT_ROW", label: "敌方前排" },
        { value: "ENEMY_BACK_ROW", label: "敌方后排" },
        { value: "ALLY_ATK_TOP1", label: "己方攻最高 1 人" },
        { value: "ALLY_ATK_TOP2", label: "己方攻最高 2 人" },
        { value: "ALLY_ATK_TOP3", label: "己方攻最高 3 人" },
        { value: "ENEMY_ATK_TOP1", label: "敌方攻最高 1 人" },
        { value: "ENEMY_ATK_TOP2", label: "敌方攻最高 2 人" },
        { value: "ENEMY_ATK_TOP3", label: "敌方攻最高 3 人" },
        { value: "ALLY_HP_LOW1", label: "己方血最低 1 人" },
        { value: "ALLY_HP_LOW2", label: "己方血最低 2 人" },
        { value: "ALLY_HP_LOW3", label: "己方血最低 3 人" },
        { value: "ENEMY_HP_LOW1", label: "敌方血最低 1 人" },
        { value: "ENEMY_HP_LOW2", label: "敌方血最低 2 人" },
        { value: "ENEMY_HP_LOW3", label: "敌方血最低 3 人" },
        { value: "ALLY_RANDOM_1", label: "己方随机 1 人" },
        { value: "ALLY_RANDOM_2", label: "己方随机 2 人" },
        { value: "ALLY_RANDOM_3", label: "己方随机 3 人" },
        { value: "ENEMY_RANDOM_1", label: "敌方随机 1 人" },
        { value: "ENEMY_RANDOM_2", label: "敌方随机 2 人" },
        { value: "ENEMY_RANDOM_3", label: "敌方随机 3 人" }
    ],
    effectTypes: [
        { value: "NONE", label: "无" },
        { value: "DAMAGE", label: "伤害" },
        { value: "PIERCE", label: "穿透" },
        { value: "TRUE_DAMAGE", label: "真实伤害" },
        { value: "HEAL", label: "治疗" },
        { value: "RAGE_CHANGE", label: "怒气变动" },
        { value: "DIVINE_POWER", label: "神威值" },
        { value: "SHIELD", label: "护盾" },
        { value: "BARRIER", label: "屏障" },
        { value: "BUFF_MAX_HP", label: "最大生命提升" },
        { value: "BUFF_ATK", label: "攻击提升" }, // todo
        { value: "BUFF_DEF", label: "防御提升" },
        { value: "BUFF_SPEED", label: "速度提升" },
        { value: "BUFF_CRIT_RATE", label: "暴击率提升" },
        { value: "BUFF_CRIT_RESIST", label: "抗暴率提升" },
        { value: "BUFF_HIT_RATE", label: "命中率提升" },
        { value: "BUFF_DODGE_RATE", label: "闪避率提升" },
        { value: "BUFF_REGEN", label: "持续回血" },
        { value: "STUN", label: "眩晕" },
        { value: "FREEZE", label: "冰冻" },
        { value: "SILENCE", label: "沉默" },
        { value: "TAUNT", label: "嘲讽" },
        { value: "INJURY", label: "受伤" },
        { value: "POISON", label: "中毒" },
        { value: "BURN", label: "灼烧" },
        { value: "BLEED", label: "流血" },
        { value: "CURSE", label: "诅咒" },
        { value: "LOCK_BLEED", label: "锁血" },
        { value: "REVIVE", label: "复活" },
        { value: "DISPEL", label: "驱散" },
        { value: "CLEANSE", label: "净化" },
        { value: "IMMUNITY", label: "免疫" },
        { value: "INVINCIBLE", label: "无敌" },
        { value: "TRANSFER_DEBUFF", label: "负面转移" },
        { value: "MARK_HEAL", label: "治疗标记" },
        { value: "MARK_DAMAGE", label: "伤害标记" },
        { value: "MARK_KILL", label: "斩杀标记" },
        { value: "MARK_PROTECT", label: "保护标记" }
    ],
    valueSources: [
        { value: "FIXED", label: "固定值" },
        { value: "ATK", label: "攻击力" },
        { value: "DEF", label: "防御力" },
        { value: "MAX_HP", label: "最大生命" },
        { value: "CUR_HP", label: "当前生命" },
        { value: "LOST_HP", label: "已损生命" }
    ],
    valueOwners: [
        { value: "CASTER", label: "施放者" },
        { value: "TARGET", label: "目标" }
    ],
    valueScales: [
        { value: "ABSOLUTE", label: "绝对值" },
        { value: "PERCENT", label: "百分比(万分比)" }
    ],
    qualityTypes: [
        { value: "WHITE", label: "白" },
        { value: "GREEN", label: "绿" },
        { value: "BLUE", label: "蓝" },
        { value: "PURPLE", label: "紫" },
        { value: "ORANGE", label: "橙" },
        { value: "RED", label: "红" },
        { value: "GOLD", label: "金" }
    ],
    positions: [
        { value: "WARRIOR", label: "战士" },
        { value: "MAGE", label: "法师" },
        { value: "TANK", label: "坦克" },
        { value: "HEALER", label: "治疗" },
        { value: "ASSASSIN", label: "刺客" }
    ],
    equipmentTypes: [
        { value: "WEAPON", label: "武器" },
        { value: "ARMOR", label: "护甲" },
        { value: "HELMET", label: "头盔" },
        { value: "BOOTS", label: "战靴" },
        { value: "STEED", label: "坐骑" },
        { value: "TALLY", label: "兵符" },
        { value: "TREASURE", label: "法宝" },
        { value: "FAMOUS", label: "名将" }
    ],
    itemTypes: [
        { value: "CONSUMABLE", label: "消耗品" },
        { value: "MATERIAL", label: "材料" },
        { value: "TREASURE", label: "宝物" },
        { value: "EQUIPMENT", label: "装备" },
        { value: "CURRENCY", label: "货币" },
        { value: "NONE", label: "其它" }
    ],
    consumableTypes: [
        { value: "NONE", label: "无" },
        { value: "HP_POTION", label: "生命药剂" },
        { value: "RAGE_POTION", label: "怒气药剂" },
        { value: "EXP_POTION", label: "经验药剂" },
        { value: "BUFF_POTION", label: "增益药剂" }
    ],
    materialTypes: [
        { value: "NONE", label: "通用" },
        { value: "CRAFT", label: "打造材料" },
        { value: "UPGRADE", label: "强化材料" },
        { value: "QUEST", label: "任务道具" },
        { value: "EVENT", label: "活动限定" }
    ],
    currencyTypes: [
        { value: "UNKNOWN", label: "未知" },
        { value: "SOFT", label: "金币/通用" },
        { value: "PREMIUM", label: "付费货币" },
        { value: "BOUND_PREMIUM", label: "绑定货币" },
        { value: "TOKEN", label: "活动代币" }
    ]
};

const getEnumOptions = (key) => enums[key] || [];
const getEnumDefault = (key) => getEnumOptions(key)[0]?.value ?? "";
const getEnumLabel = (key, value) => {
    const option = getEnumOptions(key).find((opt) => opt.value === value);
    return option ? option.label : value;
};
const normalize = (text) => (text ?? '').toString().toLowerCase();

// 保持技能效果结构一致，便于在 UI 中动态生成字段
const createEffectTemplate = () => ({
    target: getEnumDefault('targetTypes'),
    effect: getEnumDefault('effectTypes'),
    valueExpr: {
        source: getEnumDefault('valueSources'),
        owner: getEnumDefault('valueOwners'),
        scale: getEnumDefault('valueScales'),
        value: 0
    },
    duration: 0,
    chance: 100,
    canOverlay: false
});

const createItemEffectTemplate = () => ({
    type: getEnumDefault('effectTypes'),
    value: 0,
    owner: getEnumDefault('valueOwners'),
    scale: getEnumDefault('valueScales'),
    duration: 0
});

const createBattleAttrTemplate = () => ({
    hp: 0,
    maxHp: 0,
    atk: 0,
    def: 0,
    speed: 0,
    critRate: 0,
    critDamage: 0,
    hitRate: 0,
    dodgeRate: 0
});

const createSetPieceTemplate = () => ({
    pieceCount: 2,
    desc: "",
    skillId: 0,
    applyAttr: createBattleAttrTemplate()
});

const createStackRuleTemplate = () => ({
    max: 1,
    unique: false,
    autoSplit: true
});

const createOwnershipTemplate = () => ({
    tradable: true,
    bindOnPickup: false,
    bindOnEquip: false
});

const createConsumableProfile = () => ({
    category: getEnumDefault('consumableTypes'),
    cooldownSec: 0,
    destroyOnUse: true,
    effects: [createItemEffectTemplate()]
});

const createMaterialProfile = () => ({
    category: getEnumDefault('materialTypes'),
    tier: 0,
    usage: ""
});

const createCurrencyProfile = () => ({
    category: getEnumDefault('currencyTypes'),
    precision: 1,
    sharedAcrossRoles: true,
    allowNegative: false
});

// 每种数据类型的默认模板，确保新增条目字段完整
const templates = {
    skills: () => ({
        id: 0,
        name: "",
        description: "",
        trigger: getEnumDefault('skillTriggers'),
        effects: [createEffectTemplate()]
    }),
    characters: () => ({
        id: 0,
        relId: 0,
        name: "",
        baseName: "",
        quality: getEnumDefault('qualityTypes'),
        position: getEnumDefault('positions'),
        skillIds: []
    }),
    equipments: () => ({
        id: 0,
        name: "",
        type: getEnumDefault('equipmentTypes'),
        quality: getEnumDefault('qualityTypes'),
        skillId: 0,
        setId: 0,
        baseAttrs: createBattleAttrTemplate()
    }),
    setBonuses: () => ({
        setId: 0,
        name: "",
        bonuses: [createSetPieceTemplate()]
    }),
    items: () => ({
        id: 0,
        name: "",
        type: getEnumDefault('itemTypes'),
        subType: getEnumDefault('consumableTypes'),
        quality: 0,
        description: "",
        maxStack: 1,
        tags: [],
        stack: createStackRuleTemplate(),
        ownership: createOwnershipTemplate(),
        consumable: createConsumableProfile(),
        material: createMaterialProfile(),
        currency: createCurrencyProfile()
    })
};

const presetLibrary = {
    skills: [
        {
            title: '烈焰破军',
            detail: '怒气爆发群攻，对全体造成高额伤害并附带灼烧。',
            meta: '怒气技能 · 群体灼烧',
            data: {
                id: 4101,
                name: '烈焰破军',
                description: '挥出灼热刀光，对敌方全体造成自身攻击力加成的伤害并附加灼烧。',
                trigger: 'RAGE_SKILL',
                effects: [
                    {
                        target: 'ENEMY_ALL',
                        effect: 'DAMAGE',
                        valueExpr: { source: 'ATK', owner: 'CASTER', scale: 'PERCENT', value: 2200 },
                        duration: 0,
                        chance: 100,
                        canOverlay: false
                    },
                    {
                        target: 'ENEMY_ALL',
                        effect: 'BURN',
                        valueExpr: { source: 'ATK', owner: 'CASTER', scale: 'PERCENT', value: 70 },
                        duration: 2,
                        chance: 85,
                        canOverlay: true
                    }
                ]
            }
        },
        {
            title: '真武穿刺',
            detail: '普通攻击替换为多段穿刺，优先后排。',
            meta: '普通攻击 · 穿透',
            data: {
                id: 4102,
                name: '真武穿刺',
                description: '附带雷霆之力击穿敌阵，对后排造成额外穿透伤害。',
                trigger: 'NORMAL_ATTACK',
                effects: [
                    {
                        target: 'ENEMY_BACK_ROW',
                        effect: 'DAMAGE',
                        valueExpr: { source: 'ATK', owner: 'CASTER', scale: 'PERCENT', value: 1500 },
                        duration: 0,
                        chance: 100,
                        canOverlay: false
                    },
                    {
                        target: 'ENEMY_SINGLE',
                        effect: 'PIERCE',
                        valueExpr: { source: 'ATK', owner: 'CASTER', scale: 'PERCENT', value: 600 },
                        duration: 0,
                        chance: 100,
                        canOverlay: false
                    }
                ]
            }
        },
        {
            title: '星陨庇佑',
            detail: '战斗开始为友军护盾并提升防御。',
            meta: '开局 · 护盾辅助',
            data: {
                id: 4103,
                name: '星陨庇佑',
                description: '星辉坠落庇护友军，立刻生成护盾并短时间增防。',
                trigger: 'BATTLE_START',
                effects: [
                    {
                        target: 'ALLY_ALL',
                        effect: 'SHIELD',
                        valueExpr: { source: 'MAX_HP', owner: 'CASTER', scale: 'PERCENT', value: 1100 },
                        duration: 2,
                        chance: 100,
                        canOverlay: false
                    },
                    {
                        target: 'ALLY_ALL',
                        effect: 'BUFF_DEF',
                        valueExpr: { source: 'DEF', owner: 'CASTER', scale: 'PERCENT', value: 1800 },
                        duration: 2,
                        chance: 100,
                        canOverlay: false
                    }
                ]
            }
        },
        {
            title: '玄冰束缚',
            detail: '指定单体提供控制与真实伤害，适合控制位。',
            meta: '行动结束 · 控制',
            data: {
                id: 4104,
                name: '玄冰束缚',
                description: '寒霜锁链禁锢敌人，短暂冻结并造成真实伤害。',
                trigger: 'TURN_END',
                effects: [
                    {
                        target: 'ENEMY_SINGLE',
                        effect: 'FREEZE',
                        valueExpr: { source: 'FIXED', owner: 'CASTER', scale: 'ABSOLUTE', value: 0 },
                        duration: 1,
                        chance: 65,
                        canOverlay: false
                    },
                    {
                        target: 'ENEMY_SINGLE',
                        effect: 'TRUE_DAMAGE',
                        valueExpr: { source: 'ATK', owner: 'CASTER', scale: 'PERCENT', value: 1200 },
                        duration: 0,
                        chance: 100,
                        canOverlay: false
                    }
                ]
            }
        },
        {
            title: '灵泉再生',
            detail: '奇数回合为血量最低单位回血并附带持续恢复。',
            meta: '奇数回合 · 治疗',
            data: {
                id: 4105,
                name: '灵泉再生',
                description: '唤醒灵泉滋养，自动照顾血量最低的友军。',
                trigger: 'ROUND_ODD',
                effects: [
                    {
                        target: 'ALLY_HP_LOW2',
                        effect: 'HEAL',
                        valueExpr: { source: 'ATK', owner: 'CASTER', scale: 'PERCENT', value: 1800 },
                        duration: 0,
                        chance: 100,
                        canOverlay: false
                    },
                    {
                        target: 'ALLY_HP_LOW2',
                        effect: 'BUFF_REGEN',
                        valueExpr: { source: 'MAX_HP', owner: 'TARGET', scale: 'PERCENT', value: 180 },
                        duration: 2,
                        chance: 100,
                        canOverlay: false
                    }
                ]
            }
        },
        {
            title: '逐日激励',
            detail: '每回合开始提升全体进攻并注入怒气。',
            meta: '回合开始 · 强化',
            data: {
                id: 4106,
                name: '逐日激励',
                description: '旭日加持军心，开局即提升进攻节奏。',
                trigger: 'ROUND_START',
                effects: [
                    {
                        target: 'ALLY_ALL',
                        effect: 'BUFF_ATK',
                        valueExpr: { source: 'ATK', owner: 'CASTER', scale: 'PERCENT', value: 1600 },
                        duration: 2,
                        chance: 100,
                        canOverlay: false
                    },
                    {
                        target: 'ALLY_ALL',
                        effect: 'RAGE_CHANGE',
                        valueExpr: { source: 'FIXED', owner: 'CASTER', scale: 'ABSOLUTE', value: 20 },
                        duration: 0,
                        chance: 100,
                        canOverlay: false
                    }
                ]
            }
        },
        {
            title: '苍狼怒袭',
            detail: '击杀后触发追击并提升自身暴击，适配收割位。',
            meta: '击杀触发 · 收割',
            data: {
                id: 4107,
                name: '苍狼怒袭',
                description: '苍狼信仰加持，斩杀瞬间再次撕裂敌阵。',
                trigger: 'ON_KILL',
                effects: [
                    {
                        target: 'ENEMY_RANDOM_2',
                        effect: 'DAMAGE',
                        valueExpr: { source: 'ATK', owner: 'CASTER', scale: 'PERCENT', value: 1900 },
                        duration: 0,
                        chance: 100,
                        canOverlay: false
                    },
                    {
                        target: 'SELF',
                        effect: 'BUFF_CRIT_RATE',
                        valueExpr: { source: 'ATK', owner: 'CASTER', scale: 'PERCENT', value: 900 },
                        duration: 2,
                        chance: 100,
                        canOverlay: false
                    }
                ]
            }
        }
    ],
    characters: [
        {
            title: '破阵先锋·战士',
            detail: '擅长冲阵与多段输出，适合作为前排爆发。',
            meta: '战士 · 前排输出',
            data: {
                id: 6201,
                relId: 0,
                name: '破阵先锋',
                baseName: '破阵营',
                quality: 'ORANGE',
                position: 'WARRIOR',
                skillIds: []
            }
        },
        {
            title: '灵息召师·法师',
            detail: '高爆发远程法师，偏好群攻与持续灼烧。',
            meta: '法师 · 群体火力',
            data: {
                id: 6202,
                relId: 0,
                name: '灵息召师',
                baseName: '灵息殿',
                quality: 'RED',
                position: 'MAGE',
                skillIds: []
            }
        },
        {
            title: '玄甲壁垒·坦克',
            detail: '自带援护的护盾坦克，适配控制/防守技能。',
            meta: '坦克 · 守护',
            data: {
                id: 6203,
                relId: 0,
                name: '玄甲壁垒',
                baseName: '玄甲营',
                quality: 'PURPLE',
                position: 'TANK',
                skillIds: []
            }
        },
        {
            title: '星辉侍女·治疗',
            detail: '偏向持续疗愈的辅助，适合奇数回合回复类技能。',
            meta: '治疗 · 持续恢复',
            data: {
                id: 6204,
                relId: 0,
                name: '星辉侍女',
                baseName: '星辉殿',
                quality: 'GOLD',
                position: 'HEALER',
                skillIds: []
            }
        },
        {
            title: '影牙游侠·刺客',
            detail: '高机动收割者，偏好击杀触发或穿刺类技能。',
            meta: '刺客 · 收割',
            data: {
                id: 6205,
                relId: 0,
                name: '影牙游侠',
                baseName: '影牙堂',
                quality: 'ORANGE',
                position: 'ASSASSIN',
                skillIds: []
            }
        }
    ],
    equipments: [
        {
            title: '赤焰龙吟戟',
            detail: '武器模板：高攻击与暴击，适配怒气输出。',
            meta: '武器 · 爆发',
            data: {
                id: 7301,
                name: '赤焰龙吟戟',
                type: 'WEAPON',
                quality: 'ORANGE',
                skillId: 0,
                setId: 0,
                baseAttrs: {
                    hp: 0,
                    maxHp: 0,
                    atk: 320,
                    def: 60,
                    speed: 0,
                    critRate: 180,
                    critDamage: 240,
                    hitRate: 70,
                    dodgeRate: 0
                }
            }
        },
        {
            title: '星辉守御铠',
            detail: '护甲模板：高生命与防御，兼顾命中。',
            meta: '护甲 · 辅助',
            data: {
                id: 7302,
                name: '星辉守御铠',
                type: 'ARMOR',
                quality: 'PURPLE',
                skillId: 0,
                setId: 0,
                baseAttrs: {
                    hp: 0,
                    maxHp: 6400,
                    atk: 0,
                    def: 320,
                    speed: 0,
                    critRate: 0,
                    critDamage: 0,
                    hitRate: 60,
                    dodgeRate: 40
                }
            }
        },
        {
            title: '玄霜战冠',
            detail: '头盔模板：提供均衡生存与命中。',
            meta: '头盔 · 防御',
            data: {
                id: 7303,
                name: '玄霜战冠',
                type: 'HELMET',
                quality: 'BLUE',
                skillId: 0,
                setId: 0,
                baseAttrs: {
                    hp: 0,
                    maxHp: 3600,
                    atk: 0,
                    def: 210,
                    speed: 0,
                    critRate: 0,
                    critDamage: 0,
                    hitRate: 35,
                    dodgeRate: 65
                }
            }
        },
        {
            title: '霜影飞靴',
            detail: '战靴模板：速度与命中双加成。',
            meta: '战靴 · 速度',
            data: {
                id: 7304,
                name: '霜影飞靴',
                type: 'BOOTS',
                quality: 'RED',
                skillId: 0,
                setId: 0,
                baseAttrs: {
                    hp: 0,
                    maxHp: 2200,
                    atk: 0,
                    def: 110,
                    speed: 55,
                    critRate: 0,
                    critDamage: 0,
                    hitRate: 95,
                    dodgeRate: 120
                }
            }
        },
        {
            title: '逐日龙驹',
            detail: '坐骑模板：提供速度、命中与少量生命。',
            meta: '坐骑 · 机动',
            data: {
                id: 7305,
                name: '逐日龙驹',
                type: 'STEED',
                quality: 'GOLD',
                skillId: 0,
                setId: 0,
                baseAttrs: {
                    hp: 0,
                    maxHp: 3000,
                    atk: 80,
                    def: 90,
                    speed: 80,
                    critRate: 0,
                    critDamage: 0,
                    hitRate: 70,
                    dodgeRate: 60
                }
            }
        },
        {
            title: '虎符令牌',
            detail: '兵符模板：偏向命中与暴击，用于策略位。',
            meta: '兵符 · 精准',
            data: {
                id: 7306,
                name: '虎符令牌',
                type: 'TALLY',
                quality: 'ORANGE',
                skillId: 0,
                setId: 0,
                baseAttrs: {
                    hp: 0,
                    maxHp: 0,
                    atk: 140,
                    def: 0,
                    speed: 0,
                    critRate: 120,
                    critDamage: 160,
                    hitRate: 110,
                    dodgeRate: 30
                }
            }
        },
        {
            title: '玄火灵宝',
            detail: '法宝模板：兼具攻击、暴击、怒气功能。',
            meta: '法宝 · 术法',
            data: {
                id: 7307,
                name: '玄火灵宝',
                type: 'TREASURE',
                quality: 'RED',
                skillId: 0,
                setId: 0,
                baseAttrs: {
                    hp: 0,
                    maxHp: 1800,
                    atk: 200,
                    def: 80,
                    speed: 0,
                    critRate: 150,
                    critDamage: 200,
                    hitRate: 75,
                    dodgeRate: 0
                }
            }
        },
        {
            title: '名将英魂',
            detail: '名将卡槽模板：高综合属性，适配核心队伍。',
            meta: '名将 · 全面',
            data: {
                id: 7308,
                name: '名将英魂',
                type: 'FAMOUS',
                quality: 'GOLD',
                skillId: 0,
                setId: 0,
                baseAttrs: {
                    hp: 0,
                    maxHp: 5200,
                    atk: 210,
                    def: 160,
                    speed: 35,
                    critRate: 110,
                    critDamage: 160,
                    hitRate: 90,
                    dodgeRate: 90
                }
            }
        }
    ],
    setBonuses: [
        {
            title: '炎龙之志',
            detail: '双件强化暴击，四件附带怒气灼烧技能。',
            meta: '2+4 件 · 输出',
            data: {
                setId: 5201,
                name: '炎龙之志',
                bonuses: [
                    {
                        pieceCount: 2,
                        desc: '穿戴 2 件时攻击+12%并提升暴击率。',
                        skillId: 0,
                        applyAttr: {
                            hp: 0,
                            maxHp: 0,
                            atk: 260,
                            def: 0,
                            speed: 0,
                            critRate: 150,
                            critDamage: 120,
                            hitRate: 60,
                            dodgeRate: 0
                        }
                    },
                    {
                        pieceCount: 4,
                        desc: '怒气技能附带灼烧与怒气回复。',
                        skillId: 4101,
                        applyAttr: {
                            hp: 0,
                            maxHp: 0,
                            atk: 180,
                            def: 0,
                            speed: 0,
                            critRate: 0,
                            critDamage: 240,
                            hitRate: 0,
                            dodgeRate: 0
                        }
                    }
                ]
            }
        },
        {
            title: '苍穹疾风',
            detail: '2 件提升速度，4 件额外获得追击技能。',
            meta: '2+4 件 · 机动',
            data: {
                setId: 5202,
                name: '苍穹疾风',
                bonuses: [
                    {
                        pieceCount: 2,
                        desc: '移动加成并提升命中，适合先手。',
                        skillId: 0,
                        applyAttr: {
                            hp: 0,
                            maxHp: 0,
                            atk: 120,
                            def: 0,
                            speed: 60,
                            critRate: 0,
                            critDamage: 0,
                            hitRate: 120,
                            dodgeRate: 40
                        }
                    },
                    {
                        pieceCount: 4,
                        desc: '触发追击（苍狼怒袭）并获得少量暴击。',
                        skillId: 4107,
                        applyAttr: {
                            hp: 0,
                            maxHp: 0,
                            atk: 150,
                            def: 0,
                            speed: 30,
                            critRate: 90,
                            critDamage: 140,
                            hitRate: 0,
                            dodgeRate: 0
                        }
                    }
                ]
            }
        }
    ],
    items: [
        {
            title: '星辉疗愈药剂',
            detail: '即时回复大量生命的消耗品，标准 HP 药剂模板。',
            meta: '消耗品 · 生命药剂',
            data: {
                id: 8101,
                name: '星辉疗愈药剂',
                type: 'CONSUMABLE',
                subType: 'HP_POTION',
                quality: 3,
                description: '战斗中立即恢复使用者生命值。',
                maxStack: 20,
                tags: ['治疗', '战斗'],
                stack: { max: 20, unique: false, autoSplit: true },
                ownership: { tradable: true, bindOnPickup: false, bindOnEquip: false },
                consumable: {
                    category: 'HP_POTION',
                    cooldownSec: 0,
                    destroyOnUse: true,
                    effects: [
                        { type: 'HEAL', value: 4500, owner: 'TARGET', scale: 'ABSOLUTE', duration: 0 }
                    ]
                }
            }
        },
        {
            title: '怒涛战鼓',
            detail: '怒气恢复型消耗品，开启后立即回满怒气。',
            meta: '消耗品 · 怒气药剂',
            data: {
                id: 8102,
                name: '怒涛战鼓',
                type: 'CONSUMABLE',
                subType: 'RAGE_POTION',
                quality: 4,
                description: '振奋士气，瞬间回复怒气并提升下一次技能威力。',
                maxStack: 10,
                tags: ['怒气', '增益'],
                stack: { max: 10, unique: false, autoSplit: true },
                ownership: { tradable: true, bindOnPickup: false, bindOnEquip: false },
                consumable: {
                    category: 'RAGE_POTION',
                    cooldownSec: 0,
                    destroyOnUse: true,
                    effects: [
                        { type: 'RAGE_CHANGE', value: 100, owner: 'TARGET', scale: 'ABSOLUTE', duration: 0 },
                        { type: 'BUFF_ATK', value: 800, owner: 'TARGET', scale: 'ABSOLUTE', duration: 1 }
                    ]
                }
            }
        },
        {
            title: '玄铁铸锭',
            detail: '常见装备强化材料，用于升级装备等级。',
            meta: '材料 · 强化用',
            data: {
                id: 8103,
                name: '玄铁铸锭',
                type: 'MATERIAL',
                subType: 'NONE',
                quality: 2,
                description: '兵工坊需求量最高的基础强化材料。',
                maxStack: 999,
                tags: ['装备', '强化'],
                stack: { max: 999, unique: false, autoSplit: true },
                ownership: { tradable: true, bindOnPickup: false, bindOnEquip: false },
                material: {
                    category: 'UPGRADE',
                    tier: 1,
                    usage: '装备升级和精炼用的基础材料'
                }
            }
        },
        {
            title: '龙鳞秘匣',
            detail: '开启可随机获得稀有装备或宝物。',
            meta: '宝物 · 随机奖励',
            data: {
                id: 8104,
                name: '龙鳞秘匣',
                type: 'TREASURE',
                subType: 'NONE',
                quality: 5,
                description: '远征掉落的宝箱，开启后获得橙品质装备材料。',
                maxStack: 5,
                tags: ['宝箱', '稀有'],
                stack: { max: 5, unique: false, autoSplit: false },
                ownership: { tradable: false, bindOnPickup: true, bindOnEquip: false },
                material: {
                    category: 'EVENT',
                    tier: 3,
                    usage: '开启后随机掉落装备或宝物'
                }
            }
        },
        {
            title: '天工淬炼石',
            detail: '用于装备突破的专属资源。',
            meta: '装备素材 · 突破',
            data: {
                id: 8105,
                name: '天工淬炼石',
                type: 'MATERIAL',
                subType: 'NONE',
                quality: 4,
                description: '可为任意部位装备提供突破进度。',
                maxStack: 50,
                tags: ['突破', '稀有'],
                stack: { max: 50, unique: false, autoSplit: true },
                ownership: { tradable: false, bindOnPickup: true, bindOnEquip: false },
                material: {
                    category: 'UPGRADE',
                    tier: 3,
                    usage: '高级装备突破的专属材料'
                }
            }
        },
        {
            title: '王庭勋章',
            detail: '通用货币，可在王庭商店兑换稀有物品。',
            meta: '货币 · 商店',
            data: {
                id: 8106,
                name: '王庭勋章',
                type: 'CURRENCY',
                subType: 'NONE',
                quality: 1,
                description: '王庭战争委员会发行的通行货币。',
                maxStack: 9999,
                tags: ['王庭', '货币'],
                stack: { max: 9999, unique: false, autoSplit: true },
                ownership: { tradable: false, bindOnPickup: true, bindOnEquip: false },
                currency: {
                    category: 'TOKEN',
                    precision: 1,
                    sharedAcrossRoles: true,
                    allowNegative: false
                }
            }
        },
        {
            title: '风旅凭证',
            detail: '任务道具，可触发特殊剧情或开启副本。',
            meta: '特殊 · 剧情',
            data: {
                id: 8107,
                name: '风旅凭证',
                type: 'NONE',
                subType: 'NONE',
                quality: 2,
                description: '凭证收集齐后可开启风旅试炼。',
                maxStack: 1,
                tags: ['任务', '剧情'],
                stack: { max: 1, unique: true, autoSplit: false },
                ownership: { tradable: false, bindOnPickup: true, bindOnEquip: false },
                material: {
                    category: 'QUEST',
                    tier: 0,
                    usage: '用于解锁风旅试炼剧情'
                }
            }
        }
    ]
};

// 导出/导入的整体 JSON 结构
const createDefaultState = () => ({
    meta: {
        version: 1,
        updatedAt: new Date().toISOString()
    },
    skills: [],
    characters: [],
    equipments: [],
    setBonuses: [],
    items: []
});

const deepClone = (obj) => JSON.parse(JSON.stringify(obj));
const toNumber = (value) => {
    if (value === null || value === undefined || value === '') return 0;
    const parsed = Number(value);
    return Number.isNaN(parsed) ? 0 : parsed;
};

class ConfigEditor {
    constructor() {
        this.state = createDefaultState();
        this.selection = { skills: -1, characters: -1, equipments: -1, setBonuses: -1, items: -1 };
        this.filters = { skills: '', characters: '', equipments: '', setBonuses: '', items: '' };
        this.cacheDom();
        this.bindEvents();
        this.renderAll();
        this.renderPresetRails();
    }

    cacheDom() {
        this.refs = {
            tabs: document.querySelectorAll('[data-tab]'),
            panels: document.querySelectorAll('[data-panel]'),
            statusText: document.getElementById('statusText'),
            importBtn: document.getElementById('triggerImport'),
            importInput: document.getElementById('importFile'),
            exportBtn: document.getElementById('exportJson'),
            counts: {
                skills: document.getElementById('skillCount'),
                characters: document.getElementById('characterCount'),
                equipments: document.getElementById('equipCount'),
                setBonuses: document.getElementById('setBonusCount'),
                items: document.getElementById('itemCount')
            },
            listCounts: {
                skills: document.querySelector('[data-count="skills"]'),
                characters: document.querySelector('[data-count="characters"]'),
                equipments: document.querySelector('[data-count="equipments"]'),
                setBonuses: document.querySelector('[data-count="setBonuses"]'),
                items: document.querySelector('[data-count="items"]')
            },
            lists: {
                skills: document.getElementById('skillsList'),
                characters: document.getElementById('charactersList'),
                equipments: document.getElementById('equipmentsList'),
                setBonuses: document.getElementById('setBonusList'),
                items: document.getElementById('itemsList')
            },
            forms: {
                skills: document.getElementById('skillForm'),
                characters: document.getElementById('characterForm'),
                equipments: document.getElementById('equipmentForm'),
                setBonuses: document.getElementById('setBonusForm'),
                items: document.getElementById('itemForm')
            },
            placeholders: {
                skills: document.getElementById('skillPlaceholder'),
                characters: document.getElementById('characterPlaceholder'),
                equipments: document.getElementById('equipmentPlaceholder'),
                setBonuses: document.getElementById('setBonusPlaceholder'),
                items: document.getElementById('itemPlaceholder')
            },
            addButtons: document.querySelectorAll('[data-add]'),
            deleteButtons: document.querySelectorAll('[data-delete]'),
            addEffectBtn: document.getElementById('addEffect'),
            effectRepeater: document.getElementById('effectRepeater'),
            addItemEffectBtn: document.getElementById('addItemEffect'),
            itemEffectRepeater: document.getElementById('itemEffectRepeater'),
            addSetPieceBtn: document.getElementById('addSetBonusPiece'),
            setBonusPieceRepeater: document.getElementById('setBonusPieceRepeater'),
            filterInputs: document.querySelectorAll('[data-filter]'),
            skillCharacterSelect: document.getElementById('skillCharacterSelect'),
            linkCharacterBtn: document.getElementById('linkCharacterBtn'),
            skillCharacterLinks: document.getElementById('skillCharacterLinks'),
            characterSkillSelect: document.getElementById('characterSkillSelect'),
            addCharacterSkillBtn: document.getElementById('addCharacterSkill'),
            characterSkillChips: document.getElementById('characterSkillChips'),
            presetLists: {
                skills: document.getElementById('skillPresetList'),
                characters: document.getElementById('characterPresetList'),
                equipments: document.getElementById('equipmentPresetList'),
                setBonuses: document.getElementById('setBonusPresetList'),
                items: document.getElementById('itemPresetList')
            }
        };
    }

    bindEvents() {
        this.refs.tabs.forEach((tab) => {
            tab.addEventListener('click', () => this.setActiveTab(tab.dataset.tab));
        });

        this.refs.importBtn.addEventListener('click', () => this.refs.importInput.click());
        this.refs.importInput.addEventListener('change', (event) => this.handleFile(event));
        this.refs.exportBtn.addEventListener('click', () => this.exportJson());

        this.refs.addButtons.forEach((btn) => {
            btn.addEventListener('click', () => this.addEntry(btn.dataset.add));
        });

        this.refs.deleteButtons.forEach((btn) => {
            btn.addEventListener('click', () => this.deleteEntry(btn.dataset.delete));
        });

        this.refs.filterInputs.forEach((input) => {
            input.addEventListener('input', (event) => {
                const type = event.target.dataset.filter;
                this.filters[type] = event.target.value || '';
                this.renderList(type);
            });
        });

        Object.keys(this.refs.lists).forEach((type) => {
            const list = this.refs.lists[type];
            if (!list) return;
            list.addEventListener('click', (event) => {
                const card = event.target.closest('.card');
                if (!card) return;
                this.selection[type] = Number(card.dataset.index);
                this.renderList(type);
                this.renderForm(type);
            });
        });

        Object.entries(this.refs.forms).forEach(([type, form]) => {
            if (!form) return;
            form.addEventListener('submit', (event) => event.preventDefault());
            form.addEventListener('input', (event) => this.handleFormInput(event, type));
        });

        if (this.refs.addEffectBtn) {
            this.refs.addEffectBtn.addEventListener('click', () => this.addEffect());
        }

        if (this.refs.addItemEffectBtn) {
            this.refs.addItemEffectBtn.addEventListener('click', () => this.addItemEffect());
        }

        if (this.refs.addSetPieceBtn) {
            this.refs.addSetPieceBtn.addEventListener('click', () => this.addSetPiece());
        }

        this.refs.forms.skills.addEventListener('click', (event) => {
            const btn = event.target.closest('[data-action="remove-effect"]');
            if (!btn) return;
            const index = Number(btn.dataset.index);
            this.removeEffect(index);
        });

        if (this.refs.forms.items) {
            this.refs.forms.items.addEventListener('click', (event) => {
                const btn = event.target.closest('[data-action="remove-item-effect"]');
                if (!btn) return;
                const index = Number(btn.dataset.index);
                this.removeItemEffect(index);
            });
        }

        if (this.refs.forms.setBonuses) {
            this.refs.forms.setBonuses.addEventListener('click', (event) => {
                const btn = event.target.closest('[data-action="remove-set-piece"]');
                if (!btn) return;
                const index = Number(btn.dataset.index);
                this.removeSetPiece(index);
            });
        }

        if (this.refs.linkCharacterBtn) {
            this.refs.linkCharacterBtn.addEventListener('click', () => this.handleLinkCharacterToSkill());
        }

        if (this.refs.skillCharacterSelect) {
            this.refs.skillCharacterSelect.addEventListener('change', () => {
                if (this.refs.linkCharacterBtn) {
                    this.refs.linkCharacterBtn.disabled = !this.refs.skillCharacterSelect.value;
                }
            });
        }

        if (this.refs.skillCharacterLinks) {
            this.refs.skillCharacterLinks.addEventListener('click', (event) => {
                const btn = event.target.closest('[data-action="unlink-character"]');
                if (!btn) return;
                const index = Number(btn.dataset.characterIndex);
                if (Number.isNaN(index)) return;
                this.unlinkCharacterFromSkill(index);
            });
        }

        if (this.refs.addCharacterSkillBtn) {
            this.refs.addCharacterSkillBtn.addEventListener('click', () => this.handleAddSkillToCharacter());
        }

        if (this.refs.characterSkillSelect) {
            this.refs.characterSkillSelect.addEventListener('change', () => {
                if (this.refs.addCharacterSkillBtn) {
                    this.refs.addCharacterSkillBtn.disabled = !this.refs.characterSkillSelect.value;
                }
            });
        }

        if (this.refs.characterSkillChips) {
            this.refs.characterSkillChips.addEventListener('click', (event) => {
                const btn = event.target.closest('[data-action="remove-character-skill"]');
                if (!btn) return;
                this.handleRemoveSkillFromCharacter(toNumber(btn.dataset.skillId));
            });
        }

        if (this.refs.presetLists) {
            Object.entries(this.refs.presetLists).forEach(([type, container]) => {
                if (!container) return;
                container.addEventListener('click', (event) => {
                    const btn = event.target.closest('[data-action="apply-preset"]');
                    if (!btn) return;
                    const index = Number(btn.dataset.presetIndex);
                    if (Number.isNaN(index)) return;
                    this.applyPreset(type, index);
                });
            });
        }
    }

    setActiveTab(tabName) {
        this.refs.tabs.forEach((tab) => tab.classList.toggle('active', tab.dataset.tab === tabName));
        this.refs.panels.forEach((panel) => panel.classList.toggle('active', panel.dataset.panel === tabName));
    }

    addEntry(type) {
        const next = templates[type] ? templates[type]() : null;
        if (!next) return;
        this.state[type].push(next);
        this.selection[type] = this.state[type].length - 1;
        this.updateMeta();
        if (type === 'items') {
            this.ensureItemDefaults(next);
        }
        this.renderList(type);
        this.renderForm(type);
        this.renderCounts();
        if (type === 'skills') {
            this.renderCharacterSkills();
        } else if (type === 'characters') {
            this.renderSkillRelations();
        }
        this.updateStatus(`已创建新的${this.getLabel(type)}。`);
    }

    deleteEntry(type) {
        const index = this.selection[type];
        if (index < 0) {
            this.updateStatus(`请先选择一个${this.getLabel(type)}。`);
            return;
        }
        if (!confirm(`确定删除该${this.getLabel(type)}吗？`)) {
            return;
        }
        const removed = this.state[type][index];
        this.state[type].splice(index, 1);
        if (type === 'skills' && removed) {
            this.removeSkillEverywhere(removed.id);
        }
        const nextIndex = Math.min(index, this.state[type].length - 1);
        this.selection[type] = nextIndex >= 0 ? nextIndex : -1;
        this.updateMeta();
        this.renderList(type);
        if (type === 'skills') {
            this.renderList('characters');
            this.renderList('equipments');
            this.renderForm('equipments');
        }
        this.renderForm(type);
        this.renderCounts();
        if (type === 'skills') {
            this.renderCharacterSkills();
        } else if (type === 'characters') {
            this.renderSkillRelations();
        }
        this.updateStatus(`已删除一个${this.getLabel(type)}。`);
    }

    getLabel(type) {
        if (type === 'skills') return '技能';
        if (type === 'characters') return '武将';
        if (type === 'equipments') return '装备';
        if (type === 'setBonuses') return '套装';
        if (type === 'items') return '物品';
        return '条目';
    }

    ensureItemDefaults(item) {
        if (!item) return;
        const defaultStack = createStackRuleTemplate();
        item.stack = { ...defaultStack, ...(item.stack || {}) };
        if (typeof item.stack.max !== 'number' || Number.isNaN(item.stack.max)) {
            item.stack.max = item.maxStack ?? defaultStack.max;
        }
        item.maxStack = item.stack.max;

        const defaultOwnership = createOwnershipTemplate();
        item.ownership = { ...defaultOwnership, ...(item.ownership || {}) };

        if (Array.isArray(item.tags)) {
            item.tags = item.tags
                .map((tag) => (tag ?? '').toString().trim())
                .filter((tag) => tag.length > 0);
        } else if (typeof item.tags === 'string') {
            item.tags = item.tags
                .split(',')
                .map((tag) => tag.trim())
                .filter((tag) => tag.length > 0);
        } else {
            item.tags = [];
        }

        const defaultConsumable = createConsumableProfile();
        if (!item.consumable) {
            item.consumable = defaultConsumable;
        } else {
            const existingEffects = item.consumable.effects ?? item.effects;
            item.consumable = { ...defaultConsumable, ...item.consumable };
            item.consumable.effects = existingEffects ?? defaultConsumable.effects;
        }
        if (!Array.isArray(item.consumable.effects) || !item.consumable.effects.length) {
            item.consumable.effects = [createItemEffectTemplate()];
        } else {
            item.consumable.effects = item.consumable.effects.map((effect) => ({
                ...createItemEffectTemplate(),
                ...effect
            }));
        }
        item.effects = item.consumable.effects;

        const defaultMaterial = createMaterialProfile();
        item.material = { ...defaultMaterial, ...(item.material || {}) };

        const defaultCurrency = createCurrencyProfile();
        item.currency = { ...defaultCurrency, ...(item.currency || {}) };

        if (!item.name) item.name = '';
        if (!item.description) item.description = '';
        if (!item.type) item.type = getEnumDefault('itemTypes');

        if (item.type === 'CONSUMABLE') {
            item.subType = item.consumable.category || item.subType || getEnumDefault('consumableTypes');
        } else if (!item.subType) {
            item.subType = 'NONE';
        }
    }

    updateCardDisplay(type, index, item) {
        const card = this.refs.lists[type].querySelector(`[data-index="${index}"]`);
        if (!card) return;
        
        // 仅更新标题部分，不触碰表单
        const title = card.querySelector('h4');
        if (title) {
            title.textContent = `#${item.id || '未设'} ${item.name || '未命名'}`;
        }
    }

    handleFormInput(event, type) {
        const target = event.target;
        const path = target.dataset.path;
        if (!path) return;
        const entity = this.getSelected(type);
        if (!entity) return;

        let value;
        const declaredType = target.dataset.type || target.type;
        if (declaredType === 'number') {
            value = target.value === '' ? 0 : Number(target.value);
        } else if (declaredType === 'boolean') {
            value = target.checked;
        } else if (declaredType === 'int-array') {
            value = target.value
                .split(',')
                .map((seg) => parseInt(seg.trim(), 10))
                .filter((num) => !Number.isNaN(num));
        } else {
            value = target.value;
        }

        // 1. 只更新状态数据
        this.setValue(entity, path, value);
        this.updateMeta();

        // 2. 局部更新左侧列表的显示（而不是整个列表 innerHTML）
        // 这样可以保持右侧表单的焦点
        this.updateCardDisplay(type, this.selection[type], entity);

        if (type === 'skills') {
            this.renderSkillRelations();
            this.renderCharacterSkills();
        } else if (type === 'characters') {
            this.renderCharacterSkills();
            this.renderSkillRelations();
        }
    }



    addEffect() {
        const skill = this.getSelected('skills');
        if (!skill) {
            this.updateStatus('请先选择一个技能。');
            return;
        }
        skill.effects.push(createEffectTemplate());
        this.renderSkillForm();
        this.renderList('skills');
        this.updateMeta();
    }

    removeEffect(index) {
        const skill = this.getSelected('skills');
        if (!skill) return;
        skill.effects.splice(index, 1);
        if (skill.effects.length === 0) {
            skill.effects.push(createEffectTemplate());
        }
        this.renderSkillForm();
        this.renderList('skills');
        this.updateMeta();
    }

    addItemEffect() {
        const item = this.getSelected('items');
        if (!item) {
            this.updateStatus('请选择一个物品。');
            return;
        }
        this.ensureItemDefaults(item);
        item.effects.push(createItemEffectTemplate());
        item.consumable.effects = item.effects;
        this.renderItemForm();
        this.renderList('items');
        this.updateMeta();
    }

    removeItemEffect(index) {
        const item = this.getSelected('items');
        if (!item) return;
        this.ensureItemDefaults(item);
        item.effects.splice(index, 1);
        if (item.effects.length === 0) {
            item.effects.push(createItemEffectTemplate());
        }
        item.consumable.effects = item.effects;
        this.renderItemForm();
        this.renderList('items');
        this.updateMeta();
    }

    addSetPiece() {
        const setBonus = this.getSelected('setBonuses');
        if (!setBonus) {
            this.updateStatus('请选择一个套装。');
            return;
        }
        if (!Array.isArray(setBonus.bonuses)) {
            setBonus.bonuses = [];
        }
        setBonus.bonuses.push(createSetPieceTemplate());
        this.renderSetBonusForm();
        this.renderList('setBonuses');
        this.updateMeta();
    }

    removeSetPiece(index) {
        const setBonus = this.getSelected('setBonuses');
        if (!setBonus || !Array.isArray(setBonus.bonuses)) return;
        setBonus.bonuses.splice(index, 1);
        if (setBonus.bonuses.length === 0) {
            setBonus.bonuses.push(createSetPieceTemplate());
        }
        this.renderSetBonusForm();
        this.renderList('setBonuses');
        this.updateMeta();
    }

    handleFile(event) {
        const file = event.target.files?.[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = () => {
            try {
                const obj = JSON.parse(reader.result);
                this.loadState(obj, file.name);
            } catch (error) {
                console.error(error);
                this.updateStatus('导入失败：JSON 格式错误。');
            }
        };
        reader.readAsText(file, 'utf-8');
        event.target.value = '';
    }

    loadState(raw, fileName = '导入文件') {
        const next = createDefaultState();
        if (raw.meta) {
            next.meta = { ...next.meta, ...raw.meta };
        }
        ['skills', 'characters', 'equipments', 'setBonuses', 'items'].forEach((key) => {
            if (Array.isArray(raw[key])) {
                next[key] = deepClone(raw[key]);
            }
        });
        next.items.forEach((item) => this.ensureItemDefaults(item));
        this.state = next;
        this.selection = {
            skills: next.skills.length ? 0 : -1,
            characters: next.characters.length ? 0 : -1,
            equipments: next.equipments.length ? 0 : -1,
            setBonuses: next.setBonuses.length ? 0 : -1,
            items: next.items.length ? 0 : -1
        };
        this.renderAll();
        this.updateStatus(`已导入 ${fileName}`);
    }

    exportJson() {
        this.updateMeta();
        const data = JSON.stringify(this.state, null, 2);
        const blob = new Blob([data], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        const fileName = `webgame-config-${Date.now()}.json`;
        a.href = url;
        a.download = fileName;
        a.click();
        URL.revokeObjectURL(url);
        this.updateStatus(`已导出 ${fileName}`);
    }

    updateMeta() {
        this.state.meta.updatedAt = new Date().toISOString();
    }

    getSelected(type) {
        const index = this.selection[type];
        if (index < 0) return null;
        return this.state[type][index];
    }

    renderAll() {
        ['skills', 'characters', 'equipments', 'setBonuses', 'items'].forEach((type) => {
            this.renderList(type);
            this.renderForm(type);
        });
        this.renderCounts();
    }

    renderCounts() {
        this.refs.counts.skills.textContent = this.state.skills.length;
        this.refs.counts.characters.textContent = this.state.characters.length;
        this.refs.counts.equipments.textContent = this.state.equipments.length;
        if (this.refs.counts.setBonuses) {
            this.refs.counts.setBonuses.textContent = this.state.setBonuses.length;
        }
        if (this.refs.counts.items) {
            this.refs.counts.items.textContent = this.state.items.length;
        }
    }

    renderList(type) {
        const list = this.refs.lists[type];
        if (!list) return;
        const items = this.state[type] || [];
        if (!items.length) {
            list.innerHTML = '<p class="placeholder">暂无数据。</p>';
            this.updateListCount(type, 0, 0);
            return;
        }
        const entries = this.getFilteredEntries(type);
        const currentIndex = this.selection[type];
        if (!entries.length) {
            list.innerHTML = '<p class="placeholder">无匹配结果。</p>';
            this.updateListCount(type, 0, items.length);
            return;
        }
        list.innerHTML = entries
            .map(({ item, index }) => this.renderCard(type, item, index, currentIndex === index))
            .join('');
        this.updateListCount(type, entries.length, items.length);
    }

    renderCard(type, item, index, active) {
        if (type === 'skills') {
            return `
                <div class="card ${active ? 'active' : ''}" data-index="${index}">
                    <h4>#${item.id || '未设'} ${item.name || '未命名'}</h4>
                    <div>
                        <span class="badge">${getEnumLabel('skillTriggers', item.trigger)}</span>
                        <small>${item.effects?.length || 0} 个效果</small>
                    </div>
                </div>`;
        }
        if (type === 'characters') {
            return `
                <div class="card ${active ? 'active' : ''}" data-index="${index}">
                    <h4>#${item.id || '未设'} ${item.name || '未命名'}</h4>
                    <div>
                        <span class="badge">${getEnumLabel('qualityTypes', item.quality)}</span>
                        <span class="badge">${getEnumLabel('positions', item.position)}</span>
                    </div>
                    <small>技能: ${(item.skillIds && item.skillIds.length ? item.skillIds.join(', ') : '无')}</small>
                </div>`;
        }
        if (type === 'equipments') {
            return `
                <div class="card ${active ? 'active' : ''}" data-index="${index}">
                    <h4>#${item.id || '未设'} ${item.name || '未命名'}</h4>
                    <div>
                        <span class="badge">${getEnumLabel('equipmentTypes', item.type)}</span>
                        <span class="badge">${getEnumLabel('qualityTypes', item.quality)}</span>
                    </div>
                    <small>技能ID: ${item.skillId || 0} · 套装ID: ${item.setId || 0}</small>
                </div>`;
        }
        if (type === 'setBonuses') {
            const pieceText = Array.isArray(item.bonuses)
                ? item.bonuses.map((bonus) => bonus.pieceCount).filter(Boolean).sort((a, b) => a - b).join('/')
                : '—';
            return `
                <div class="card ${active ? 'active' : ''}" data-index="${index}">
                    <h4>#${item.setId || '未设'} ${item.name || '未命名'}</h4>
                    <div>
                        <span class="badge">${pieceText ? pieceText + ' 件' : '套装'}</span>
                    </div>
                    <small>${item.bonuses?.length || 0} 个效果</small>
                </div>`;
        }
        if (type === 'items') {
            return `
                <div class="card ${active ? 'active' : ''}" data-index="${index}">
                    <h4>#${item.id || '未设'} ${item.name || '未命名'}</h4>
                    <div>
                        <span class="badge">${getEnumLabel('itemTypes', item.type)}</span>
                        <span class="badge">${getEnumLabel('consumableTypes', item.subType)}</span>
                    </div>
                    <small>品质: ${item.quality || 0} · 堆叠上限: ${item.maxStack || 1}</small>
                </div>`;
        }
        return '';
    }

    updateListCount(type, filtered, total) {
        const holder = this.refs.listCounts[type];
        if (!holder) return;
        if (!total) {
            holder.textContent = '共 0 条';
            return;
        }
        if (filtered === total) {
            holder.textContent = `共 ${total} 条`;
        } else {
            holder.textContent = `匹配 ${filtered} / ${total} 条`;
        }
    }

    getFilteredEntries(type) {
        const term = (this.filters[type] || '').trim().toLowerCase();
        const items = this.state[type];
        if (!term) {
            return items.map((item, index) => ({ item, index }));
        }
        return items.reduce((acc, item, index) => {
            if (this.matchesFilter(type, item, term)) {
                acc.push({ item, index });
            }
            return acc;
        }, []);
    }

    matchesFilter(type, item, term) {
        if (type === 'skills') {
            return [
                String(item.id ?? ''),
                normalize(item.name),
                normalize(item.description),
                normalize(getEnumLabel('skillTriggers', item.trigger))
            ].some((value) => value.includes(term));
        }
        if (type === 'characters') {
            const enumTexts = [
                normalize(getEnumLabel('qualityTypes', item.quality)),
                normalize(getEnumLabel('positions', item.position))
            ];
            const skillString = (item.skillIds || []).map((id) => String(id)).join(',');
            return [
                String(item.id ?? ''),
                normalize(item.name),
                normalize(item.baseName),
                skillString,
                ...enumTexts
            ].some((value) => value.includes(term));
        }
        if (type === 'equipments') {
            return [
                String(item.id ?? ''),
                normalize(item.name),
                String(item.skillId ?? ''),
                String(item.setId ?? ''),
                normalize(getEnumLabel('equipmentTypes', item.type)),
                normalize(getEnumLabel('qualityTypes', item.quality))
            ].some((value) => value.includes(term));
        }
        if (type === 'setBonuses') {
            const bonusDesc = (item.bonuses || [])
                .map((bonus) => `${bonus.pieceCount || 0}-${bonus.desc || ''}`)
                .join(' ')
                .toLowerCase();
            return [
                String(item.setId ?? ''),
                normalize(item.name),
                bonusDesc
            ].some((value) => value.includes(term));
        }
        if (type === 'items') {
            return [
                String(item.id ?? ''),
                normalize(item.name),
                normalize(item.description),
                normalize(getEnumLabel('itemTypes', item.type)),
                normalize(getEnumLabel('consumableTypes', item.subType))
            ].some((value) => value.includes(term));
        }
        return true;
    }

    renderForm(type) {
        const form = this.refs.forms[type];
        const placeholder = this.refs.placeholders[type];
        if (!form || !placeholder) return;
        const entity = this.getSelected(type);
        if (!entity) {
            form.classList.add('hidden');
            placeholder.classList.remove('hidden');
            return;
        }
        form.classList.remove('hidden');
        placeholder.classList.add('hidden');
        if (type === 'items') {
            this.ensureItemDefaults(entity);
        }
        this.fillForm(form, entity);
        if (type === 'skills') {
            this.renderSkillForm();
        } else if (type === 'characters') {
            this.renderCharacterSkills();
        } else if (type === 'setBonuses') {
            this.renderSetBonusForm();
        } else if (type === 'items') {
            this.renderItemForm();
        }
    }

    renderSkillForm() {
        const skill = this.getSelected('skills');
        if (!skill) return;
        this.fillForm(this.refs.forms.skills, skill);
        this.renderEffects(skill);
        this.renderSkillRelations();
    }

    renderItemForm() {
        const item = this.getSelected('items');
        if (!item) return;
        this.ensureItemDefaults(item);
        this.fillForm(this.refs.forms.items, item);
        this.renderItemEffects(item);
    }

    renderSetBonusForm() {
        const setBonus = this.getSelected('setBonuses');
        if (!setBonus) return;
        if (!Array.isArray(setBonus.bonuses) || !setBonus.bonuses.length) {
            setBonus.bonuses = [createSetPieceTemplate()];
        }
        this.fillForm(this.refs.forms.setBonuses, setBonus);
        this.renderSetBonusPieces(setBonus);
    }

    renderEffects(skill) {
        this.refs.effectRepeater.innerHTML = '';
        skill.effects.forEach((effect, index) => {
            const card = document.createElement('div');
            card.className = 'effect-card';
            card.innerHTML = `
                <header>
                    <h5>效果 ${index + 1}</h5>
                    <button type="button" class="ghost" data-action="remove-effect" data-index="${index}">移除</button>
                </header>
                <div class="effect-grid-inner">
                    <label>目标
                        <select data-path="effects.${index}.target"></select>
                    </label>
                    <label>效果类型
                        <select data-path="effects.${index}.effect"></select>
                    </label>
                    <label>数值来源
                        <select data-path="effects.${index}.valueExpr.source"></select>
                    </label>
                    <label>归属对象
                        <select data-path="effects.${index}.valueExpr.owner"></select>
                    </label>
                    <label>数值尺度
                        <select data-path="effects.${index}.valueExpr.scale"></select>
                    </label>
                    <label>数值
                        <input type="number" data-path="effects.${index}.valueExpr.value" data-type="number" step="1">
                    </label>
                    <label>持续回合
                        <input type="number" data-path="effects.${index}.duration" data-type="number" step="1" min="0">
                    </label>
                    <label>触发概率(%)
                        <input type="number" data-path="effects.${index}.chance" data-type="number" step="1" min="0" max="100">
                    </label>
                    <label>允许叠加
                        <input type="checkbox" data-path="effects.${index}.canOverlay" data-type="boolean">
                    </label>
                </div>
            `;
            this.refs.effectRepeater.appendChild(card);
            this.populateSelect(card.querySelector(`select[data-path="effects.${index}.target"]`), 'targetTypes', effect.target);
            this.populateSelect(card.querySelector(`select[data-path="effects.${index}.effect"]`), 'effectTypes', effect.effect);
            this.populateSelect(card.querySelector(`select[data-path="effects.${index}.valueExpr.source"]`), 'valueSources', effect.valueExpr.source);
            this.populateSelect(card.querySelector(`select[data-path="effects.${index}.valueExpr.owner"]`), 'valueOwners', effect.valueExpr.owner);
            this.populateSelect(card.querySelector(`select[data-path="effects.${index}.valueExpr.scale"]`), 'valueScales', effect.valueExpr.scale);
            this.syncInput(card.querySelector('input[data-path="effects.' + index + '.valueExpr.value"]'), effect.valueExpr.value);
            this.syncInput(card.querySelector('input[data-path="effects.' + index + '.duration"]'), effect.duration);
            this.syncInput(card.querySelector('input[data-path="effects.' + index + '.chance"]'), effect.chance);
            const overlayInput = card.querySelector('input[data-path="effects.' + index + '.canOverlay"]');
            overlayInput.checked = Boolean(effect.canOverlay);
        });
    }

    renderItemEffects(item = this.getSelected('items')) {
        if (!item || !this.refs.itemEffectRepeater) return;
        this.refs.itemEffectRepeater.innerHTML = '';
        const effects = Array.isArray(item.consumable?.effects) ? item.consumable.effects : item.effects;
        if (!Array.isArray(item.consumable?.effects)) {
            item.consumable = item.consumable || createConsumableProfile();
            item.consumable.effects = effects || [createItemEffectTemplate()];
        }
        item.effects = item.consumable.effects;
        item.consumable.effects.forEach((effect, index) => {
            const card = document.createElement('div');
            card.className = 'effect-card item-effect-card';
            card.innerHTML = `
                <header>
                    <h5>效果 ${index + 1}</h5>
                    <button type="button" class="ghost" data-action="remove-item-effect" data-index="${index}">移除</button>
                </header>
                <div class="effect-grid-inner">
                    <label>效果类型
                        <select data-path="effects.${index}.type"></select>
                    </label>
                    <label>数值
                        <input type="number" data-path="effects.${index}.value" data-type="number" step="1" min="0">
                    </label>
                    <label>归属对象
                        <select data-path="effects.${index}.owner"></select>
                    </label>
                    <label>数值尺度
                        <select data-path="effects.${index}.scale"></select>
                    </label>
                    <label>持续回合
                        <input type="number" data-path="effects.${index}.duration" data-type="number" step="1" min="0">
                    </label>
                </div>`;
            this.refs.itemEffectRepeater.appendChild(card);
            this.populateSelect(card.querySelector(`select[data-path="effects.${index}.type"]`), 'effectTypes', effect.type);
            this.syncInput(card.querySelector(`input[data-path="effects.${index}.value"]`), effect.value);
            this.populateSelect(card.querySelector(`select[data-path="effects.${index}.owner"]`), 'valueOwners', effect.owner);
            this.populateSelect(card.querySelector(`select[data-path="effects.${index}.scale"]`), 'valueScales', effect.scale);
            this.syncInput(card.querySelector(`input[data-path="effects.${index}.duration"]`), effect.duration);
        });
    }

    renderSetBonusPieces(setBonus = this.getSelected('setBonuses')) {
        if (!setBonus || !this.refs.setBonusPieceRepeater) return;
        const attrFields = [
            { key: 'hp', label: '生命值' },
            { key: 'maxHp', label: '最大生命' },
            { key: 'atk', label: '攻击' },
            { key: 'def', label: '防御' },
            { key: 'speed', label: '速度' },
            { key: 'critRate', label: '暴击率' },
            { key: 'critDamage', label: '暴击伤害' },
            { key: 'hitRate', label: '命中率' },
            { key: 'dodgeRate', label: '闪避率' }
        ];
        this.refs.setBonusPieceRepeater.innerHTML = '';
        setBonus.bonuses.forEach((bonus, index) => {
            if (!bonus.applyAttr) {
                bonus.applyAttr = createBattleAttrTemplate();
            }
            const card = document.createElement('div');
            card.className = 'effect-card set-piece-card';
            card.innerHTML = `
                <header>
                    <h5>效果 ${index + 1}</h5>
                    <button type="button" class="ghost" data-action="remove-set-piece" data-index="${index}">移除</button>
                </header>
                <div class="effect-grid-inner">
                    <label>需要件数
                        <input type="number" data-path="bonuses.${index}.pieceCount" data-type="number" min="1">
                    </label>
                    <label>技能 ID
                        <input type="number" data-path="bonuses.${index}.skillId" data-type="number" min="0">
                    </label>
                    <label class="full-width">效果描述
                        <textarea data-path="bonuses.${index}.desc" rows="2" placeholder="描述该套装加成"></textarea>
                    </label>
                </div>
                <div class="field-grid two set-bonus-attr-grid">
                    ${attrFields
                        .map(
                            (field) => `
                        <label>${field.label}
                            <input type="number" data-path="bonuses.${index}.applyAttr.${field.key}" data-type="number" step="1">
                        </label>`
                        )
                        .join('')}
                </div>`;
            this.refs.setBonusPieceRepeater.appendChild(card);
            this.syncInput(card.querySelector(`input[data-path="bonuses.${index}.pieceCount"]`), bonus.pieceCount);
            this.syncInput(card.querySelector(`input[data-path="bonuses.${index}.skillId"]`), bonus.skillId);
            const descField = card.querySelector(`textarea[data-path="bonuses.${index}.desc"]`);
            if (descField) {
                descField.value = bonus.desc ?? '';
            }
            attrFields.forEach((field) => {
                const input = card.querySelector(`input[data-path="bonuses.${index}.applyAttr.${field.key}"]`);
                this.syncInput(input, bonus.applyAttr?.[field.key] ?? 0);
            });
        });
    }

    renderSkillRelations() {
        const container = this.refs.skillCharacterLinks;
        const select = this.refs.skillCharacterSelect;
        const button = this.refs.linkCharacterBtn;
        if (!container || !select || !button) return;

        const skill = this.getSelected('skills');
        const reset = (text, optionText) => {
            container.innerHTML = `<p class="helper-text">${text}</p>`;
            select.innerHTML = `<option value="">${optionText}</option>`;
            select.value = '';
            select.disabled = true;
            button.disabled = true;
        };

        if (!skill) {
            reset('请选择一个技能。', '暂无武将可选');
            return;
        }

        const skillId = toNumber(skill.id);
        if (!skillId) {
            reset('请先填写技能 ID 才能建立关联。', '请输入技能 ID');
            return;
        }

        const charactersWithIndex = this.state.characters.map((character, index) => ({ character, index }));

        const linked = charactersWithIndex.filter(({ character }) =>
            Array.isArray(character.skillIds) && character.skillIds.some((id) => toNumber(id) === skillId)
        );

        container.innerHTML = linked.length
            ? linked
                  .map(({ character, index }) => {
                      const label = this.getCharacterLabel(character);
                      return `
                        <span class="chip">
                            <span>${label}</span>
                            <button type="button" data-action="unlink-character" data-character-index="${index}" aria-label="移除 ${label} 的技能">&times;</button>
                        </span>`;
                  })
                  .join('')
            : '<p class="helper-text">尚未关联任何武将，使用右侧下拉快速添加。</p>';

        const available = charactersWithIndex.filter(({ character }) =>
            !Array.isArray(character.skillIds) || !character.skillIds.some((id) => toNumber(id) === skillId)
        );

        if (!available.length) {
            select.innerHTML = '<option value="">暂无可添加的武将</option>';
            select.value = '';
            select.disabled = true;
            button.disabled = true;
            return;
        }

        select.innerHTML = ['<option value="">选择武将</option>', ...available.map(({ character, index }) => `
            <option value="${index}">${this.getCharacterLabel(character)}</option>`)].join('');
        select.value = '';
        select.disabled = false;
        button.disabled = true;
    }

    handleLinkCharacterToSkill() {
        const select = this.refs.skillCharacterSelect;
        const button = this.refs.linkCharacterBtn;
        const skill = this.getSelected('skills');
        if (!select || !button || !skill) {
            this.updateStatus('请选择一个技能再尝试关联。');
            return;
        }
        const skillId = toNumber(skill.id);
        if (!skillId) {
            this.updateStatus('请先填写技能 ID。');
            return;
        }
        const characterIndex = select.value === '' ? -1 : Number(select.value);
        if (characterIndex < 0 || Number.isNaN(characterIndex)) {
            this.updateStatus('请选择需要关联的武将。');
            return;
        }
        const character = this.state.characters[characterIndex];
        if (!character) {
            this.updateStatus('未找到对应的武将。');
            return;
        }
        const added = this.addSkillToCharacter(character, skillId);
        if (!added) {
            this.updateStatus('该武将已包含此技能。');
            return;
        }
        this.updateMeta();
        this.renderSkillRelations();
        if (this.selection.characters >= 0 && this.state.characters[this.selection.characters] === character) {
            this.renderCharacterSkills();
        }
        this.renderList('characters');
        select.value = '';
        button.disabled = true;
        this.updateStatus(`已把 ${this.getSkillLabelById(skillId)} 关联到 ${this.getCharacterLabel(character)}。`);
    }

    unlinkCharacterFromSkill(characterIndex) {
        const skill = this.getSelected('skills');
        if (!skill) {
            this.updateStatus('请选择一个技能。');
            return;
        }
        const skillId = toNumber(skill.id);
        if (!skillId) {
            this.updateStatus('当前技能缺少 ID，无法取消关联。');
            return;
        }
        const character = this.state.characters[characterIndex];
        if (!character) {
            this.updateStatus('未找到对应的武将。');
            return;
        }
        const removed = this.removeSkillFromCharacter(character, skillId);
        if (!removed) {
            this.updateStatus('该武将未持有此技能。');
            return;
        }
        this.updateMeta();
        this.renderSkillRelations();
        if (this.selection.characters >= 0 && this.state.characters[this.selection.characters] === character) {
            this.renderCharacterSkills();
        }
        this.renderList('characters');
        this.updateStatus(`已取消 ${this.getCharacterLabel(character)} 的 ${this.getSkillLabelById(skillId)}。`);
    }

    renderCharacterSkills() {
        const container = this.refs.characterSkillChips;
        const select = this.refs.characterSkillSelect;
        const button = this.refs.addCharacterSkillBtn;
        if (!container || !select || !button) return;
        const character = this.getSelected('characters');

        const reset = (text, optionText) => {
            container.innerHTML = `<p class="helper-text">${text}</p>`;
            select.innerHTML = `<option value="">${optionText}</option>`;
            select.disabled = true;
            select.value = '';
            button.disabled = true;
        };

        if (!character) {
            reset('请选择一个武将。', '暂无技能');
            return;
        }

        const skillIds = Array.isArray(character.skillIds) ? character.skillIds : [];
        container.innerHTML = skillIds.length
            ? skillIds
                  .map((skillId) => {
                      const label = this.getSkillLabelById(skillId);
                      return `
                        <span class="chip">
                            <span>${label}</span>
                            <button type="button" data-action="remove-character-skill" data-skill-id="${skillId}" aria-label="移除技能 ${label}">&times;</button>
                        </span>`;
                  })
                  .join('')
            : '<p class="helper-text">暂无技能，使用上方下拉快速添加。</p>';

        const availableSkills = this.state.skills.filter((skill) =>
            !skillIds.some((id) => toNumber(id) === toNumber(skill.id))
        );
        const selectableSkills = availableSkills.filter((skill) => toNumber(skill.id));

        if (!this.state.skills.length) {
            select.innerHTML = '<option value="">暂无技能可选</option>';
            select.disabled = true;
        } else if (!availableSkills.length) {
            select.innerHTML = '<option value="">已添加全部技能</option>';
            select.disabled = true;
        } else if (!selectableSkills.length) {
            select.innerHTML = '<option value="">暂无可用技能（请先设置技能 ID）</option>';
            select.disabled = true;
        } else {
            select.innerHTML = ['<option value="">选择技能</option>', ...selectableSkills.map((skill) => `
                <option value="${skill.id}">${this.getSkillLabelById(skill.id)}</option>`)].join('');
            select.disabled = false;
        }

        select.value = '';
        button.disabled = true;
        this.syncCharacterSkillInput(character);
    }

    handleAddSkillToCharacter() {
        const select = this.refs.characterSkillSelect;
        const button = this.refs.addCharacterSkillBtn;
        const character = this.getSelected('characters');
        if (!select || !button || !character) {
            this.updateStatus('请选择一个武将。');
            return;
        }
        const skillId = toNumber(select.value);
        if (!skillId) {
            this.updateStatus('请选择需要添加的技能。');
            return;
        }
        const skill = this.findSkillById(skillId);
        if (!skill) {
            this.updateStatus('未找到该技能。');
            return;
        }
        const added = this.addSkillToCharacter(character, skillId);
        if (!added) {
            this.updateStatus('该技能已存在于列表中。');
            return;
        }
        this.updateMeta();
        this.renderCharacterSkills();
        this.renderSkillRelations();
        this.renderList('characters');
        select.value = '';
        button.disabled = true;
        this.updateStatus(`已为 ${this.getCharacterLabel(character)} 添加 ${this.getSkillLabelById(skillId)}。`);
    }

    handleRemoveSkillFromCharacter(skillId) {
        const character = this.getSelected('characters');
        if (!character) {
            this.updateStatus('请选择一个武将。');
            return;
        }
        if (!skillId) return;
        const removed = this.removeSkillFromCharacter(character, skillId);
        if (!removed) {
            this.updateStatus('该技能不存在于当前武将。');
            return;
        }
        this.updateMeta();
        this.renderCharacterSkills();
        this.renderSkillRelations();
        this.renderList('characters');
        this.updateStatus(`已从 ${this.getCharacterLabel(character)} 移除 ${this.getSkillLabelById(skillId)}。`);
    }

    addSkillToCharacter(character, skillId) {
        const normalized = toNumber(skillId);
        if (!normalized) return false;
        if (!Array.isArray(character.skillIds)) {
            character.skillIds = [];
        }
        const exists = character.skillIds.some((id) => toNumber(id) === normalized);
        if (exists) {
            return false;
        }
        character.skillIds.push(normalized);
        return true;
    }

    removeSkillFromCharacter(character, skillId) {
        if (!Array.isArray(character.skillIds)) {
            return false;
        }
        const normalized = toNumber(skillId);
        const index = character.skillIds.findIndex((id) => toNumber(id) === normalized);
        if (index === -1) {
            return false;
        }
        character.skillIds.splice(index, 1);
        return true;
    }

    removeSkillEverywhere(skillId) {
        const normalized = Number(skillId);
        if (Number.isNaN(normalized)) return;
        this.state.characters.forEach((character) => {
            if (!Array.isArray(character.skillIds)) return;
            character.skillIds = character.skillIds.filter((id) => toNumber(id) !== normalized);
        });
        this.state.equipments.forEach((equipment) => {
            if (toNumber(equipment.skillId) === normalized) {
                equipment.skillId = 0;
            }
        });
    }

    syncCharacterSkillInput(character) {
        const hiddenInput = this.refs.forms.characters?.querySelector('[data-path="skillIds"]');
        if (!hiddenInput) return;
        const values = Array.isArray(character.skillIds) ? character.skillIds : [];
        hiddenInput.value = values.join(',');
    }

    findSkillById(id) {
        const target = toNumber(id);
        return this.state.skills.find((skill) => toNumber(skill.id) === target);
    }

    getCharacterLabel(character) {
        if (!character) return '未命名武将';
        const idText = character.id || '未设';
        const nameText = character.name || '未命名';
        return `#${idText} ${nameText}`;
    }

    getSkillLabelById(skillId) {
        const skill = this.findSkillById(skillId);
        const idText = skill ? (skill.id || '未设') : (skillId || '未设');
        const nameText = skill?.name || '未命名';
        return `#${idText} ${nameText}`;
    }

    populateSelect(select, enumKey, current) {
        if (!select) return;
        const options = getEnumOptions(enumKey);
        select.innerHTML = options
            .map((opt) => `<option value="${opt.value}">${opt.label}</option>`)
            .join('');
        const targetValue = current ?? options[0]?.value ?? '';
        select.value = targetValue;
    }

    syncInput(input, value) {
        if (!input) return;
        input.value = value ?? '';
    }

    fillForm(form, entity) {
        form.querySelectorAll('[data-path]').forEach((input) => {
            if (input.closest('.effect-card')) {
                return; // 由 renderEffects 负责
            }
            const value = this.getValue(entity, input.dataset.path);
            if (input.type === 'checkbox') {
                input.checked = Boolean(value);
            } else if (input.dataset.type === 'int-array') {
                input.value = Array.isArray(value) ? value.join(',') : '';
            } else {
                input.value = value ?? '';
            }
            const enumType = input.dataset.enum;
            if (enumType && enums[enumType]) {
                this.populateSelect(input, enumType, value);
                return;
            }
        });
    }

    getValue(obj, path) {
        return path.split('.').reduce((acc, key) => (acc !== undefined && acc !== null ? acc[key] : undefined), obj);
    }

    setValue(obj, path, value) {
        const parts = path.split('.');
        const last = parts.pop();
        const parent = parts.reduce((acc, key) => (acc[key] === undefined ? (acc[key] = {}) : acc[key]), obj);
        parent[last] = value;
    }

    renderPresetRails() {
        if (!this.refs.presetLists) return;
        Object.keys(this.refs.presetLists).forEach((type) => this.renderPresetRail(type));
    }

    renderPresetRail(type) {
        const container = this.refs.presetLists?.[type];
        if (!container) return;
        const presets = presetLibrary[type] || [];
        if (!presets.length) {
            container.innerHTML = '<p class="helper-text">暂无模板，稍后补充。</p>';
            return;
        }
        container.innerHTML = presets
            .map((preset, index) => this.renderPresetCard(preset, type, index))
            .join('');
    }

    renderPresetCard(preset, type, index) {
        const metaText = preset.meta ? `<small>${preset.meta}</small>` : '<span></span>';
        return `
            <article class="preset-card">
                <strong>${preset.title}</strong>
                <p>${preset.detail || ''}</p>
                <div class="preset-card-footer">
                    ${metaText}
                    <button type="button" class="ghost" data-action="apply-preset" data-preset-index="${index}" aria-label="插入模板 ${preset.title}">套用</button>
                </div>
            </article>`;
    }

    applyPreset(type, index) {
        const collection = presetLibrary[type];
        if (!collection || !collection[index]) {
            this.updateStatus('当前分类暂无可用模板。');
            return;
        }
        const payload = deepClone(collection[index].data);
        this.state[type].push(payload);
        this.selection[type] = this.state[type].length - 1;
        this.updateMeta();
        this.renderList(type);
        this.renderForm(type);
        this.renderCounts();
        if (type === 'skills') {
            this.renderCharacterSkills();
            this.renderSkillRelations();
        } else if (type === 'characters') {
            this.renderCharacterSkills();
            this.renderSkillRelations();
        }
        const label = collection[index].title || this.getLabel(type);
        this.updateStatus(`已插入模板：${label}。请检查 ID 避免与现有数据冲突。`);
    }

    updateStatus(message) {
        this.refs.statusText.textContent = message;
    }
}

document.addEventListener('DOMContentLoaded', () => {
    new ConfigEditor();
});
