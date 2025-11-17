# 项目文档索引

## 📚 文档结构

### 🏗️ 架构设计 (Architecture/)
- [OTA Bootloader 实现总结](./Architecture/OTA_BOOTLOADER_SUMMARY.md) - OTA Bootloader 实现要点和架构设计

### 🔧 编译和配置 (Setup/)
- [编译器 vs IDE Include 路径](./Setup/COMPILER_VS_IDE_INCLUDE.md) - 编译器如何解决同名文件问题
- [多个 main.h 文件说明](./Setup/MULTIPLE_MAIN_H_EXPLANATION.md) - 为什么有两个 main.h 及如何解决

### 🚀 Bootloader (Bootloader/)
- [Bootloader 实现文档](./Bootloader/README.md) - Bootloader 基础框架和功能说明
- [Bootloader 编译配置](./Bootloader/COMPILE_SETUP.md) - Include 路径和编译配置
- [Bootloader Include 路径](./Bootloader/INCLUDE_PATHS.md) - Include 路径配置说明

### 📦 模块文档
- [BSP 层文档](../BSP/README.md) - 板级支持包说明
- [App 层文档](../App/README.md) - 应用层说明

## 🗂️ 快速导航

### 新手入门
1. 先阅读 [BSP 层文档](../BSP/README.md) 了解架构
2. 查看 [App 层文档](../App/README.md) 了解应用层
3. 阅读 [Bootloader 实现文档](./Bootloader/README.md) 了解 Bootloader

### 遇到编译问题
1. [多个 main.h 文件说明](./Setup/MULTIPLE_MAIN_H_EXPLANATION.md) - 了解问题原因
2. [编译器 vs IDE Include 路径](./Setup/COMPILER_VS_IDE_INCLUDE.md) - 理解编译器工作原理
3. [Bootloader 编译配置](./Bootloader/COMPILE_SETUP.md) - 配置编译环境

### 实现 OTA 功能
1. [OTA Bootloader 实现总结](./Architecture/OTA_BOOTLOADER_SUMMARY.md) - 总体架构和实现要点
2. [Bootloader 实现文档](./Bootloader/README.md) - 具体实现细节

## 📝 文档说明

所有项目相关的文档都集中在这个 `Instructions/` 文件夹中，保持项目根目录整洁。

- **Architecture/** - 架构设计文档
- **Setup/** - 编译、配置、环境设置文档
- **Bootloader/** - Bootloader 相关文档

模块级别的 README.md（如 `BSP/README.md`、`App/README.md`）保留在各自目录中，方便快速查看。

