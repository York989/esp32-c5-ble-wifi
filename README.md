# ESP32-C5 BLE/Wi-Fi 项目

这是一个基于 ESP-IDF 的 ESP32-C5 项目。当前阶段以 BLE 外设开发为主，代码从 NimBLE GATT Server 示例演进而来；Wi-Fi 相关能力会在后续迭代中逐步接入。

## 当前功能

- 使用 NimBLE 初始化 BLE Host 和 Controller。
- 以 `NimBLE_GATT` 作为 BLE 设备名进行可连接广播。
- 提供 Heart Rate Service：UUID `0x180D`。
- 提供 Heart Rate Measurement Characteristic：UUID `0x2A37`，支持读取和 indication。
- 提供 Automation IO Service：UUID `0x1815`。
- 提供 LED 写入特征：写入非零值恢复板载 RGB LED 流水彩虹，写入 `0x00` 熄灭。
- 板载 RGB LED 使用 `GPIO27`，上电后自动循环彩虹渐变。
- 每秒生成一次模拟心率数据，并在订阅 indication 后主动上报。

## 硬件与环境

- 目标芯片：`esp32c5`
- ESP-IDF：`v6.0.1`
- 当前串口：`/dev/ttyACM0`
- 默认广播名：`NimBLE_GATT`
- 已验证日志关键字：`advertising started!`

WSL2 默认不能直接使用 Windows 主机蓝牙。如果需要在 WSL2 内扫描 BLE 广播，需要把 USB 蓝牙适配器通过 `usbipd` 绑定进 WSL2；否则可先用手机 BLE Scanner 或 nRF Connect 验证广播。

## 项目结构

```text
.
├── main/                    # ESP-IDF 应用入口、BLE GAP/GATT、LED 和心率模拟逻辑
│   ├── include/             # 项目头文件
│   └── src/                 # 功能实现
├── docs/
│   ├── project/             # 项目配置、构建、调试和迭代文档
│   ├── ble-learning/        # BLE 学习路线和学习笔记
│   └── data/                # 板卡原理图、用户手册和整理笔记
├── sdkconfig.defaults*      # 各目标芯片的默认配置
├── dependencies.lock        # ESP-IDF 组件依赖锁定文件
└── AGENTS.md                # 仓库协作与代理规则
```

`build/`、`managed_components/`、`sdkconfig` 和 `.vscode/` 是本地生成或本机配置内容，不提交到仓库。

## 构建、烧录与运行

先进入已初始化 ESP-IDF 的 shell，或手动加载环境：

```bash
. /home/u888/esp/.espressif/v6.0.1/esp-idf/export.sh
```

常用命令：

```bash
idf.py set-target esp32c5
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

退出串口监视器：

```text
Ctrl+]
```

如果使用 ESP-IDF MCP，也可以直接执行项目构建工具验证当前工程。

## BLE 验证方式

1. 烧录并打开串口监视器。
2. 确认日志中出现 `advertising started!`。
3. 使用手机 BLE Scanner、nRF Connect 或可用的主机 BLE 扫描工具搜索 `NimBLE_GATT`。
4. 连接后检查 GATT 服务：
   - `0x180D`：心率服务。
   - `0x2A37`：心率测量，支持读取和 indication。
   - `0x1815`：Automation IO 服务，包含 LED 写入特征。

## 文档维护

- 项目运行方式、环境配置和调试结论写入 `docs/project/`。
- BLE 学习资料按顺序写入 `docs/ble-learning/`。
- 板卡原理图、用户手册和整理笔记写入 `docs/data/`。
- BLE 服务、UUID、广播字段或硬件环境变化后，需要同步更新 README 和对应项目文档。

## 提交规范

提交说明使用中文。可以保留 Conventional Commits 的类型前缀，例如：

```text
docs: 更新项目说明
feat(ble): 增加心率 indication
fix(build): 修正 ESP32-C5 默认配置
```
