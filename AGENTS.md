# 仓库指南

## 项目结构与模块组织

当前仓库是 ESP32-C5 BLE 项目，代码必须按 `app -> svc -> drv -> bsp` 分层组织。`main/` 只保留 ESP-IDF 入口，例如 `main/app_main.c`；项目代码放在 `components/`。`components/bsp/` 存放板级资源与 IO 定义；`components/drv/` 存放硬件驱动适配；`components/svc/` 存放业务服务；`components/app/` 存放应用编排。具体模块目录直接放 `.c` 与 `.h`，例如 `components/drv/rgb_led/drv_rgb_led.c` 和 `drv_rgb_led.h`，不再为每个模块额外拆 `include/`、`src/`。

ESP-IDF 组件粒度按层划分：`bsp/`、`drv/`、`svc/`、`app/` 这四个层级目录可各自维护一个 `CMakeLists.txt`；普通模块目录不单独放 `CMakeLists.txt`。依赖方向只能向下：`app` 可依赖 `svc`，`svc` 可依赖 `drv`，`drv` 可依赖 `bsp`，禁止反向依赖和跨层乱调。文档统一放在 `docs/`：项目文档在 `docs/project/`，BLE 学习文档在 `docs/ble-learning/`，板卡原理图、用户手册和整理笔记在 `docs/data/`。

## 构建、测试与开发命令

请在已初始化 ESP-IDF 环境的 shell 中运行命令。

- `idf.py set-target esp32c5`：首次构建前选择当前项目目标芯片；如使用其他板卡，请替换为实际 target。
- `idf.py menuconfig`：配置 BLE、分区、日志级别和板级选项。
- `idf.py build`：编译固件并检查 CMake 配置。
- `idf.py -p /dev/ttyACM0 flash monitor`：烧录开发板并打开串口日志。
- `idf.py fullclean`：当配置或构建缓存异常时清理生成文件。

## 代码风格与命名规范

遵循 ESP-IDF 示例项目中的 C/C++ 风格，使用 4 空格缩进。文件、函数、变量使用 `snake_case`，宏使用 `UPPER_SNAKE_CASE`。公开符号必须带层级和模块前缀，例如 `drv_rgb_led_init()`、`svc_led_effect_start()`、`svc_ble_start()`、`app_ble_demo_start()`。私有 `static` 函数也应保留模块前缀，方便跳转阅读。

变量命名必须区分作用域。文件内私有静态变量使用 `s_<module>_<name>`，例如 `s_drv_rgb_led_strip`；跨文件只读全局常量使用 `g_<module>_<name>`，例如 `g_bsp_board_info`。默认禁止跨文件可变全局变量；确需使用时必须使用 `g_` 前缀，并在声明处说明所有权、线程保护方式和写入入口。局部变量不加作用域前缀，但命名必须表达含义。BLE UUID、特征名称、GPIO 编号和连接参数应定义为具名常量，避免魔法值。

## 测试规范

所有功能修改至少应通过 `idf.py build`。组件测试尽量靠近被测组件，Unity 测试文件使用 `test_*.c`，pytest 硬件测试使用 `test_*.py`。涉及 BLE 行为时，请记录所需开发板、手机或工具、预期广播名称，以及需要验证的串口日志。

## 提交与 Pull Request 规范

提交说明必须使用中文。可以保留 Conventional Commits 的类型前缀，例如 `feat(ble): 增加广播服务`、`fix(build): 修正目标芯片配置` 或 `docs: 更新烧录说明`。Pull Request 应包含简要说明、关联 issue、目标硬件、ESP-IDF 版本、配置变更，以及实际运行过的构建、测试或烧录命令。

## 安全与配置建议

不要提交 Wi-Fi 密码、私钥、配对数据库或设备专属密钥。安全默认值可放入 `sdkconfig.defaults`，生成的 `sdkconfig` 应保持可审查。需要本地设置的环境变量请记录在 `docs/`。

## BLE 学习文档维护

`docs/ble-learning/` 用于存放面向学习的 BLE 文档，必须按编号组织，例如 `00-学习路线.md`、`01-BLE基础概念.md`、`02-广播与扫描.md`。文档应从基础概念逐步深入到 ESP-IDF 实现、GATT 服务、安全配对、低功耗和调试。每次新增 BLE 功能、修复关键问题或发现重要调试经验时，都应同步更新对应学习文档或索引，避免代码和学习资料脱节。

## 项目文档维护

`docs/project/` 用于存放项目自身的持续维护文档，记录“这个 BLE 项目正在做什么、为什么这样做、当前怎么运行”。项目初始化后，应优先维护项目概览、硬件与环境、构建烧录步骤、BLE 协议设计、架构决策、调试记录和迭代日志。每次改动工程结构、BLE 服务定义、硬件连接、配置项、测试方法或关键问题结论时，都要同步更新项目文档。

## 代理专用说明

当用户请求措辞不明确或不够准确时，先将理解后的请求用更清晰的中文复述给用户确认，再进行较大范围修改。
