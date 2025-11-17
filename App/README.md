# App (Application) 层

## 目录结构

```
App/
├── Inc/
│   └── app.h          # 应用层头文件
└── Src/
    └── app.c          # 应用层实现
```

## 功能说明

### App层的作用
- 实现业务逻辑
- 调用BSP接口访问硬件
- 不直接操作HAL库
- 便于测试和维护

### 使用方式

在`main.c`中：
```c
#include "app.h"

int main(void)
{
    // ... 系统初始化 ...
    BSP_Init();
    
    App_Init();  // 应用初始化
    
    while(1)
    {
        App_Process();  // 应用主循环
    }
}
```

### 接口说明

- `App_Init()` - 应用初始化（在main中调用一次）
- `App_Process()` - 应用主循环处理（在main的while循环中调用）

## 开发建议

1. **业务逻辑放在App层**
   - 温度采集逻辑
   - CAN通信协议处理
   - 数据处理和算法

2. **硬件操作通过BSP**
   - 使用`BSP_CAN_Send()`而不是`HAL_CAN_AddTxMessage()`
   - 使用`BSP_LED_Toggle()`而不是`HAL_GPIO_TogglePin()`

3. **保持分层清晰**
   - App层不包含HAL相关代码
   - App层不直接操作硬件寄存器

