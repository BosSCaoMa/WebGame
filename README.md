# WebGame
简易 web 游戏

## 构建与测试

### 主程序
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
./build/WebGame
```

推荐使用防端口占用启动脚本：

```
chmod +x scripts/run_server.sh
./scripts/run_server.sh
```

该脚本会在启动前检查 `18888` 端口，若检测到残留 `WebGame` 进程会先尝试优雅停止，避免 `bind failed: Address already in use`。
若仍被其他进程占用，脚本会自动回退到下一个空闲端口（并通过 `WEBGAME_PORT` 注入给服务）。你也可以手动指定端口：

```
WEBGAME_PORT=18889 ./scripts/run_server.sh
```

### `/battle` 并发压测（带结果落盘）

```
chmod +x scripts/bench_battle.sh
TOTAL_REQUESTS=500 CONCURRENCY=50 WEBGAME_PORT=18901 AUTO_START_SERVER=1 ./scripts/bench_battle.sh
```

压测结果会保存到：`build/log/battle_bench_时间戳/`，核心文件包括：

- `summary.txt`：人类可读汇总（吞吐、成功率、RPS、延迟分位、网络阶段耗时、状态码分布）
- `summary.json`：结构化汇总，便于后续图表或自动化处理
- `raw_metrics.tsv`：每个请求的原始明细（http code、time_total、time_connect、TTFB、下载字节等）
- `status_counts.tsv`：状态码分布统计
- `server.log`：脚本自动拉起服务时的服务端日志

常用参数：

- `TOTAL_REQUESTS`：总请求数（默认 `500`）
- `CONCURRENCY`：并发数（默认 `50`）
- `WEBGAME_PORT`：目标端口（默认 `18888`）
- `AUTO_START_SERVER`：`1` 自动启动服务，`0` 压测已在运行的服务
- `BATTLE_ACTION`：压测动作（默认 `attack`）
- `BATTLE_ACCOUNT`：请求头 `x-account` 的账号值

### 完整战斗流程压测（`start -> attack*N -> end`）

```
chmod +x scripts/bench_battle_flow.sh
FLOW_USERS=200 CONCURRENCY=40 ATTACK_STEPS=6 WEBGAME_PORT=18920 AUTO_START_SERVER=1 ./scripts/bench_battle_flow.sh
```

结果目录：`build/log/battle_flow_bench_时间戳/`，核心文件：

- `summary.txt`：流程维度汇总（flow 成功率、flow RPS、流程延迟分位、战斗结束率）
- `summary.json`：结构化流程指标
- `raw_metrics.tsv`：每个虚拟用户一行（是否成功、失败阶段、流程耗时、攻击步数、结果）
- `server.log`：自动拉起服务时的日志

适用场景：验证“完整业务闭环”而非单次接口性能，便于在简历中展示端到端稳定性数据。

### 独立的 gtest 测试
测试工程位于 `test/` 目录，拥有自己的 `CMakeLists.txt`，默认不会和主程序一起编译。想运行测试时，请使用单独的构建目录并显式启用 `BUILD_TESTING`：

1. **配置构建（首次）**  
	`cmake -S . -B build-tests -DBUILD_TESTING=ON`
2. **编译测试目标**  
	`cmake --build build-tests`
3. **执行 gtest**  
	`cd build-tests && ctest --output-on-failure`

提示：这套流程会通过 `FetchContent` 自动下载 GoogleTest，需要能够访问 GitHub；若在离线环境，可在 `test/CMakeLists.txt` 中改用本地镜像。构建后的 `build-tests/WebGameTests` 与主程序互不干扰，删除或忽略 `build-tests/` 即可恢复纯净工作区。
