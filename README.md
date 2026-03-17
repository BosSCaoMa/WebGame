# WebGame（C++ 后端面试项目）

一个基于 C++17 的轻量多人游戏服务端示例，包含：
- 自研事件循环与连接管理（`EventLoop` / `EventLoopGroup`）
- 登录鉴权、会话管理、自动战斗模拟
- HTTP API 接入层 + Nginx 静态托管前端
- 可观测性指标（请求量 / 错误量 / 路由延迟）

## 1. 项目定位

> 独立设计并实现 C++ 游戏后端服务，构建从网络接入、认证、战斗逻辑到可观测性监控的完整闭环；
> 使用自研 Reactor 事件模型支撑多连接处理，并通过 `/metrics` 提供路由级延迟与错误统计，支持线上问题快速定位。

## 2. 架构概览

- **网络层**：`src/network`
	- `LoginAcceptor`：监听与接入
	- `EventLoopGroup`：I/O 线程池
	- `ProtocolDispatcher`：HTTP 路由分发
- **游戏层**：`src/game` + `src/core`
	- `Player`、`Inventory`、`BattleManager`
	- `CharacterConfig`、`SkillConfig` 配置驱动
- **会话层**：`SessionManager`
	- 登录后按 `x-account` 绑定连接上下文

## 3. 关键 API

### `GET /ping`
健康检查。

### `POST /login`
Header：
- `x-account`
- `x-token`

返回新增：
- `profile`：用户等级/经验/金币/钻石
- `storage_backend`：`postgresql` 或 `memory-fallback`
- `persistent`：是否真实持久化

### `GET /profile`
Header：
- `x-account`
- `x-token`

用于拉取当前用户持久化档案。

### `GET /profile/history`
Header：
- `x-account`
- `x-token`
- `x-limit`（可选，1~50，默认 10）

返回最近战斗记录（胜负、回合、总伤害、奖励、时间戳），用于个人战绩页。

### `GET /leaderboard`
Header：
- `x-limit`（可选，1~100，默认 10）

返回按 `level DESC, gold DESC` 排序的玩家榜单。

### `GET /daily`
Header：
- `x-account`
- `x-token`

返回当日签到与任务进度（是否已签到、战斗次数、任务是否领取）。

### `POST /daily/signin`
Header：
- `x-account`
- `x-token`

每日签到领奖，重复签到会返回 `rejected`。

### `POST /daily/claim-task`
Header：
- `x-account`
- `x-token`

领取每日任务奖励（当前条件：当日完成 3 场战斗）。

### `POST /battle`
当前只支持自动战斗：

```json
{
	"action": "auto",
	"user_team": [2001, 2002, 2003],
	"enemy_team": [2004, 2005, 2006],
	"loadout": {
		"user_item_id": 1,
		"enemy_item_id": 2,
		"user_equip_id": 101,
		"enemy_equip_id": 102
	}
}
```

返回包含：
- `session`（回合、胜负、存活信息）
- `damage_board`（单角色伤害）
- `damage_summary`（我方/敌方总伤害、MVP）
- `battle_logs`（战斗过程）
- `reward`（本场结算收益）
- `profile_after`（结算后的用户档案）
- `battle_record`（本场战斗记录摘要，可用于历史页）

### `GET /metrics`
服务可观测性指标快照：
- 路由请求总数
- 路由错误数
- 平均耗时 / 最大耗时
- 进程 uptime

## 4. 本地运行

### PostgreSQL 初始化（推荐）

```bash
psql -U postgres -f /home/ubuntu/code/WebGame/scripts/postgres_init.sql
```

如果你环境里只能 `sudo` 到 postgres 账号，可用：

```bash
sudo -u postgres psql -f /home/ubuntu/code/WebGame/scripts/postgres_init.sql
```

如果提示 `Permission denied`（`postgres` 无法访问你的 home 路径），改为：

```bash
cp /home/ubuntu/code/WebGame/scripts/postgres_init.sql /tmp/webgame_postgres_init.sql
sudo -u postgres psql -f /tmp/webgame_postgres_init.sql
```

该脚本支持重复执行（幂等）。

启动服务前可设置：

```bash
export WEBGAME_PG_HOST=127.0.0.1
export WEBGAME_PG_PORT=5432
export WEBGAME_PG_DB=webgame
export WEBGAME_PG_USER=webgame
export WEBGAME_PG_PASSWORD=webgame
```

如果本机未安装 `libpq` 或 PostgreSQL 不可达，服务会自动回退到内存存储（`memory-fallback`），功能可用但重启后数据丢失。

### 编译

```bash
cd /home/ubuntu/code/WebGame
cmake -S . -B build
cmake --build build -j
```

### 启动服务

```bash
cd /home/ubuntu/code/WebGame
./scripts/run_server.sh
```

### 部署前端静态资源（Nginx）

```bash
cd /home/ubuntu/code/WebGame
sudo src/client/load.sh
```

## 5. 面试可讲的工程点

1. **事件驱动网络模型**：如何拆分接入线程、I/O 线程与协议分发。
2. **战斗系统建模**：角色、技能、队伍配置如何解耦。
3. **接口鲁棒性**：参数校验、错误码约定、异常兜底。
4. **可观测性落地**：用 `/metrics` 追踪路由吞吐、错误和延迟。
5. **前后端分离部署**：Nginx 静态托管 + `/api` 反向代理到后端。

## 6. 下一步规划

- 增加压测脚本与 QPS/延迟基线报告
- 增加 battle 回放持久化（便于复盘）
- 增加单元测试与集成测试覆盖率统计
- 增加限流与熔断策略（保护战斗接口）
