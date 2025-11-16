# Bootloader HAL Library

## 说明

这是 Bootloader 专用的 HAL 库副本，使 Bootloader 可以独立编译，不依赖主工程的 HAL 库。

## 目录结构

```
Bootloader/Drivers/
├── STM32F1xx_HAL_Driver/
│   ├── Inc/          # HAL 驱动头文件（全部）
│   └── Src/          # HAL 驱动源文件（仅需要的模块）
└── CMSIS/
    ├── Core/Include/     # CMSIS Core 头文件
    └── Device/ST/STM32F1xx/Include/  # STM32F1xx 设备头文件
```

## 已启用的 HAL 模块

根据 Bootloader 的需求，以下模块已启用：

- `HAL_CORTEX_MODULE_ENABLED` - Cortex-M 核心功能
- `HAL_DMA_MODULE_ENABLED` - DMA 功能
- `HAL_FLASH_MODULE_ENABLED` - Flash 操作
- `HAL_EXTI_MODULE_ENABLED` - 外部中断
- `HAL_GPIO_MODULE_ENABLED` - GPIO 功能
- `HAL_PWR_MODULE_ENABLED` - 电源管理
- `HAL_RCC_MODULE_ENABLED` - 时钟配置
- `HAL_CAN_MODULE_ENABLED` - CAN 通信（用于升级）
- `HAL_IWDG_MODULE_ENABLED` - 独立看门狗

## 已复制的源文件

以下 HAL 源文件已复制到 `Bootloader/Drivers/STM32F1xx_HAL_Driver/Src/`：

- `stm32f1xx_hal.c` - HAL 核心
- `stm32f1xx_hal_rcc.c` - 时钟配置
- `stm32f1xx_hal_rcc_ex.c` - 时钟扩展
- `stm32f1xx_hal_gpio.c` - GPIO
- `stm32f1xx_hal_gpio_ex.c` - GPIO 扩展
- `stm32f1xx_hal_dma.c` - DMA
- `stm32f1xx_hal_cortex.c` - Cortex-M 核心
- `stm32f1xx_hal_pwr.c` - 电源管理
- `stm32f1xx_hal_flash.c` - Flash 操作
- `stm32f1xx_hal_flash_ex.c` - Flash 扩展
- `stm32f1xx_hal_exti.c` - 外部中断
- `stm32f1xx_hal_can.c` - CAN 通信
- `stm32f1xx_hal_iwdg.c` - 独立看门狗

## 配置文件

- `Bootloader/Core/Inc/stm32f1xx_hal_conf.h` - HAL 配置（已启用所需模块）

## 优势

1. **独立性**：Bootloader 可以独立编译，不依赖主工程
2. **最小化**：只包含需要的 HAL 模块，减少代码大小
3. **灵活性**：可以为 Bootloader 单独配置 HAL 参数

## 注意事项

- HAL 库头文件已全部复制（头文件很小，不影响大小）
- 源文件只复制了需要的模块（减少编译时间）
- 如果需要添加新的 HAL 模块，需要：
  1. 在 `stm32f1xx_hal_conf.h` 中启用模块
  2. 复制对应的源文件到 `Src/` 目录

