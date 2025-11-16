# STM32F103 Bootloader

STM32F103 微控制器的独立固件引导程序，Flash 限制 4KB。

## 功能特性

- 应用程序引导和验证
- 固件升级（CAN/BLE/RF）
- Flash 操作（擦除、写入、读取、校验）
- 看门狗保护（3秒超时）
- 升级标志管理

## 内存布局

```
0x08000000 - 0x08000FFF (4KB)   : Bootloader
0x08001000 - 0x0800EFFF (56KB)  : Application
0x08007000 - 0x08007FFF (4KB)   : Configuration
```

## 构建

```bash
mkdir build && cd build
cmake ..
make bootloader
st-flash write bootloader.bin 0x08000000
```

## 要求

- CMake >= 3.22
- ARM GCC 工具链 (arm-none-eabi-gcc)

## 状态

- ✅ 核心功能完成
- ⚠️ 升级协议待实现

## 许可证

Copyright (c) 2025 TT1nker  
联系方式: hostsjim22@gmail.com

按"原样"提供，不提供担保。
