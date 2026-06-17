# 硬件资料索引

本目录存放当前 ESP32-C5 开发板的原始硬件资料和基于 PDF 整理出的开发笔记。

## 原始 PDF

- `E101-C5WN8系列开发板+datasheet_CN_v1.0 (1).pdf`：E101-C5WN8 系列开发板用户手册。
- `E101-C5WN8-PS-TB-SCH (1).pdf`：E101-C5WN8-PS-TB 原理图。

## 整理文档

- `E101-C5WN8开发板用户手册整理.md`：从用户手册中整理出的模块能力、接口、供电方式、管脚定义和开发注意事项。
- `E101-C5WN8原理图整理.md`：从原理图视觉检查中整理出的电源、USB、UART、按键、自动下载、LED 和排针关系。

## 处理方式

本目录的 Markdown 资料由 PDF skill 辅助处理：

- 使用 `pdfinfo` 检查 PDF 页数、页面尺寸和元信息。
- 使用 `pdftotext -layout` 抽取用户手册正文。
- 使用 `pdftoppm` 将 PDF 渲染为 PNG 后进行视觉检查。

注意：原理图 PDF 没有可抽取文本，整理内容来自渲染图像的人工核对。后续如果替换 PDF，需要重新生成整理文档，并同步检查 `README.md`、`docs/project/README.md` 和相关项目文档入口。
