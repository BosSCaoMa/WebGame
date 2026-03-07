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

压测结果会保存到：`build/benchmarks/battle_api/battle_bench_时间戳/`，核心文件包括：

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

### 完整战斗流程压测（默认 `start -> auto`）

```
chmod +x scripts/bench_battle_flow.sh
FLOW_USERS=200 CONCURRENCY=40 FLOW_MODE=auto WEBGAME_PORT=18920 AUTO_START_SERVER=1 ./scripts/bench_battle_flow.sh
```

可选模式：

- `FLOW_MODE=auto`（默认）：每个用户执行 `start -> auto`，由服务端一次性跑完整场战斗。
- `FLOW_MODE=manual`：执行 `start -> attack*N -> end`，其中 `N` 由 `ATTACK_STEPS` 指定（用于调试逐回合行为）。

结果目录：`build/benchmarks/battle_flow/battle_flow_bench_时间戳/`，核心文件：

- `summary.txt`：流程维度汇总（flow 成功率、flow RPS、流程延迟分位、战斗结束率、平均回合数、胜负分布）
- `summary.json`：结构化流程指标
- `raw_metrics.tsv`：每个虚拟用户一行（是否成功、失败阶段、流程耗时、攻击步数、结果、最终回合数）
- `server.log`：自动拉起服务时的日志
- `aggregate_summary.txt`：该目录下历史流程压测自动汇总（每次运行后自动刷新）
- `aggregate_summary.json`：自动汇总的结构化版本

适用场景：验证“完整业务闭环”而非单次接口性能，便于在简历中展示端到端稳定性数据。



## 每次测试：命令与结果位置（固定流程）

### 1) 单接口压测（`/battle`）

```bash
cd /home/ubuntu/code/WebGame
TOTAL_REQUESTS=500 CONCURRENCY=50 WEBGAME_PORT=18901 AUTO_START_SERVER=1 ./scripts/bench_battle.sh
```

结果位置：

- `build/benchmarks/battle_api/battle_bench_时间戳/summary.txt`
- `build/benchmarks/battle_api/battle_bench_时间戳/summary.json`
- `build/benchmarks/battle_api/battle_bench_时间戳/raw_metrics.tsv`

快速查看最新一次：

```bash
cd /home/ubuntu/code/WebGame
latest=$(ls -dt build/benchmarks/battle_api/battle_bench_* | head -n1)
echo "$latest"
cat "$latest/summary.txt"
```

### 2) 完整流程压测（推荐简历口径：`start -> auto`）

```bash
cd /home/ubuntu/code/WebGame
FLOW_USERS=1200 CONCURRENCY=120 FLOW_MODE=auto WEBGAME_PORT=18888 AUTO_START_SERVER=1 ./scripts/bench_battle_flow.sh
```

结果位置：

- `build/benchmarks/battle_flow/battle_flow_bench_时间戳/summary.txt`
- `build/benchmarks/battle_flow/battle_flow_bench_时间戳/summary.json`
- `build/benchmarks/battle_flow/battle_flow_bench_时间戳/raw_metrics.tsv`
- `build/benchmarks/battle_flow/aggregate_summary.txt`
- `build/benchmarks/battle_flow/aggregate_summary.json`

快速查看最新一次：

```bash
cd /home/ubuntu/code/WebGame
latest=$(ls -dt build/benchmarks/battle_flow/battle_flow_bench_* | head -n1)
echo "$latest"
cat "$latest/summary.txt"
cat build/benchmarks/battle_flow/aggregate_summary.txt
```

### 3) 阶梯压测（找峰值/边界）

推荐一键脚本（会逐档执行并自动生成矩阵汇总）：

```bash
cd /home/ubuntu/code/WebGame
MATRIX_POINTS="400:40,800:80,1200:120,1600:160,2400:320,3200:480" \
FLOW_MODE=auto \
AUTO_START_SERVER=1 \
./scripts/run_bench_matrix.sh
```

如果压测机和服务机分离，推荐在“新机器（压测机）”使用：

```bash
cd /home/ubuntu/code/WebGame
SERVER_HOST=10.0.0.8 SERVER_PORT=18888 \
MATRIX_POINTS="400:40,800:80,1200:120,1600:160,2400:320,3200:480" \
./scripts/run_remote_bench_matrix.sh
```

说明：

- `run_remote_bench_matrix.sh` 会先请求 `http://SERVER_HOST:SERVER_PORT/ping` 做连通性检查。
- 该脚本固定 `AUTO_START_SERVER=0`，只做远端压测，不在本机启动服务。
- 默认输出到 `test/benchmarks_remote/battle_flow/`（可通过 `BENCH_ROOT_DIR` 修改）。

矩阵结果位置：

- `test/benchmarks/battle_flow/matrix_时间戳/matrix_raw.tsv`
- `test/benchmarks/battle_flow/matrix_时间戳/matrix_summary.txt`
- `test/benchmarks/battle_flow/matrix_时间戳/matrix_summary.json`

常用参数：

- `MATRIX_POINTS`：压测档位，格式 `users:concurrency`，用逗号分隔。
- `STOP_ON_FAIL`：`1` 表示遇到失败立即停止，默认 `0`。
- `BENCH_ROOT_DIR`：统一输出根目录，默认 `test/benchmarks`。

也可手工逐档执行：

```bash
cd /home/ubuntu/code/WebGame
for rc in "400 40" "800 80" "1200 120" "1600 160" "2400 320" "3200 480"; do
	u=${rc% *}; c=${rc#* }
	FLOW_USERS=$u CONCURRENCY=$c FLOW_MODE=auto AUTO_START_SERVER=1 ./scripts/bench_battle_flow.sh
done
```

批量汇总（推荐脚本，压测后也会自动调用）：

```bash
cd /home/ubuntu/code/WebGame
python3 scripts/summarize_battle_flow.py \
  --base-dir build/benchmarks/battle_flow \
  --flow-mode auto
```

### 4) 一键性能画像（定位瓶颈）

脚本会采集线程占用、上下文切换、运行队列、系统调用占比与可选 perf 热点，并输出自动诊断结论。

支持两种模式：

- `PROFILE_MODE=timed`：按固定时长采样（自动结束）。
- `PROFILE_MODE=interactive`：交互式采样（输入结束命令后停止）。

#### 固定时长模式（timed）

```bash
cd /home/ubuntu/code/WebGame
PROFILE_MODE=timed RUN_MATRIX=1 DURATION_SEC=20 MATRIX_POINTS="400:40,800:80,1200:120" ./scripts/profile_quick.sh
```

#### 交互模式（interactive，一键起服 + 手动结束）

```bash
cd /home/ubuntu/code/WebGame
PROFILE_MODE=interactive START_SERVER=1 RUN_MATRIX=0 WEBGAME_PORT=18888 ./scripts/profile_quick.sh
```

脚本启动后输入结束命令（默认是 `stop`）：

```bash
stop
```

结果目录：`test/benchmarks/profiling_时间戳/`

- `summary.txt`：快速结论（锁争用/事件等待/单线程热点等信号）
- `meta.json`：结构化诊断结果
- `ps_threads.txt` / `top_threads.txt`：线程 CPU 快照
- `pidstat_u.txt` / `pidstat_w.txt`：CPU 与上下文切换采样
- `vmstat.txt`：系统运行队列采样
- `strace_summary.txt`：系统调用占比
- `perf_report.txt`：热点函数（若 perf 权限允许）

常用参数：

- `PROFILE_MODE`：采样模式。`timed` 固定时长；`interactive` 交互结束。
- `START_SERVER`：仅在 `interactive` 模式有效。`1` 自动拉起 `build/WebGame`；`0` 不自动起服。
- `STOP_COMMAND`：交互模式结束命令（默认 `stop`）。
- `RUN_MATRIX`：`1` 联动阶梯压测；`0` 仅对当前运行中的服务采样。
- `TARGET_PID`：指定采样进程 PID（常用于 `RUN_MATRIX=0` 且不自动起服）。
- `DURATION_SEC`：采样时长（秒，`timed` 模式生效）。
- `SAMPLE_INTERVAL_SEC`：`pidstat/vmstat` 采样间隔（秒）。
- `MATRIX_POINTS`：联动压测档位。
- `WEBGAME_HOST` / `WEBGAME_PORT`：自动起服时的监听地址与端口。
- `WEBGAME_VERBOSE_CONN_LOG`：`1` 打开连接/请求级高频日志；默认关闭以减少锁竞争。
- `BENCH_ROOT_DIR`：结果根目录（默认 `test/benchmarks`）。

默认行为说明：当 `PROFILE_MODE=interactive` 且 `START_SERVER=1` 时，若未显式传 `WEBGAME_HOST`，脚本默认按 `0.0.0.0` 启动服务（方便远端机器访问）。

说明：`strace` 与 `perf` 可能受系统权限限制；权限不足时脚本会降级执行，仍产出其余统计结果。

### 独立的 gtest 测试
测试工程位于 `test/` 目录，拥有自己的 `CMakeLists.txt`，默认不会和主程序一起编译。想运行测试时，请使用单独的构建目录并显式启用 `BUILD_TESTING`：

1. **配置构建（首次）**  
	`cmake -S . -B build-tests -DBUILD_TESTING=ON`
2. **编译测试目标**  
	`cmake --build build-tests`
3. **执行 gtest**  
	`cd build-tests && ctest --output-on-failure`

提示：这套流程会通过 `FetchContent` 自动下载 GoogleTest，需要能够访问 GitHub；若在离线环境，可在 `test/CMakeLists.txt` 中改用本地镜像。构建后的 `build-tests/WebGameTests` 与主程序互不干扰，删除或忽略 `build-tests/` 即可恢复纯净工作区。

