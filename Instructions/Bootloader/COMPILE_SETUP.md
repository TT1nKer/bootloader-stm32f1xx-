# Bootloader 编译配置说明

## Include 路径配置

Bootloader 和应用程序共享 HAL 库，需要正确配置 include 路径。

### 应用程序的 Include 路径

应用程序的 BSP 文件（`BSP/Inc/bsp.h` 等）需要访问 `Core/Inc/main.h`：

```
Include Paths for Application:
- Core/Inc                          # 包含 main.h, stm32f1xx_hal_conf.h
- BSP/Inc                           # BSP 头文件
- App/Inc                           # 应用层头文件
- Drivers/STM32F1xx_HAL_Driver/Inc  # HAL 驱动
- Drivers/CMSIS/Device/ST/STM32F1xx/Include
- Drivers/CMSIS/Core/Include
```

### Bootloader 的 Include 路径

Bootloader 需要访问主工程的 HAL 库：

```
Include Paths for Bootloader:
- Bootloader/Core/Inc               # Bootloader 配置
- Bootloader/BSP/Inc                # Bootloader BSP 头文件
- Core/Inc                          # 主工程的 HAL 配置（共享）
- Drivers/STM32F1xx_HAL_Driver/Inc  # HAL 驱动（共享）
- Drivers/CMSIS/Device/ST/STM32F1xx/Include
- Drivers/CMSIS/Core/Include
```

## 文件结构说明

```
Temperature/
├── Core/
│   └── Inc/
│       ├── main.h                  # 应用程序的 main.h
│       └── stm32f1xx_hal_conf.h
├── BSP/
│   └── Inc/
│       ├── bsp.h                   # 包含 "main.h" → 找到 Core/Inc/main.h
│       └── bsp_can.h
├── Bootloader/
│   └── Core/
│       └── Inc/
│           └── main.h              # Bootloader 的 main.h（不同文件）
└── Drivers/
    └── STM32F1xx_HAL_Driver/
        └── Inc/
            └── stm32f1xx_hal.h
```

## IDE 配置（VS Code / Cursor）

在 `.vscode/c_cpp_properties.json` 中配置：

```json
{
    "configurations": [
        {
            "name": "STM32",
            "includePath": [
                "${workspaceFolder}/Core/Inc",
                "${workspaceFolder}/BSP/Inc",
                "${workspaceFolder}/App/Inc",
                "${workspaceFolder}/Bootloader/Core/Inc",
                "${workspaceFolder}/Bootloader/BSP/Inc",
                "${workspaceFolder}/Drivers/STM32F1xx_HAL_Driver/Inc",
                "${workspaceFolder}/Drivers/CMSIS/Device/ST/STM32F1xx/Include",
                "${workspaceFolder}/Drivers/CMSIS/Core/Include"
            ],
            "defines": [
                "USE_HAL_DRIVER",
                "STM32F103xB"
            ],
            "compilerPath": "/usr/bin/arm-none-eabi-gcc",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "gcc-arm"
        }
    ],
    "version": 4
}
```

## CMake 配置示例

```cmake
# 应用程序的 include 路径
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/BSP/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Core/Include
)

# Bootloader 的 include 路径
target_include_directories(bootloader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootloader/BSP/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc                    # 共享 HAL 配置
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Core/Include
)
```

## 解决 IDE 错误提示

如果 IDE 仍然报错 `'main.h' file not found`：

1. **检查 include 路径配置**：确保 `Core/Inc` 在 include 路径中
2. **重启 IDE**：有时需要重启 IDE 才能识别新的配置
3. **清理缓存**：删除 `.vscode/.browse.vc.db` 等缓存文件
4. **使用相对路径**：如果问题持续，可以在 BSP 文件中使用 `#include "../Core/Inc/main.h"`

## 注意事项

1. **两个不同的 main.h**：
   - `Core/Inc/main.h` - 应用程序的 main.h
   - `Bootloader/Core/Inc/main.h` - Bootloader 的 main.h
   - 它们通过 include 路径区分

2. **共享 HAL 库**：
   - Bootloader 和应用程序共享同一个 HAL 库
   - 都使用 `Core/Inc/stm32f1xx_hal_conf.h` 配置

3. **编译顺序**：
   - 可以先编译应用程序，确保 HAL 库正常
   - 再编译 Bootloader，它会引用相同的 HAL 库

