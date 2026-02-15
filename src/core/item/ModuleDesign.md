# Item 模块设计

## 1. 目标与定位
Item 模块承载游戏中所有通用物品与装备的基础定义，用于串联数据配置层（`ItemConfig`）、运行时背包／装备系统以及战斗属性系统。其核心目标：
- 提供统一的类别枚举，确保客户端与服务端在物品分类上的协议一致。
- 以轻量结构描述物品模板（`ItemTemplate`）和运行时实例（`Item`），允许共享只读配置并最小化复制。
- 定义装备（`Equipment`）与套装（`SetBonus`）的基础字段，支撑战斗属性模块 `BattleAttr` 和技能触发系统。
- 通过可组合的 stacking / ownership / payload 配置，适配消耗品、材料、货币等不同品类。

## 2. 目录与文件概览
- `ItemTypes.h`：所有物品、装备、词缀相关枚举定义，贯穿配置解析与业务逻辑。
- `item.h`：物品模板（配置层）、运行时实例以及与消耗品/材料/货币关联的载荷结构。
- `equip.h`：装备、词缀以及套装效果的数据结构，负责与战斗属性打通。

## 3. 类型体系（`ItemTypes.h`）
该文件以枚举的形式定义模块的分类基石：
- `ItemType`：顶层物品类别，指导 `ItemTemplate` 选择 payload 结构以及客户端 UI 渲染。
- `ConsumableType` / `MaterialType` / `CurrencyType`：各子类的二级分类，细化业务规则（如冷却、用途描述）。
- `EquipmentType`：装备位划分（武器、盔甲、坐骑等），与装备栏数量保持一致。
- `AffixType`：装备词缀属性枚举，覆盖基础属性、战斗属性、抗性等，为后续随机词缀与平衡系统铺垫。

所有枚举均采用数值稳定的 `enum class`，避免命名空间污染并利于序列化。

## 4. 物品模板模型（`item.h`）
### 4.1 通用配置片段
- `ItemEffect`：描述消耗品产生的单个效果，明确效果类型、数值、取值方式（`ValueScale`）以及受益者（`ValueOwner`）。
- `ItemStackingRule`：定义堆叠上限、唯一性及是否自动拆分，用于背包整理逻辑。
- `ItemOwnershipRule`：约束 tradable / bind-on-pickup / bind-on-equip，支持绑定策略。
- `ItemMeta`：聚合 id、名称、描述、品质（`QualityType`）、堆叠／绑定规则与标签，供 UI、索引、日志等通用逻辑使用。

### 4.2 Payload 载荷
针对不同物品类型的扩展字段：
- `ConsumableProfile`：冷却、销毁策略、效果列表。
- `MaterialProfile`：材料或宝物的 tier、用途说明。
- `CurrencyProfile`：货币精度、跨角色共享、是否允许负数。

这些载荷通过 `std::variant` (`ItemPayload`) 聚合，`ItemTemplate::TryGet<T>()` 允许类型安全地读取具体配置。

### 4.3 `ItemTemplate`
`ItemTemplate` 汇总 `ItemMeta` 与 `ItemPayload`，并暴露 `IsStackable()`、`HasTag()`、`IsConsumable()` 等便捷判断。配置加载器（`ItemConfig`）将 JSON 节点映射为该结构并放入内存索引。

### 4.4 `Item` 运行时实例
- 只包含 GUID、计数、绑定/锁定状态以及指向 `ItemTemplate` 的指针，保证运行时对象轻量且共享配置。
- `CanStackWith()` 根据模板 id、堆叠规则与绑定状态判断堆叠兼容性，为背包整理、掉落拾取等流程提供统一判断。

## 5. 装备与套装模型（`equip.h`）
- `EquipmentAffix`：词缀结构，目前仅保存类型与数值，占位以支持主/副词缀拓展。
- `Equipment`：装备实例数据，包含 id、名称、`EquipmentType`、品质、基础战斗属性 (`BattleAttr`)、携带技能 id、套装 id。`hasSkill()`、`GetDescription()` 为 UI/日志提供帮助。
- `SetBonus`：套装定义，`Bonus` 内含触发件数、描述、技能以及附加属性，支撑 2/4/6 件等组合效果。

装备与套装结构与 `BattleAttr` 紧密耦合，供战斗模块读取属性加成或触发额外技能。

## 6. 配置流与其他模块的接口
1. **数据加载**：`ItemConfig::loadFromFile()` 解析 JSON，构建 `ItemTemplate` / `EquipmentTemplate` / `SetBonus`，并通过宏 `REG_ITEM`、`REG_EQUIP`、`REG_SET_BONUS` 注册到单例 `ItemConfig` 中。
2. **运行时引用**：背包（`Inventory`）或战斗逻辑只保存 `Item`/`Equipment` 指针或 id，通过 `GET_ITEM(id)` 取回模板，避免重复解析。
3. **战斗联动**：装备基础属性直接映射至 `BattleAttr`，词缀与 `AffixType` 供未来的随机属性系统实现。
4. **标签/索引**：`ItemMeta::tags` 与 `ItemConfig` 中的 `tagIndex_` 支撑按主题（活动、阵营等）批量检索物品。

## 7. 典型工作流
- **配置生成 → 上线**：策划在 JSON 中编辑物品→构建时由 `ItemConfig` 解析并填充 `ItemTemplate`→运行期所有逻辑通过 id 查询。
- **背包堆叠**：拾取掉落时创建 `Item` 实例，调用 `CanStackWith()` 判断是否合并，否则新建格子；若 `bindOnPickup` 为真，则调用 `Bind()`。
- **装备穿戴**：根据 `Equipment.type` 匹配装备槽，读取 `baseAttrs` 应用到角色属性，并检测 `setId` 与当前已穿件数触发 `SetBonus`。
- **消耗品使用**：根据 `ItemTemplate::TryGet<ConsumableProfile>()` 取出效果列表，逐条应用 `ItemEffect`，同时遵循 `cooldownSec` 与 `destroyOnUse`。

## 8. 扩展与演进方向
- **词缀系统**：填充 `EquipmentAffix`，并在 `Equipment` 中加入主/副词缀容器与随机生成逻辑。
- **Payload 扩展**：可通过在 `ItemPayload` 中增加自定义 `struct` 来支持新的物品类型（如宠物、坐骑部件）。
- **序列化协议**：`Item` 目前只在服务器侧定义，可补充序列化函数与 DTO，以便网络同步。
- **表现层联动**：利用 `ItemMeta::tags`、`quality` 组合 UI 渲染（颜色、边框、粒子效果等）。

## 9. 依赖关系
- **Battle 模块**：引用 `BattleTypes.h`、`BattleAttr.h` 以确保属性字段一致。
- **数据配置**：`ItemConfig` 位于 `src/data/code`，负责解析与注册；`Item` 模块自身不感知文件格式，保持纯粹的数据定义。
- **工具链**：未来可由脚本自动生成 `ItemTypes` 与配置模板，保证策划配置与代码枚举同步。

## 10. 使用建议
- 新增物品类型时，先在 `ItemTypes.h` 扩展枚举，再在配置解析（`ItemConfig.cpp`）中映射，并在 `ItemTemplate` 或 `Item` 上添加必要的辅字段。
- 避免在运行时复制 `ItemTemplate`；所有业务逻辑应通过 const 指针引用，保持内存占用可控。
- 当需要对某类物品批量操作（如批量发放活动道具），优先使用 `tag`/`type` 索引，减少硬编码 id。
