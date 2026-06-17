# BLE 调试工具

## 当前环境

- 系统：WSL2 Ubuntu 26.04
- 已有 BlueZ 基础工具：`bluetoothctl`、`btmon`、`btmgmt`、`hciconfig`、`hcitool`、`gatttool`
- 当前限制：WSL2 内未发现 HCI 蓝牙适配器，`/sys/class/bluetooth` 为空。

WSL2 默认不会直接使用 Windows 主机蓝牙。要在 WSL2 内执行 BLE 扫描、连接和 GATT 调试，通常需要把 USB 蓝牙适配器通过 `usbipd` 绑定到 WSL2。

## 已安装的用户级工具

Python BLE 工具安装在：

```text
/home/u888/.local/venvs/ble-debug/
```

包含：

- `bleak`：跨平台 BLE 扫描、连接、GATT 读写库。
- `bumble`：Python Bluetooth/BLE 协议栈和调试工具集。
- `aioblescan`：BLE 广播扫描库。

已创建命令入口：

```bash
ble-status
ble-scan 10
ble-gatt-dump <transport> [address-or-name]
```

`ble-status` 用于检查 WSL、BlueZ 工具和适配器状态。`ble-scan` 使用 Bleak 扫描 BLE 广播；当前无 HCI 适配器时会直接提示。`ble-gatt-dump` 调用 Bumble 的 GATT dump 工具。

## 需要 sudo 的系统工具

当前会话没有免密 sudo，因此未安装以下 apt 包。需要时在终端手动执行：

```bash
sudo apt update
sudo apt install -y bluez-tools bluetooth tshark wireshark-common python3-bleak
```

用途：

- `bluez-tools`：补充 BlueZ 命令行工具。
- `bluetooth`：BlueZ 蓝牙服务元包。
- `tshark` / `wireshark-common`：分析 `btmon`、HCI、pcap 抓包。
- `python3-bleak`：系统级 Bleak 包；用户级虚拟环境已安装，可选。

## Skill 与 MCP 结论

未发现官方 BLE 调试 skill 或官方 BLE MCP。`skills.sh` 搜索到的相关 skill 主要是 iOS CoreBluetooth 或低安装量 Arduino/ESPHome 方向，不适合作为当前 ESP-IDF BLE 项目默认依赖。

网络上存在第三方 BLE MCP，例如通用 BLE MCP 或设备特定 MCP，但当前 WSL2 没有可见蓝牙适配器，接入后大概率无法工作。等 USB 蓝牙适配器绑定进 WSL2 并验证 `ble-scan` 可用后，再评估是否接入第三方 BLE MCP。
