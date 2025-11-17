# BSP (Board Support Package) 层

## 目录结构

```
BSP/
├── Inc/
│   ├── bsp.h          # BSP总入口头文件
│   ├── bsp_can.h      # CAN接口
│   └── bsp_gpio.h     # GPIO接口（LED等）
└── Src/
    ├── bsp.c          # BSP初始化实现
    ├── bsp_can.c      # CAN实现
    └── bsp_gpio.c     # GPIO实现
```

## 功能说明

### BSP层的作用
- 封装HAL库的硬件操作
- 提供统一的硬件接口给应用层
- 隐藏硬件细节（引脚、配置等）
- 便于移植和维护

### 架构说明（方案B - 推荐）
- **CubeMX生成的初始化代码保留在main.c中**（`MX_CAN_Init()`, `MX_GPIO_Init()`等）
- **BSP层不重复初始化**，只封装操作接口
- **BSP使用main.c中的句柄**（通过extern引用）
- **优点**：CubeMX重新生成代码时不会丢失BSP代码，配置自动同步

### 使用方式

在`main.c`中：
```c
#include "bsp.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    MX_GPIO_Init();  // CubeMX生成的初始化
    MX_CAN_Init();   // CubeMX生成的初始化
    
    /* USER CODE BEGIN 2 */
    BSP_Init();      // BSP只做封装，不重复初始化
    App_Init();
    /* USER CODE END 2 */
    
    while(1) { App_Process(); }
}
```

### 模块说明

#### CAN模块 (`bsp_can.h/c`)
- `BSP_CAN_Init()` - 启动CAN（CAN已在main.c中由MX_CAN_Init()初始化）
- `BSP_CAN_Start()` - 启动CAN（与BSP_CAN_Init()相同）
- `BSP_CAN_Stop()` - 停止CAN
- `BSP_CAN_Send()` - 发送CAN消息
- `BSP_CAN_Receive()` - 接收CAN消息
- `BSP_CAN_GetHandle()` - 获取CAN句柄（引用main.c中的hcan）

#### GPIO模块 (`bsp_gpio.h/c`)
- `BSP_GPIO_Init()` - 空函数（GPIO已在main.c中由MX_GPIO_Init()初始化，保留用于兼容性）
- `BSP_LED_Init()` - 初始化LED（独立初始化LED，如果只需要LED）
- `BSP_LED_On()` - LED开
- `BSP_LED_Off()` - LED关
- `BSP_LED_Toggle()` - LED翻转

## 注意事项

- **MSP回调函数**（`HAL_CAN_MspInit`等）保留在`Core/Src/stm32f1xx_hal_msp.c`中
- **硬件句柄**（如`CAN_HandleTypeDef hcan`）在`main.c`中定义，BSP通过`extern`引用
- **初始化顺序**：`MX_xxx_Init()` → `BSP_Init()` → `App_Init()`
- **BSP不重复初始化**：`BSP_CAN_Init()`只启动CAN，不重新配置
- 如需添加新的外设模块，按照现有模式创建`bsp_xxx.h/c`文件，使用`extern`引用main.c中的句柄

