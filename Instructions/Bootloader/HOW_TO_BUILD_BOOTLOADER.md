# 如何将代码编译成 Bootloader

## 关键步骤

要让你的代码成为 Bootloader，需要做三件事：

1. **使用 Bootloader 的链接脚本**（指定起始地址 0x08000000）
2. **编译 Bootloader 的源文件**（不是应用程序的源文件）
3. **烧录到 Flash 的 0x08000000 地址**

## 1. 链接脚本的区别

### 应用程序的链接脚本
```ld
/* STM32F103XX_FLASH.ld */
FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 64K  /* 整个 64KB */
```

### Bootloader 的链接脚本
```ld
/* Bootloader/STM32F103XX_BOOTLOADER.ld */
FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 4K   /* 只有 4KB */
```

**关键区别：**
- 应用程序：可以使用整个 64KB Flash
- Bootloader：只使用前 4KB（0x08000000 - 0x08000FFF）

## 2. 编译配置

### 在 CMakeLists.txt 中添加 Bootloader 目标

```cmake
# 应用程序目标（已存在）
add_executable(${CMAKE_PROJECT_NAME})
# ... 应用程序配置 ...

# Bootloader 目标（新增）
add_executable(bootloader)

# Bootloader 源文件
target_sources(bootloader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/Core/Src/bootloader_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/BSP/Src/bsp_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/BSP/Src/bsp_jump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/BSP/Src/bsp_upgrade_flag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/BSP/Src/bsp_watchdog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32f103xb.s  # 启动文件
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/system_stm32f1xx.c  # 系统初始化
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32f1xx_hal_msp.c  # MSP 回调
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32f1xx_it.c  # 中断处理
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/syscalls.c
)

# Bootloader Include 路径
target_include_directories(bootloader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/BSP/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Core/Include
)

# Bootloader 使用 Bootloader 链接脚本
set_target_properties(bootloader PROPERTIES
    LINK_FLAGS "-T \"${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/STM32F103XX_BOOTLOADER.ld\""
)

# Bootloader 链接 HAL 库
target_link_libraries(bootloader
    stm32cubemx
)
```

## 3. 编译命令

### 编译应用程序
```bash
mkdir -p build
cd build
cmake ..
make Temperature    # 编译应用程序
```

### 编译 Bootloader
```bash
cd build
make bootloader     # 编译 Bootloader
```

## 4. 烧录 Bootloader

### 使用 ST-Link
```bash
# 烧录 Bootloader 到 0x08000000
st-flash write bootloader.elf 0x08000000

# 或者使用 OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
    -c "program bootloader.elf verify reset exit"
```

### 使用 J-Flash 或其他工具
- 选择 Bootloader 的 .elf 或 .bin 文件
- 设置起始地址：0x08000000
- 烧录

## 5. 关键区别总结

| 项目 | 应用程序 | Bootloader |
|------|---------|-----------|
| **链接脚本** | `STM32F103XX_FLASH.ld` | `Bootloader/STM32F103XX_BOOTLOADER.ld` |
| **Flash 起始地址** | 0x08001000 | 0x08000000 |
| **Flash 大小** | 12KB (或更多) | 4KB |
| **主函数** | `main()` (Core/Src/main.c) | `main()` (Bootloader/Core/Src/bootloader_main.c) |
| **编译目标** | `Temperature` | `bootloader` |
| **输出文件** | `Temperature.elf` | `bootloader.elf` |

## 6. 验证 Bootloader 是否正确

### 方法1：查看 map 文件
```bash
# 编译后查看 bootloader.map
grep "Reset_Handler" build/bootloader.map
# 应该显示地址在 0x08000000 附近
```

### 方法2：使用 objdump
```bash
arm-none-eabi-objdump -h build/bootloader.elf
# 查看 .isr_vector 段的地址，应该是 0x08000000
```

### 方法3：使用 readelf
```bash
arm-none-eabi-readelf -l build/bootloader.elf
# 查看程序头，确认起始地址是 0x08000000
```

## 7. 完整流程示例

```bash
# 1. 配置 CMake（添加 Bootloader 目标）
# 编辑 CMakeLists.txt（如上所示）

# 2. 编译 Bootloader
cd build
cmake ..
make bootloader

# 3. 检查输出
ls -lh build/bootloader.elf
arm-none-eabi-size build/bootloader.elf
# 应该显示大小 < 4KB

# 4. 烧录 Bootloader
st-flash write build/bootloader.bin 0x08000000

# 5. 验证
# 复位 MCU，应该进入 Bootloader
# 可以通过 LED 闪烁或串口输出确认
```

## 8. 重要注意事项

1. **首次烧录 Bootloader**：
   - 需要先擦除整个 Flash
   - 然后烧录 Bootloader 到 0x08000000
   - 之后可以单独更新应用程序

2. **Bootloader 大小限制**：
   - 必须 < 4KB
   - 如果超过，需要优化代码或增加 Bootloader 区域大小

3. **应用程序适配**：
   - 应用程序的链接脚本需要修改起始地址为 0x08001000
   - 应用程序的 SystemInit() 需要设置 `SCB->VTOR = 0x08001000`

4. **调试**：
   - Bootloader 和应用程序可以分别调试
   - 需要配置不同的调试脚本

## 总结

**你的代码成为 Bootloader 的关键：**
1. ✅ 使用 `Bootloader/STM32F103XX_BOOTLOADER.ld` 链接脚本
2. ✅ 编译 Bootloader 的源文件（不是应用程序的）
3. ✅ 烧录到 0x08000000 地址
4. ✅ 确保代码大小 < 4KB

**编译系统通过链接脚本知道：**
- 代码应该放在哪里（0x08000000）
- 代码有多大（4KB）
- 入口点是什么（Reset_Handler）

**硬件通过向量表知道：**
- 复位后从 0x08000000 读取向量表
- 跳转到 Reset_Handler
- Reset_Handler 调用你的 main()

这就是你的代码如何成为 Bootloader 的完整过程！

