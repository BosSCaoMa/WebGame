# Player 序列化说明（Protobuf）

本文档记录 `WebGame` 当前已经落地的 `Player` 序列化实现、使用方式与后续演进约定。

## 1. 当前落地状态

已完成项：

- 使用 `proto3` 定义 `Player` 及相关子结构：`Character`、`Inventory`、`Skill`、`Equipment`、`BattleAttr`。
- 在 `Player.cpp` 实现 `Player <-> webgame.player.v1::Player` 双向转换。
- `Player::saveDataToServer()` / `Player::loadDataFromServer()` 已改为真实二进制存档读写。
- 增加回归测试 `test/PlayerSerializationTest.cpp`，覆盖 round-trip（保存后再加载）。

## 2. 相关文件

- Schema：`proto/player.proto`
- 业务实现：`src/game/player/Player.cpp`
- 对外接口：`src/game/player/Player.h`
- 构建接入：`CMakeLists.txt`
- 回归测试：`test/PlayerSerializationTest.cpp`

## 3. 构建与代码生成

### 3.1 依赖

Linux（Ubuntu）安装命令：

```bash
sudo apt-get update
sudo apt-get install -y protobuf-compiler libprotobuf-dev
```

### 3.2 工程内生成方式（推荐）

项目已在 `CMakeLists.txt` 中接入 `find_package(Protobuf REQUIRED)` 与 `protobuf_generate_cpp(...)`。

直接构建即可自动根据 `proto/player.proto` 生成并编译 `player.pb.cc/.h`：

```bash
cd /home/ubuntu/code/WebGame
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

## 4. 数据落盘行为

- 存档文件路径：`build/player_data/player_<id>.pb`
- 序列化格式：protobuf binary
- 当前 `loadDataFromServer()` 在存档不存在时返回 `false`

## 5. 字段映射约定（当前实现）

- `Player::mainCharacter` 不存指针，使用 `main_character_id` 持久化后回填指针。
- `std::map<enum, T>` 统一为 `map<int32, T>`。
- `currencyWallet_` 与 `shopPurchaseHistory_` 已完整落盘并可恢复。
- `schema_version` 已写入（当前值 `1`）。
- `Player` 中已预留 `reserved 15~19` 作为后续扩展缓冲位。

## 6. 已知限制

1. `Inventory::Item` 的 `guid`、`locked` 目前仅在 schema 内定义，运行时加载受现有 `Inventory` API 限制，暂未精确回放。
2. `Inventory::next_guid` 当前未从运行时对象完整回写/恢复。
3. 当前为本地文件存档实现（路径在 `build/` 下），尚未接入远端 DB/服务。

上述限制不影响当前可用性（核心字段 round-trip 已通过测试），但会影响“严格字节级恢复”。

## 7. 验证方式

```bash
cd /home/ubuntu/code/WebGame
cmake --build build -j$(nproc)
cd build
ctest --output-on-failure
```

当前与序列化相关的测试用例：

- `PlayerSerializationTest.SaveAndLoadRoundTrip`

## 8. 演进规则（必须遵守）

- 不复用历史字段号。
- 删除字段时必须 `reserved`。
- 新增字段只追加，不改变旧字段语义。
- 枚举新增值只追加，不修改既有值。
- 发生破坏性升级时新建 package（例如 `webgame.player.v2`）。

## 9. 推荐下一步

1. 抽象 `IPlayerStorage`（文件版/DB版）解耦存储介质。
2. 补齐 `Inventory` 序列化缺口（`guid/locked/next_guid` 精确恢复）。
3. 增加“老存档兼容”测试与版本迁移测试。
