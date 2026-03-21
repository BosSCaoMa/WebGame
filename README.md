# WebGame（C++ 后端面试项目）

`WebGame` 是一个以 C++ 后端为核心的游戏服务示例项目，覆盖：
- 网络接入与路由分发（Reactor 风格）
- 登录鉴权、自动战斗、战斗历史
- 用户资产持久化（等级/经验/金币/钻石）
- 每日签到与每日任务系统
- 排行榜与基础可观测性（`/metrics`）
- 前后端分离部署（Nginx 静态托管 + `/api` 反代）

## 1. 项目亮点

- 自研事件循环与连接管理（`EventLoop` / `EventLoopGroup`）
- 业务模块完整：账号、战斗、奖励、每日系统、榜单
- PostgreSQL 持久化 + 内存回退，保证开发环境可用性
- 路由级指标统计（请求量/错误量/延迟）
- 前端控制台三视图（主页 / 排行 / 战斗）与后端联动

## 2. 架构概览

- **网络层**：`src/network`
	- `LoginAcceptor`：监听接入
	- `EventLoopGroup`：I/O 线程池
	- `ProtocolDispatcher`：HTTP 路由分发
- **业务层**：`src/core` + `src/game` + `src/server`
	- `BattleManager`：自动战斗流程
	- `PlayerProfileStore`：用户资产、战斗记录、每日状态、排行榜
- **配置层**：`src/data`
	- `CharacterConfig`、`SkillConfig`、`ItemConfig`

## 3. 数据库表（当前）

- `user_profiles`：用户资产与等级进度
- `battle_records`：战斗记录与结算数据
- `daily_states`：每日签到与任务状态

> 脚本：`scripts/postgres_init.sql`（支持幂等重复执行）

## 4. API 一览

### 4.1 基础
- `GET /ping`：健康检查
- `GET /metrics`：路由指标快照

### 4.2 账号与档案
- `POST /login`
	- Header：`x-account`、`x-token`
	- 返回：`profile`、`storage_backend`、`persistent`
- `GET /profile`
	- Header：`x-account`、`x-token`

### 4.3 战斗
- `POST /battle`（仅支持 `action=auto`）
	- 返回：`session`、`damage_board`、`damage_summary`、`battle_logs`、`reward`、`profile_after`、`battle_record`
- `GET /profile/history`
	- Header：`x-account`、`x-token`、可选 `x-limit(1~50)`

### 4.4 每日系统
- `GET /daily`
	- Header：`x-account`、`x-token`
	- 返回：`daily`（签到、战斗进度、任务状态）
- `POST /daily/signin`
	- Header：`x-account`、`x-token`
	- 重复签到返回 `409`
- `POST /daily/claim-task`
	- Header：`x-account`、`x-token`
	- 当前条件：当日完成 3 场战斗，重复领取返回 `409`

### 4.5 排行榜
- `GET /leaderboard`
	- 可选 Header：`x-limit(1~100)`
	- 规则：`level DESC, gold DESC, account ASC`

## 5. 本地运行

### 5.1 初始化 PostgreSQL（推荐）

```bash
psql -U postgres -f /home/ubuntu/code/WebGame/scripts/postgres_init.sql
```

若需切到 postgres 账号：

```bash
sudo -u postgres psql -f /home/ubuntu/code/WebGame/scripts/postgres_init.sql
```

若出现 `Permission denied`（postgres 无法访问你的 home）：

```bash
cp /home/ubuntu/code/WebGame/scripts/postgres_init.sql /tmp/webgame_postgres_init.sql
sudo -u postgres psql -f /tmp/webgame_postgres_init.sql
```

### 5.2 设置数据库环境变量

```bash
export WEBGAME_PG_HOST=127.0.0.1
export WEBGAME_PG_PORT=5432
export WEBGAME_PG_DB=webgame
export WEBGAME_PG_USER=webgame
export WEBGAME_PG_PASSWORD=webgame
```

### 5.3 编译与启动

```bash
cd /home/ubuntu/code/WebGame
cmake -S . -B build
cmake --build build -j
./scripts/run_server.sh
```

### 5.4 发布前端到 Nginx

```bash
cd /home/ubuntu/code/WebGame
sudo src/client/load.sh
```

## 6. 前端说明

- 前端目录：`src/client`
- 当前采用顶部导航三视图：`主页` / `排行` / `战斗`
- 主页展示：用户资产 + 每日任务
- 排行页展示：等级/金币榜
- 战斗页展示：阵容配置、自动战斗、战报弹窗

## 7. 常见问题

- **`/metrics` 连接失败**：先确认后端是否已启动并监听 `18888`
- **页面不更新**：执行 `sudo src/client/load.sh` 后强刷（`Ctrl+Shift+R`）
- **显示 `memory-fallback`**：说明未启用 `libpq` 或 PostgreSQL 不可达
- **`sudo -u postgres psql -f ...` 失败**：用 `/tmp/webgame_postgres_init.sql` 方式执行

## 8. 下一步建议

- 增加 Redis 排行榜缓存（TTL 30~60s）
- 增加压测脚本与性能基线报告
- 增加 battle 回放持久化与回放接口
- 增加限流与熔断策略
