# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

SHServer 是一个基于 Qt6 的智能家居服务端程序，用于室内空气质量监控与 AI 联动告警。服务端通过 TCP 协议接收传感器网关上报的数据，存入 MySQL 数据库，并根据可配置的阈值自动向网关下发风扇、报警灯、报警器等设备的控制指令。

### 项目生态

本仓库是智能家居系统的**服务端**，同级目录下还有：
- `SHGateway/` — 传感器网关（C++ Qt，负责采集硬件传感器数据并上报给 SHServer）
- `SHGatewayClient/` — 桌面客户端（C++ Qt，用户交互界面）
- `SHGatewayAndroidClient/` — 安卓客户端
- `App/DeviceVirtual V1.0.0/` — 设备虚拟仿真器（用于开发调试）

## 构建与运行

```bash
# 使用 Qt Creator 打开 CMakeLists.txt 进行构建
# 或命令行构建：
cmake -B build -DCMAKE_PREFIX_PATH=<Qt6安装路径> -G "MinGW Makefiles"
cmake --build build --config Debug

# 运行（可执行文件名 = 项目名 = SHServer）
./build/SHServer
```

- **构建系统**: CMake 3.19+
- **Qt 版本**: 6.5+，依赖 `Core`、`Network`、`Sql` 模块
- **数据库驱动**: QMYSQL（MySQL）
- **编译器**: MinGW 64-bit (Qt 6.8.3)
- **IDE**: Qt Creator（项目目录下已有 `.qtcreator` 配置）
- 项目当前无自动化测试、无 CI/CD 配置

## 架构概览

### 启动流程

```
main.cpp → SystemControl::systemInit()
  ├── ConfigContext::setConfigFilePath()     # 设定配置路径（根据 exe 名推导）
  ├── LogContext::getObject()                # 初始化日志单例
  ├── NetContext::getObject()                # 初始化网络单例（创建 MyTcpServer）
  ├── ConfigContext::get*()                  # 读取服务器/数据库配置
  ├── DBContext → setConfig() → initContext() # 初始化数据库连接池
  └── SystemControl::systemStart()
       └── NetContext → startServer(port)    # 开始 TCP 监听
```

### 模块职责

| 模块 | 类 | 职责 |
|------|-----|------|
| 入口 | `main.cpp` | QCoreApplication 初始化，根据 exe 文件名推导 INI 配置文件路径 |
| 编排 | `SystemControl` | 按序初始化各子系统，最后启动 TCP 监听 |
| 配置 | `ConfigContext` | 静态方法从 INI 文件读取 server/DB/MQTT 参数，缺省值自动写入 |
| 日志 | `LogContext` | 单例，带时间戳输出到控制台 |
| 网络 | `NetContext` + `MyTcpServer` + `MyTcpSocket` + `ThreadsHandle` | TCP 服务端，每连接一个线程，JSON 帧协议通信 |
| 数据库 | `DBContext` + `DBExec` | 连接池（QSemaphore 控制并发），支持传感器数据、用户、好友、群聊 |
| MQTT | `MQTTContext` + `MQTTPublisher` | 连接池，用于对外推送消息 |
| 阈值 | `Threshold` | 结构体，CO2/PM2.5/温度/甲烷/冷却延迟的阈值配置 |

### 关键设计模式

- **单例**: `LogContext`、`NetContext`、`DBContext`、`MQTTContext` 均通过 `static getObject()` 获取
- **连接池 + 信号量**: `DBContext` 和 `MQTTContext` 使用 `QSemaphore::acquire()` 阻塞获取，`release*()` 归还
- **每连接一线程**: `MyTcpServer::incomingConnection()` 通过 `ThreadsHandle::getTh()` 获取线程，`socket->moveToThread(th)` 将连接移入

### 通信协议

自定义 JSON-over-TCP 协议，按 `{}` 括号匹配拆帧（持久化缓冲区防半包）。消息类型号：

| type 范围 | 方向 | 说明 |
|-----------|------|------|
| 1 | 客户端→服务端 | 设备注册（room_id, device_id, is_gateway） |
| 10001~10008 | 网关→服务端 | 传感器数据上报（CO2/温度/火光/PM2.5/甲烷/光照/烟雾/湿度） |
| 20001~20008 | 客户端→网关 | 传感器数据请求（由服务端转发） |
| 20011~20015 | 客户端→网关 | 设备控制指令（风扇/报警器/报警灯开关） |
| 20013 | 服务端→网关 | 风扇开关（sw: true/false） |
| 20014 | 服务端→网关 | 报警器开关 |
| 20015 | 服务端→网关 | 报警灯开关 |
| 20050 | 服务端→所有客户端 | 告警广播通知 |
| 30001~30002 | 客户端↔服务端 | 阈值配置/确认 |
| 40001~40002 | 客户端↔服务端 | 历史数据查询/响应 |

### 联动控制逻辑（`MyTcpSocket::checkAndTrigger`）

当传感器数据上报后自动判定：

- **CO2/PM2.5 超标** → 开风扇 + 报警灯，冷却延迟后可**自动关闭**，超标期间支持手动关闭
- **甲烷/烟雾/火光** → 开报警器 + 报警灯，**仅手动关闭**（属于紧急类型），恢复正常后可重新检测
- **关闭条件**: 所有同类传感器恢复正常 + 冷却延迟已过（`fanCooldownSeconds` 配置）

紧急传感器（`emergencySensors`）活跃时会阻止报警灯自动关闭。手动关闭设备时同步清理服务端的触发追踪状态。

### 数据库表

- 传感器表: `Co2`, `Temperature`, `Humidity`, `PM25`, `Light`, `Smoke`, `Methane`, `Fire`（结构：value + time）
- 用户系统: `chat_userinfo`（用户认证），`chat_userinfo_exp`（用户扩展信息），`chat_friend`（好友关系），`chat_request_friend`（好友申请）
- 群聊系统: `chat_groupinfo`, `chat_group_user`, `chat_group_admin_user`

### 配置文件

INI 格式，自动生成默认值。关键配置项（`[server]` 段）：

| 键 | 默认值 | 说明 |
|-----|--------|------|
| `listen_port` | 10086 | TCP 监听端口 |
| `client_limit` | 500 | 最大客户端连接数 |
| `db_limit` | 50 | 数据库连接池大小 |
| `db_host` | 127.0.0.1 | 数据库地址 |
| `db_port` | 10086 | 数据库端口 |
| `db_username` | root | 数据库用户名 |
| `db_password` | 123456 | 数据库密码 |
| `db_name` | smarthome | 数据库名 |

## 注意事项

- 所有代码注释使用中文
- `MyTcpSocket` 使用 `friend class` 声明让 `MyTcpServer` 访问私有成员，这不是 bug 而是一种有意设计
- `DBExec` 的构造函数中 `QSqlDatabase::addDatabase("QMYSQL", conName)` 要求连接名全局唯一，每个 `DBExec` 使用 `con_1`、`con_2`...作为连接名
- MQTT 模块（`MQTTContext` / `MQTTPublisher`）代码已写但 `systemInit()` 中尚未调用初始化，处于预留状态
- 配置文件路径由 exe 文件名推导：取 baseName 加 `.ini` 后缀存入 `QStandardPaths::AppConfigLocation`

## 已知安全风险

- **SQL 注入**: `DBExec` 中所有 SQL 查询使用字符串拼接构造，未使用参数化查询（`QSqlQuery::prepare()` + `bindValue()`）
- **密码存储**: 密码使用 MD5 哈希存储（已过时，应迁移至 bcrypt/argon2）
- **网络明文**: TCP 通信和 MQTT 连接均未加密，无 TLS
