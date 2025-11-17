# STM32F103 Bootloader

STM32F103 微控制器的极简引导程序，Flash 限制 8KB。

## 功能特性

- ✅ 应用程序引导和验证
- ✅ 固件升级（CAN/BLE/RF 框架已就绪）
- ✅ Flash 操作（擦除、写入、读取、校验）
- ✅ 独立看门狗保护（3秒超时）
- ✅ 升级标志管理
- ✅ SWD 调试支持（禁用 JTAG 以释放引脚）

## 内存布局

```
0x08000000 - 0x08001FFF (8KB)   : Bootloader
0x08002000 - 0x0800EFFF (52KB)  : Application
0x0800F000 - 0x0800FFFF (4KB)   : Configuration
```

**当前大小**: 4012 字节 / 8192 字节 (48.97%)

## 快速开始

### 1. 编译

```bash
# Windows
mkdir build
cd build
cmake ..
ninja bootloader
```

### 2. 烧录

**使用脚本（推荐）**:
```cmd
flash_bootloader.bat
```

**手动使用 STM32CubeProgrammer**:
1. 打开 STM32CubeProgrammer
2. 通过 ST-LINK (SWD) 连接
3. 加载 `build/bootloader.hex`
4. 烧录到地址 `0x08000000`

### 3. 验证

- LED (PC13) 以 1Hz 频率闪烁 → Bootloader 正常运行
- 可随时重新连接调试器 → SWD 已启用

## 硬件连接

```
ST-Link    →    STM32F103
SWDIO      →    PA13
SWCLK      →    PA14
GND        →    GND
3.3V       →    VCC（如板子有独立供电则可选）
```

## 环境要求

- **CMake** >= 3.22
- **ARM GCC 工具链** (arm-none-eabi-gcc)
- **Ninja** 构建系统
- **STM32CubeProgrammer** 用于烧录

## 重要说明

### ⚠️ SWD 调试接口

本 bootloader **保持 SWD 启用**以便调试，同时**禁用 JTAG** 以释放引脚：
- ✅ **PA13, PA14** (SWD) - 用于调试/烧录
- ✅ **PA15, PB3, PB4** (JTAG) - 释放用于 GPIO

**为什么？** 早期版本使用 `__HAL_AFIO_REMAP_SWJ_DISABLE()` 完全禁用了调试接口。已修复为 `__HAL_AFIO_REMAP_SWJ_NOJTAG()`。

### 应用程序开发

您的应用固件应该：
1. **从 `0x08002000` 开始**（8KB 偏移）
2. **设置向量表偏移**: `SCB->VTOR = 0x08002000;`
3. **大小限制**: 最大 52KB

示例链接脚本：
```ld
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08002000, LENGTH = 52K
  RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 20K
}
```

## 故障排除

### 烧录后无法连接调试器

**解决方案**: 已修复！请确保使用最新版本，SWD 接口已正确启用。

### LED 不闪烁

1. 检查 bootloader 是否烧录到 `0x08000000`
2. 检查 LED 连接（PC13）
3. 检查电源供电（3.3V）

### 看门狗复位循环

如果 LED 每 3 秒快速闪烁一次，说明 bootloader 卡在 `Error_Handler()` 中。通常原因：
- `0x08002000` 处的应用程序无效
- Flash 损坏

**修复**: 擦除应用区域并确保烧录有效固件。

## 项目结构

```
.
├── Core/
│   ├── Inc/           # 主头文件和配置
│   └── Src/           # 主 bootloader 代码
├── BSP/               # 板级支持包
│   ├── Inc/           # BSP 头文件
│   └── Src/           # Flash、跳转、看门狗、升级标志
├── Drivers/           # STM32 HAL 和 CMSIS
├── build/             # 编译输出（自动生成）
├── CMakeLists.txt     # 构建配置
├── flash_bootloader.bat      # 烧录脚本
├── test_connection.bat       # 测试 ST-Link 连接
└── check_bootloader_size.sh  # 大小检查脚本
```

## 版本历史

### v1.0.0 (2025-01-XX)
- ✅ 核心 bootloader 功能
- ✅ 修复 SWD 调试接口（关键 bug 修复）
- ✅ 优化至 4KB（50% Flash 使用率）
- ✅ 看门狗保护
- ✅ 应用程序验证和跳转

## 许可证

Copyright (c) 2025 TT1nker  
GitHub: [TT1nker](https://github.com/TT1nker)  
联系方式: hostsjim22@gmail.com

本软件按"原样"提供，不提供任何形式的担保。

## 贡献

欢迎贡献！请确保：
- 代码编译无警告
- Bootloader 大小保持在 8KB 以下
- SWD 调试功能正常
