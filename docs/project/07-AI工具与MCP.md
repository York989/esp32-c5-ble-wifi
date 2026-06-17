# AI 工具与 MCP

## ESP-IDF MCP

当前 ESP-IDF v6.0.1 自带 MCP 入口，位置如下：

```text
/home/u888/esp/.espressif/v6.0.1/esp-idf/tools/idf_py_actions/mcp_ext.py
```

推荐使用 EIM 启动，而不是手动 source `export.sh`：

```bash
eim run "idf.py -C /home/u888/esp/project/ble mcp-server"
```

已验证 `eim run "idf.py --help"` 能看到 `mcp-server` 命令，`eim run "idf.py mcp-server --help"` 能显示帮助。ESP-IDF MCP 提供构建、设置 target、烧录、清理等工具，以及项目配置、构建状态、设备列表等资源。

当前以 Windows 侧 Codex 配置为主，但 ESP-IDF MCP 实际运行在 WSL2。共享的 Windows `config.toml` 中已使用 `wsl.exe` 转发到 Ubuntu：

```toml
[mcp_servers.esp_idf_wsl]
type = "stdio"
command = "wsl.exe"
args = [
  "-d", "Ubuntu",
  "--cd", "/home/u888/esp/project/ble",
  "--",
  "eim", "run", "idf.py -C /home/u888/esp/project/ble mcp-server"
]
startup_timeout_sec = 120
```

该配置保留 Windows 侧模型供应商、key 和通用 Codex 设置为唯一主配置，同时把 ESP-IDF MCP 的实际执行环境固定到 WSL2。

## WSL2 全局 skills

WSL2 内的 Codex 全局 skill 目录是：

```text
/home/u888/.codex/skills/
```

当前已安装：

- `find-skills`：来源 `https://www.skills.sh/vercel-labs/skills/find-skills`
- `brainstorming`：来源 `https://www.skills.sh/obra/superpowers/brainstorming`
- `content-research-writer`：来源 `https://skills.sh/composiohq/awesome-codex-skills/content-research-writer`

安装落点：

```text
/home/u888/.codex/skills/find-skills/SKILL.md
/home/u888/.codex/skills/brainstorming/SKILL.md
/home/u888/.codex/skills/content-research-writer/SKILL.md
```

## 当前 skill 调研结论

官方 `openai/skills` 仓库中没有发现面向 ESP-IDF、ESP32、BLE 或单片机固件开发的通用 skill。此前未在官方仓库找到 `find-skills`，后来确认它位于 `vercel-labs/skills`，可通过 `skills.sh` 页面安装。`skill-of-skills` 是第三方 skill/工具目录，不是 Codex skill 本体，因为仓库根目录没有 `SKILL.md`。

已发现的相关资源：

- `rymcu/esp-brookesia-build-flash`：真实 Codex skill，但只适合 ESP-Brookesia、Windows PowerShell 和 rymcu_bigsmart 板卡流程。
- `aklofas/kicad-happy`：面向 KiCad/PCB 的 AI skill 资源，偏硬件设计，不是 ESP-IDF 固件开发。
- `atopile/atopile`：用代码设计电路板，适合硬件原理设计探索。
- `ailyProject/aily-blockly`：面向 Arduino、MicroPython、ESP32、STM32 的硬件开发 IDE，风险级别较高，不建议直接作为当前 BLE 项目依赖。
