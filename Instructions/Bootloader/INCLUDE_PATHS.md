# Bootloader Include Paths Configuration

## 问题说明

Bootloader 需要访问主工程的 HAL 库和配置文件。需要配置正确的 include 路径。

## 解决方案

### 方案1：在编译器中配置 Include 路径（推荐）

在 CMakeLists.txt 或 IDE 配置中添加以下 include 路径：

```
Include Paths:
- Bootloader/Core/Inc
- Bootloader/BSP/Inc
- Core/Inc                    # 主工程的 HAL 配置
- Drivers/STM32F1xx_HAL_Driver/Inc
- Drivers/CMSIS/Device/ST/STM32F1xx/Include
- Drivers/CMSIS/Core/Include
```

### 方案2：修改 main.h 使用相对路径

如果使用方案1，`main.h` 中的 include 应该改为：

```c
#include "stm32f1xx_hal.h"  // 通过 include 路径找到
```

如果使用方案2（相对路径），`main.h` 中应该使用：

```c
#include "../Core/Inc/stm32f1xx_hal.h"  // 相对路径
```

### 方案3：创建符号链接（适用于 Unix/Linux/macOS）

```bash
cd Bootloader/Core/Inc
ln -s ../../Core/Inc/stm32f1xx_hal_conf.h
ln -s ../../Core/Inc/stm32f1xx_it.h
```

## 当前配置

当前 `main.h` 使用相对路径 `../Core/Inc/stm32f1xx_hal.h`，这要求：
1. Bootloader 的编译工作目录在 `Temperature/` 根目录
2. 或者配置 include 路径包含 `../Core/Inc`

## 推荐配置

**在 CMakeLists.txt 中：**

```cmake
# Bootloader include paths
target_include_directories(bootloader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/BSP/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Core/Include
)
```

然后在 `main.h` 中使用：

```c
#include "stm32f1xx_hal.h"  // 通过 include 路径找到
```

