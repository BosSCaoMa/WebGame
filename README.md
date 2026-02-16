# WebGame
简易 web 游戏

## 构建与测试

### 主程序
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
./build/WebGame
```

### 独立的 gtest 测试
测试工程位于 `test/` 目录，拥有自己的 `CMakeLists.txt`，默认不会和主程序一起编译。想运行测试时，请使用单独的构建目录并显式启用 `BUILD_TESTING`：

1. **配置构建（首次）**  
	`cmake -S . -B build-tests -DBUILD_TESTING=ON`
2. **编译测试目标**  
	`cmake --build build-tests`
3. **执行 gtest**  
	`cd build-tests && ctest --output-on-failure`

提示：这套流程会通过 `FetchContent` 自动下载 GoogleTest，需要能够访问 GitHub；若在离线环境，可在 `test/CMakeLists.txt` 中改用本地镜像。构建后的 `build-tests/WebGameTests` 与主程序互不干扰，删除或忽略 `build-tests/` 即可恢复纯净工作区。
