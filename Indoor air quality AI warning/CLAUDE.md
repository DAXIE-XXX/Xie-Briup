# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

室内空气质量监测与智能家居 AI 预警系统。通过串口采集传感器数据，经网关上报至服务器，服务器根据阈值触发联动告警，客户端（桌面/Android）可实时监控并远程控制设备。

## 系统架构（四大组件）

```
传感器硬件 --串口--> SHGateway(网关) --TCP/JSON--> SHServer(后端) --TCP/JSON--> SHGatewayClient(桌面客户端)
                                                                               +--> SHGatewayAndroidClient(Android客户端)
```

### SHGateway — 数据采集网关（Qt6 Widgets）

- 通过串口与物理传感器通信（`SerialContext` 单例），支持 8 种传感器：PM2.5、烟雾、甲烷、光照、CO2、火光、温度、湿度
- 可下发控制指令：报警器、报警灯、LED2、风扇
- 通过 TCP 将传感器数据上报到 SHServer（`NetContext` 单例），每个传感器对应固定 type 编号
- 支持断线自动重连（指数退避，基础5秒，最大30秒）
- 连接信息持久化到 `gateway_config.ini`，启动时自动重连
- 定时器每10秒自动轮询全部传感器

### SHServer — 后端服务（Qt6 Console，无 GUI）

- 基于 `QTcpServer` 的多线程 TCP 服务（`MyTcpServer` + `MyTcpSocket` + `ThreadsHandle`）
- 核心逻辑：设备注册 → 房间-网关映射 → 传感器数据阈值判断 → 联动指令下发
- `systemInit()` 流程：加载 INI 配置 → 初始化日志 → 初始化数据库连接池 → 启动 TCP 监听
- 模块划分：
  - `net/` — 网络层：TCP 服务器、Socket 管理、线程调度、阈值联动逻辑
  - `db/` — 数据库连接池（`DBContext` + `DBExec`），信号量控制并发
  - `mqtt/` — MQTT 连接池（当前仅基础设施，未实际使用）
  - `config/` — INI 配置文件读取
  - `log/` — 日志输出
- 默认监听端口：10086
- 配置文件路径由 `argv[0]`（程序名）动态拼接生成

### SHGatewayClient — 桌面监控客户端（Qt6 Widgets + Charts + SQL）

- `TcpClient` 封装 TCP 通信，按 `\n` 分割 JSON 帧
- `MainWindow` 包含：传感器实时数据显示、设备控制按钮、阈值配置面板、历史数据图表
- 支持按天/周/月的全部数据和日均值查询（Qt Charts 折线图）
- 本地 SQLite 缓存历史数据
- `LoginDialog` 提供服务器 IP/端口和房间号的登录入口

### SHGatewayAndroidClient — Android 移动客户端（Qt6 Quick/QML）

- 与桌面客户端共享相同的 `TcpClient` 网络层
- `SensorController` 作为 C++ 后端 → QML 前端的桥梁，通过 `Q_PROPERTY` 和 `Q_INVOKABLE` 暴露接口
- QML 页面：`LoginPage` → `DashboardPage`（仪表盘）、`ControlPage`（设备控制）、`ChartPage`（图表）、`ThresholdPage`（阈值设置）、`AlertDialog`（告警弹窗）
- Android 包名：`com.smarthome.gatewayclient`，最低 SDK 28，目标 SDK 34

## 通信协议

所有组件间采用 TCP 传输，以 `\n` 分隔 JSON 帧。

传感器数据类型编号（type 字段）：
| type | 传感器 | 数据类型 |
|------|--------|---------|
| 10001 | CO2 | int (ppm) |
| 10002 | 温度 | double (℃) |
| 10003 | 火光 | bool |
| 10004 | PM2.5 | int (µg/m³) |
| 10005 | 甲烷 | bool |
| 10006 | 光照 | int |
| 10007 | 烟雾 | bool |
| 10008 | 湿度 | double (%) |

设备控制指令：服务器下发 JSON 到网关，网关通过串口转发给硬件执行（如开关风扇、报警器）。

## 构建命令

所有组件使用 CMake + Qt6（最低 6.5），C++17 标准。

### 桌面端编译（Windows MSVC / MinGW）

```bash
# SHServer（无 GUI，console 应用）
cmake -B build_server -S SHServer -DCMAKE_PREFIX_PATH=<Qt6路径> -DCMAKE_BUILD_TYPE=Debug
cmake --build build_server

# SHGateway（GUI 串口网关）
cmake -B build_gateway -S SHGateway -DCMAKE_PREFIX_PATH=<Qt6路径> -DCMAKE_BUILD_TYPE=Debug
cmake --build build_gateway

# SHGatewayClient（桌面客户端）
cmake -B build_client -S SHGatewayClient -DCMAKE_PREFIX_PATH=<Qt6路径> -DCMAKE_BUILD_TYPE=Debug
cmake --build build_client
```

### Android 客户端编译

```bash
cmake -B build_android -S SHGatewayAndroidClient \
  -DCMAKE_PREFIX_PATH=<Qt6_android路径> \
  -DANDROID_SDK_ROOT=<Android_SDK路径> \
  -DANDROID_NDK_ROOT=<NDK路径> \
  -DQT_ANDROID_BUILD_ALL_ABIS=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build_android --target apk
```

### 单个文件的编译测试

修改某个 `.cpp` 后，只需在对应 build 目录下执行 `cmake --build .` 即可增量编译。

## 关键设计模式与注意事项

- **单例模式**：`SerialContext`、`NetContext`、`DBContext`、`MQTTContext`、`ConfigContext` 均通过静态 `getObject()` 获取唯一实例，构造函数为 `protected`
- **连接池模式**：`DBContext` 和 `MQTTContext` 通过 `QSemaphore` 实现阻塞式连接获取/归还
- **多线程**：`MyTcpServer` 使用 `ThreadsHandle` 为每个 socket 分配独立线程，通过 `moveToThread()` 隔离
- **线程安全**：`MyTcpServer` 中的 `socketRoomMap`、`roomGatewayMap` 等映射使用 `QMutex` 保护
- **联动告警冷却**：`RoomTriggerState` 追踪每个房间的风扇和报警器触发状态，支持冷却计时和延迟关闭
- **MSVC 注意**：`SHGateway` 和 `SHGatewayAndroidClient` 的 CMakeLists.txt 包含 MSVC 编译选项 `/Zc:__cplusplus /utf-8 /permissive-`，确保 C++17 宏和 UTF-8 源文件正常编译
- **Qt MOC 必需**：所有继承 `QObject` 的类必须在类声明第一行放置 `Q_OBJECT` 宏，且头文件需包含在 CMakeLists.txt 的 `qt_add_executable` 中
