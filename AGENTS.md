# 仓库指南

## 项目结构与模块组织

当前仓库是早期阶段的 ESP BLE 项目，新增文件应遵循 ESP-IDF 的常见布局。应用入口代码放在 `main/`，可复用驱动、BLE 服务或业务组件放在 `components/<name>/`，测试代码放在 `tests/` 或 `components/<name>/test/`。根目录保留项目级构建与配置文件，例如 `CMakeLists.txt`、`sdkconfig.defaults`、可选的 `partitions.csv`。文档统一放在 `docs/`：项目文档固定放在 `docs/project/`，BLE 学习文档固定放在 `docs/ble-learning/`。

## 构建、测试与开发命令

请在已初始化 ESP-IDF 环境的 shell 中运行命令。

- `idf.py set-target esp32`：首次构建前选择目标芯片；如使用其他板卡，请替换为实际 target。
- `idf.py menuconfig`：配置 BLE、分区、日志级别和板级选项。
- `idf.py build`：编译固件并检查 CMake 配置。
- `idf.py -p /dev/ttyUSB0 flash monitor`：烧录开发板并打开串口日志。
- `idf.py fullclean`：当配置或构建缓存异常时清理生成文件。

## 代码风格与命名规范

遵循 ESP-IDF 示例项目中的 C/C++ 风格。建议使用 4 空格缩进；C 函数和文件名使用 `snake_case`；宏使用 `UPPER_SNAKE_CASE`；公开符号使用简短模块前缀，例如 `ble_gap_start()` 或 `sensor_ble_init()`。BLE UUID、特征名称和连接参数应定义为具名常量，避免散落在代码中的魔法值。

## 测试规范

所有功能修改至少应通过 `idf.py build`。组件测试尽量靠近被测组件，Unity 测试文件使用 `test_*.c`，pytest 硬件测试使用 `test_*.py`。涉及 BLE 行为时，请记录所需开发板、手机或工具、预期广播名称，以及需要验证的串口日志。

## 提交与 Pull Request 规范

本仓库采用 Conventional Commits，例如 `feat(ble): add advertising service`、`fix(build): correct target config` 或 `docs: add flashing notes`。Pull Request 应包含简要说明、关联 issue、目标硬件、ESP-IDF 版本、配置变更，以及实际运行过的构建、测试或烧录命令。

## 安全与配置建议

不要提交 Wi-Fi 密码、私钥、配对数据库或设备专属密钥。安全默认值可放入 `sdkconfig.defaults`，生成的 `sdkconfig` 应保持可审查。需要本地设置的环境变量请记录在 `docs/`。

## BLE 学习文档维护

`docs/ble-learning/` 用于存放面向学习的 BLE 文档，必须按编号组织，例如 `00-学习路线.md`、`01-BLE基础概念.md`、`02-广播与扫描.md`。文档应从基础概念逐步深入到 ESP-IDF 实现、GATT 服务、安全配对、低功耗和调试。每次新增 BLE 功能、修复关键问题或发现重要调试经验时，都应同步更新对应学习文档或索引，避免代码和学习资料脱节。

## 项目文档维护

`docs/project/` 用于存放项目自身的持续维护文档，记录“这个 BLE 项目正在做什么、为什么这样做、当前怎么运行”。项目初始化后，应优先维护项目概览、硬件与环境、构建烧录步骤、BLE 协议设计、架构决策、调试记录和迭代日志。每次改动工程结构、BLE 服务定义、硬件连接、配置项、测试方法或关键问题结论时，都要同步更新项目文档。

## 代理专用说明

当用户请求措辞不明确或不够准确时，先将理解后的请求用更清晰的中文复述给用户确认，再进行较大范围修改。
