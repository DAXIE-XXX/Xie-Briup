# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

室内空气质量 AI 告警系统 — 智能家居网关 Android 客户端。采用 Qt 6.8.3 + QML 构建，通过 TCP（JSON）与网关服务器通信，实现传感器数据监控、设备联动控制、告警阈值配置和历史数据图表查看。

## 构建与运行

### 桌面调试（MinGW 64-bit）

```bash
# 配置（Debug）
cmake -B build -S . -G "MinGW Makefiles" \
  -DCMAKE_PREFIX_PATH=E:/Qt/6.8.3/mingw_64 \
  -DCMAKE_BUILD_TYPE=Debug

# 构建
cmake --build build --config Debug

# 运行
./build/GatewayClient.exe
```

### Android 构建（arm64-v8a）

通过 Qt Creator 构建，或手动：

```bash
cmake -B build_android -S . -G "Ninja" \
  -DCMAKE_PREFIX_PATH=E:/Qt/6.8.3/android_arm64_v8a \
  -DANDROID_SDK_ROOT=E:/QTdown/Qt_android/android \
  -DANDROID_NDK_ROOT=E:/QTdown/Qt_android/android/ndk/26.1.10909125 \
  -DANDROID_ABI=arm64-v8a \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build_android
# 最终 APK 生成由 androiddeployqt 在 Qt Creator 中触发
```

### 构建目标

- **桌面构建产物** (`build/`)：`GatewayClient.exe` + 各 Debug/Release 子目录
- **Android 构建产物** (`build_android/`)：`libGatewayClient_arm64-v8a.so`，最终打包为 APK（包名 `com.smarthome.gatewayclient`）

### 编译关键点

- **必须使用 `QApplication` 而非 `QGuiApplication`**：Qt Charts 内部依赖 Widget 子系统，使用 `QGuiApplication` 会导致图表渲染崩溃。项目已在 `main.cpp` 中使用 `QApplication`。
- **`qt_import_qml_plugins()` 不可删除**：该调用将 QtCharts 等 QML 插件静态链接到二进制中，Android APK 打包时必不可少，缺少会导致运行时找不到图表组件。
- **MSVC 需要 `/Zc:__cplusplus` + `/utf-8`**：CMakeLists.txt 中已配置，确保 C++17 宏正确定义且源文件编码为 UTF-8。
- **Android 最终 APK 生成**：`cmake --build` 只生成 `.so` 文件。完整的 APK 打包（含资源、清单合并、签名）必须由 Qt Creator 触发 `androiddeployqt` 完成。

## 技术栈

| 层 | 技术 |
|---|---|
| UI | Qt Quick (QML) + Qt Quick Controls 2 |
| 图表 | Qt Charts（ChartPage 中使用） |
| 网络 | QTcpSocket，TCP 长连接，JSON 协议以 `\n` 分隔 |
| 构建 | CMake + Ninja（Android）/ MinGW Makefiles（桌面） |
| 语言标准 | C++17 |

## 架构分层（Android 新架构）

```
┌──────────────────────────────────────┐
│  QML 界面层（qml/）                    │
│  main.qml → StackView 路由             │
│    ├── LoginPage.qml      登录/连接    │
│    ├── DashboardPage.qml  传感器监控    │
│    ├── ControlPage.qml    设备控制      │
│    ├── ChartPage.qml      历史图表      │
│    ├── ThresholdPage.qml  阈值配置      │
│    └── AlertDialog.qml    告警弹窗      │
├──────────────────────────────────────┤
│  C++ 控制器（sensorcontroller.cpp/h）  │
│  Q_PROPERTY + Q_INVOKABLE → QML       │
│  业务逻辑：连接管理、数据存取、控制、阈值│
├──────────────────────────────────────┤
│  TCP 通信层（tcpclient.cpp/h）         │
│  QTcpSocket 封装，按 \n 拆包为 JSON    │
│  serverCommandReceived: type 20050-20059 │
│  dataReceived: 其他所有 JSON           │
├──────────────────────────────────────┤
│  Gateway 服务器                         │
└──────────────────────────────────────┘
```

### 关键设计点

1. **SensorController 是 QML 和 C++ 的唯一桥梁**。通过 `Q_PROPERTY` 暴露传感器数据和阈值，通过 `Q_INVOKABLE` 暴露操作方法。`main.cpp` 中通过 `rootContext()->setContextProperty("controller", &controller)` 注入 QML。

2. **数据流向是单向的**：C++ → QML 通过 `Q_PROPERTY` + `NOTIFY` 信号（自动更新绑定），QML → C++ 通过 `Q_INVOKABLE` 方法调用。跨边界传递复杂数据时使用 JSON 字符串（如 `historyReceived` 信号传递 `dataJson`），避免 QVariant 类型转换问题。

3. **StackView 导航**：`main.qml` 使用 `StackView` 管理页面栈，4 个主页面通过底部 TabBar 切换，使用 `stackView.replace()` 而非 `push()` 避免栈无限增长。LoginPage 使用 `push()` 进入 DashboardPage（首次进入），之后 TabBar 切换全部使用 `replace()`。

4. **遗留桌面代码**：`mainwindow.cpp/h` 和 `logindialog.cpp/h` 是旧的 QWidgets 版本，使用 SQLite 本地存储 + QChartView。已被 QML 新架构取代，**这些文件未被 CMakeLists.txt 引用，不会编译进任何构建目标**，可安全删除。

## QML 关键模式

### Inline Component 模式（Qt 6.5+）

DashboardPage、ControlPage、ThresholdPage 均使用 `component` 关键字定义内联可复用组件：

| 文件 | 内联组件 | 用途 |
|---|---|---|
| DashboardPage.qml | `SensorCard` | 传感器数据卡片（图标 + 数值 + 单位 + 颜色指示） |
| ControlPage.qml | `ControlSwitch` | 设备开关（名称 + Switch） |
| ControlPage.qml | `ControlSlider` | 滑块控制（名称 + Slider + 数值） |
| ThresholdPage.qml | `SettingCard` | 阈值配置卡片（名称 + Slider + 单位 + 回调） |

这些组件通过 `property` 定义参数接口，通过 `signal` / `property var onXxx`（回调函数）向上传递用户操作。

### AlertDialog 单例模式

`AlertDialog` 在 `main.qml` 中声明为 ApplicationWindow 级别的单例：
```
main.qml: AlertDialog { id: alertDialog }
```
触发方式：C++ `alertReceived` 信号 → `main.qml` 中的 `Connections` → 设置弹窗属性 → `alertDialog.open()`。
告警确认时调用 `controller.sendControl(20051, {...})` 发送回执。

### ChartPage 异步查询生命周期

```
Component.onCompleted → doQuery(sensorType, range)
  → controller.queryHistory(sensorType, range)
  → TCP 发送 40001 → 服务器返回 40002
  → C++ sensorcontroller 发出 historyReceived(sensorType, range, dataJson)
  → ChartPage Connections.onHistoryReceived
  → JSON.parse(dataJson) → append 到 LineSeries → 更新轴范围
```

查询期间使用 `Timer`（5 秒超时）检测无响应。使用 `isLoading` 标记防止重复查询。

### TCP 连接后的自动初始化流程

```
TCP connected
  → onConnected() 发送设备注册（type: 1）
  → QTimer::singleShot(500ms) 延迟发送当前阈值配置（type: 30001）
```

这个延迟是必要的，确保服务器在注册完成后再接收阈值配置。

## 通信协议（JSON/TCP）

每条消息以 `\n` 换行符分隔。核心 type 枚举：

| type 范围 | 方向 | 含义 |
|---|---|---|
| `1` | 发送 | 设备注册（room_id, is_gateway, device_id） |
| `10001`~`10008` | 接收 | 传感器数据（CO₂/温度/火焰/PM2.5/甲烷/光照/烟雾/湿度） |
| `20001`~`20008` | 发送 | 请求传感器读取对应关系同上 |
| `20013`~`20016` | 发送 | 设备控制（风扇/报警器/报警灯等） |
| `20050` | 接收 | 服务器告警通知（action, value, timestamp） |
| `20051` | 发送 | 告警确认回执 |
| `30001` | 发送 | 阈值配置下发 |
| `30002` | 接收 | 阈值确认（success） |
| `40001` | 发送 | 历史数据查询（sensor_type, range） |
| `40002` | 接收 | 历史数据响应（data[]） |

### 传感器类型编号映射

请求（20xxx）和接收（10xxx）之间的偏移固定为 **10000**：

| 编号 | 请求 type | 接收 type | 传感器 | 数据范围 |
|---|---|---|---|---|
| 1 | 20001 | 10001 | CO₂ | 连续值 (ppm) |
| 2 | 20002 | 10002 | 温度 | 连续值 (°C) |
| 3 | 20003 | 10003 | 火焰 | 布尔型 (>0 为检测到) |
| 4 | 20004 | 10004 | PM2.5 | 连续值 (µg/m³) |
| 5 | 20005 | 10005 | 甲烷 | 布尔型 (>0 为检测到) |
| 6 | 20006 | 10006 | 光照 | 连续值 (lux) |
| 7 | 20007 | 10007 | 烟雾 | 布尔型 (>0 为检测到) |
| 8 | 20008 | 10008 | 湿度 | 连续值 (%) |

布尔型传感器（火焰、甲烷、烟雾）在 C++ 端通过 `value > 0` 转换为 `bool` 类型暴露给 QML。

### 设备控制 type（仅发送，无对应接收）

| type | 设备 | 参数 |
|---|---|---|
| 20011 | LED 亮度 | `{ id, light }` |
| 20012 | RGB 灯光 | `{ red, green, blue }` |
| 20013 | 风扇开关 | `{ sw }` |
| 20014 | 报警器开关 | `{ sw }` |
| 20015 | 报警灯开关 | `{ sw }` |

## 文件清单速查

| 文件 | 角色 |
|---|---|
| `main.cpp` | 入口，创建 QApplication + SensorController，加载 QML |
| `sensorcontroller.h/cpp` | **核心**，QML ↔ C++ 桥接，所有业务逻辑 |
| `tcpclient.h/cpp` | TCP 套接字封装，半包缓冲按 `\n` 拆帧 |
| `mainwindow.h/cpp` | ⚠️ 遗留桌面版代码，**未编译**，可删除 |
| `logindialog.h/cpp` | ⚠️ 遗留桌面版代码，**未编译**，可删除 |
| `qml/main.qml` | 应用主窗口，StackView + TabBar 路由，AlertDialog 单例 |
| `qml/LoginPage.qml` | 登录/连接页面 |
| `qml/DashboardPage.qml` | 8 种传感器实时数据网格展示，内联 SensorCard 组件 |
| `qml/ControlPage.qml` | 风扇/报警器/报警灯/ LED/RGB 控制，内联 ControlSwitch/ControlSlider |
| `qml/ChartPage.qml` | 历史数据时间序列图表（Qt Charts），异步查询 + 超时处理 |
| `qml/ThresholdPage.qml` | CO₂/PM2.5/温度/甲烷告警阈值配置，内联 SettingCard |
| `qml/AlertDialog.qml` | 告警弹窗，含确认回执发送 |
| `qml.qrc` | QML 资源索引文件 |
| `CMakeLists.txt` | 构建配置，Qt6 模块依赖声明 |
| `android/AndroidManifest.xml` | Android 清单：网络权限、明文流量、QtActivity |

## 关键注意事项

- **TCP 消息分隔符**：每条消息必须附加 `\n`，tcpclient 的 `onReadyRead` 依赖此字符进行半包拆分。忘记添加会导致接收端无法解析。
- **Q_PROPERTY 类型一致性**：`sensorcontroller.h` 中 `Q_PROPERTY` 声明的类型必须与对应的 getter 返回类型完全一致，否则 QML 端属性绑定静默失效。
- **新增 QML 文件**：添加新 `.qml` 文件后需要同时更新两处：`qml.qrc`（资源声明）和 `CMakeLists.txt` 中的 `qt6_add_resources` FILES 列表。
- **ChartPage 时间解析**：服务器返回的时间格式为 `"yyyy-MM-dd HH:mm:ss"`（空格分隔），QML 端通过 `replace(" ", "T")` 转换为 JS Date 可解析的 ISO 格式。
- **无测试基础设施**：项目当前没有单元测试或集成测试框架。
