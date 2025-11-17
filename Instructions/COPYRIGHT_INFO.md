# 版权信息说明

## Bootloader 版权信息

所有 Bootloader 相关文件已添加以下版权信息：

```
Copyright (c) 2025 TT1nker (GitHub: TT1nker)
All rights reserved.

Contact: hostsjim22@gmail.com
```

## 版权信息包含的内容

### 必需项
1. **版权年份**：`Copyright (c) 2025`
   - 通常是首次发布的年份
   - 如果后续修改，可以写范围：`Copyright (c) 2025-2026`

2. **版权所有者**：`TT1nker (GitHub: TT1nker)`
   - 可以是个人姓名、GitHub 用户名、公司名等
   - 建议包含 GitHub 用户名便于识别

3. **版权声明**：`All rights reserved.`
   - 表示保留所有权利

### 可选项（但建议包含）
4. **联系方式**：`Contact: hostsjim22@gmail.com`
   - 便于他人联系
   - 邮箱不是必需的，但有助于沟通

5. **许可证声明**：
   - `This software component is provided AS-IS, without any warranty of any kind.`
   - 表示软件按"现状"提供，不提供任何保证
   - 如果需要开源，可以添加许可证（如 MIT、Apache 2.0 等）

## 已更新的文件

以下 Bootloader 文件已添加版权信息：

### Core 文件
- `Bootloader/Core/Src/bootloader_main.c`
- `Bootloader/Core/Inc/main.h`
- `Bootloader/Core/Inc/bootloader_config.h`

### BSP 文件
- `Bootloader/BSP/Inc/bsp_flash.h`
- `Bootloader/BSP/Src/bsp_flash.c`
- `Bootloader/BSP/Inc/bsp_jump.h`
- `Bootloader/BSP/Src/bsp_jump.c`
- `Bootloader/BSP/Inc/bsp_upgrade_flag.h`
- `Bootloader/BSP/Src/bsp_upgrade_flag.c`
- `Bootloader/BSP/Inc/bsp_watchdog.h`
- `Bootloader/BSP/Src/bsp_watchdog.c`

## 版权信息格式

标准格式：
```c
/**
  ******************************************************************************
  * @file    filename.c
  * @brief   File description
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 TT1nker (GitHub: TT1nker)
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  * This software component is provided AS-IS, without any warranty of any kind.
  * User is responsible for the proper use of this software.
  *
  ******************************************************************************
  */
```

## 关于邮箱

**邮箱不是版权信息的必需项**，但建议包含，因为：
- ✅ 便于他人联系你（报告 bug、提问等）
- ✅ 显示代码维护者的联系方式
- ✅ 增加代码的专业性

如果不希望公开邮箱，可以：
- 只保留 GitHub 用户名
- 使用 GitHub 的邮箱（通常是 noreply 邮箱）
- 或者完全省略联系方式

## 许可证选项

如果需要开源，常见的许可证：

### MIT License（推荐）
```
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
```

### Apache 2.0
```
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License...
```

### 专有软件（当前使用）
```
All rights reserved.
This software component is provided AS-IS, without any warranty of any kind.
```

## 注意事项

1. **版权年份**：建议使用首次发布的年份，如果后续修改可以更新
2. **版权所有者**：可以是个人、团队或公司
3. **许可证**：如果不指定许可证，默认是专有软件（All rights reserved）
4. **一致性**：同一项目的所有文件应该使用相同的版权信息格式

## 当前配置总结

- ✅ 版权年份：2025
- ✅ 版权所有者：TT1nker (GitHub: TT1nker)
- ✅ 联系方式：hostsjim22@gmail.com
- ✅ 许可证：专有软件（AS-IS，无保证）
- ✅ 所有 Bootloader 文件已更新

版权信息已完整添加！

