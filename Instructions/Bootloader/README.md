# Bootloader 实现文档

## 目录结构

```
Bootloader/
├── Core/
│   ├── Inc/
│   │   └── bootloader_config.h    # Bootloader配置（内存布局等）
│   └── Src/
│       └── bootloader_main.c       # Bootloader主程序
├── BSP/
│   ├── Inc/
│   │   ├── bsp_flash.h            # Flash操作接口
│   │   ├── bsp_jump.h             # 应用程序跳转接口
│   │   ├── bsp_upgrade_flag.h     # 升级标志管理接口
│   │   └── bsp_watchdog.h         # 看门狗接口
│   └── Src/
│       ├── bsp_flash.c            # Flash操作实现
│       ├── bsp_jump.c             # 应用程序跳转实现
│       ├── bsp_upgrade_flag.c     # 升级标志管理实现
│       └── bsp_watchdog.c         # 看门狗实现
└── README.md
```

## 内存布局

```
STM32F103C8T6 (64KB Flash)

┌─────────────────────────────────────┐
│ 0x08000000 - 0x08000FFF (4KB)       │  Bootloader区域
│ 0x08001000 - 0x08003FFF (12KB)      │  应用程序区域
│ 0x08004000 - 0x08006FFF (12KB)      │  备份应用程序区域（可选）
│ 0x08007000 - 0x08007FFF (4KB)       │  配置数据区
│ 0x08008000 - 0x0800FFFF (32KB)      │  数据存储区（可选）
└─────────────────────────────────────┘
```

## 功能模块

### 1. Flash操作模块 (`bsp_flash.h/c`)

提供Flash的读写、擦除和验证功能：

- `BSP_Flash_Erase()` - 擦除Flash页
- `BSP_Flash_Write()` - 写入Flash（按半字）
- `BSP_Flash_Read()` - 读取Flash
- `BSP_Flash_Verify()` - 验证Flash内容
- `BSP_Flash_ReadWord()` - 读取32位字

### 2. 应用程序跳转模块 (`bsp_jump.h/c`)

实现从Bootloader跳转到应用程序：

- `BSP_Jump_IsApplicationValid()` - 检查应用程序是否有效
- `BSP_Jump_ToApplication()` - 跳转到应用程序

**跳转流程：**
1. 关闭所有中断
2. 重置SysTick
3. 设置向量表偏移
4. 设置堆栈指针
5. 跳转到应用程序入口

### 3. 升级标志管理模块 (`bsp_upgrade_flag.h/c`)

管理升级标志，决定是否进入升级模式：

- `BSP_UpgradeFlag_Get()` - 获取升级标志
- `BSP_UpgradeFlag_Set()` - 设置升级标志
- `BSP_UpgradeFlag_Clear()` - 清除升级标志

**升级模式：**
- `UPGRADE_MODE_NONE` - 正常启动
- `UPGRADE_MODE_CAN` - CAN升级
- `UPGRADE_MODE_BLE` - 蓝牙升级
- `UPGRADE_MODE_RF` - 射频升级

### 4. 看门狗模块 (`bsp_watchdog.h/c`)

提供看门狗保护功能：

- `BSP_Watchdog_Init()` - 初始化看门狗（默认3秒超时）
- `BSP_Watchdog_Feed()` - 喂狗

## Bootloader启动流程

```
1. 系统初始化
   ├─ HAL_Init()
   ├─ SystemClock_Config()
   └─ MX_GPIO_Init()

2. 初始化看门狗（3秒超时）

3. 检查升级标志
   ├─ 如果 UPGRADE_MODE_NONE
   │   ├─ 检查应用程序有效性
   │   └─ 如果有效，跳转到应用程序
   │   └─ 如果无效，进入错误处理
   │
   └─ 如果升级模式
       ├─ 清除升级标志
       └─ 进入升级模式循环
           ├─ 处理升级消息
           └─ 定期喂狗（每500ms）
```

## 使用说明

### 在应用程序中触发升级

```c
// 在应用程序中设置升级标志
#include "bootloader_config.h"
#include "bsp_upgrade_flag.h"

void TriggerUpgrade(UpgradeMode_t mode)
{
    // 设置升级标志
    BSP_UpgradeFlag_Set(mode);
    
    // 软件复位
    NVIC_SystemReset();
}
```

### 应用程序配置

应用程序需要修改以下配置：

1. **链接脚本**：应用程序起始地址改为 `0x08001000`
2. **中断向量表**：在 `SystemInit()` 中设置 `SCB->VTOR = 0x08001000`

## 下一步实现

1. ✅ Bootloader基础框架
2. ⏳ CAN升级协议实现
3. ⏳ 蓝牙OTA升级实现
4. ⏳ 射频OTA升级实现
5. ⏳ 固件校验（CRC32）
6. ⏳ 升级失败回滚机制

## 注意事项

1. **Flash写入限制**：STM32F103 Flash可擦写约10,000次
2. **看门狗超时**：建议设置为2-5秒，在长时间操作中要定期喂狗
3. **应用程序验证**：跳转前必须验证应用程序有效性
4. **升级标志**：使用魔数（0xDEADBEEF）确保标志有效性



