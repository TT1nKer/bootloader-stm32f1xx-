# 如何确保 Bootloader 在 4KB 以内

## 1. 链接脚本限制

### 当前配置
```ld
/* Bootloader/STM32F103XX_BOOTLOADER.ld */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 4K  /* 限制为 4KB */
}
```

**关键点：**
- `LENGTH = 4K` 告诉链接器 Flash 只有 4KB
- 如果代码超过 4KB，链接器会报错：`region 'FLASH' overflowed`

## 2. 编译后检查大小

### 方法1：使用 size 命令（推荐）

```bash
# 编译 Bootloader
make bootloader

# 查看大小
arm-none-eabi-size build/bootloader.elf

# 输出示例：
#    text    data     bss     dec     hex filename
#    3456     128      64    3648     e40 bootloader.elf
```

**解读：**
- `text`：代码段大小（Flash 中的代码）
- `data`：已初始化数据（Flash 中的初始值 + RAM 中的数据）
- `bss`：未初始化数据（只在 RAM 中）
- **关键看 `text` + `data`**：这是 Flash 中实际占用的空间

**判断标准：**
- `text + data` < 4096 字节（4KB）✅
- `text + data` >= 4096 字节（4KB）❌ 需要优化

### 方法2：查看 map 文件

```bash
# 编译后查看 map 文件
cat build/bootloader.map | grep "Memory Configuration"

# 或者查看 Flash 使用情况
cat build/bootloader.map | grep "FLASH"
```

### 方法3：使用 objdump

```bash
# 查看各段大小
arm-none-eabi-objdump -h build/bootloader.elf

# 输出示例：
# Sections:
# Idx Name          Size      VMA       LMA       File off  Algn
#   0 .isr_vector   00000100  08000000  08000000  00001000  2**0
#   1 .text         00000d00  08000100  08000100  00001100  2**2
#   2 .rodata       00000100  08000e00  08000e00  00001e00  2**2
#   3 .data         00000080  20000000  08000f00  00001f00  2**2
```

**计算 Flash 使用：**
- `.isr_vector` + `.text` + `.rodata` + `.data` 的 LMA（加载地址）部分
- 应该 < 0x1000 (4096 字节)

### 方法4：使用 readelf

```bash
arm-none-eabi-readelf -l build/bootloader.elf

# 查看 Program Headers，找到 LOAD 段
# 检查在 Flash 中的段大小
```

## 3. 链接脚本中的保护机制

### 添加大小检查（可选）

在链接脚本末尾添加：

```ld
/* 检查是否超过 4KB */
ASSERT((_etext - 0x08000000) <= 4096, "Bootloader exceeds 4KB limit!")
```

如果超过 4KB，链接会失败并显示错误信息。

## 4. 优化策略（如果超过 4KB）

### 4.1 编译器优化

```cmake
# 在 CMakeLists.txt 中
target_compile_options(bootloader PRIVATE
    -Os              # 优化代码大小（而不是速度）
    -ffunction-sections
    -fdata-sections
)

target_link_options(bootloader PRIVATE
    -Wl,--gc-sections    # 移除未使用的代码段
    -Wl,--print-memory-usage
)
```

### 4.2 移除不必要的功能

- 移除调试代码（printf、assert 等）
- 移除未使用的 HAL 模块
- 简化错误处理（如果不需要详细错误信息）

### 4.3 使用内联函数

```c
// 小函数使用 static inline
static inline void FeedWatchdog(void) {
    HAL_IWDG_Refresh(&hiwdg);
}
```

### 4.4 优化字符串

```c
// 避免使用长字符串
// 使用短字符串或移除字符串常量
```

### 4.5 减少 Flash 中的常量数据

```c
// 将常量数据移到 RAM（如果可能）
// 或者使用查找表而不是大数组
```

## 5. 实际检查脚本

创建检查脚本：

```bash
#!/bin/bash
# check_bootloader_size.sh

BOOTLOADER_ELF="build/bootloader.elf"
MAX_SIZE=4096

if [ ! -f "$BOOTLOADER_ELF" ]; then
    echo "Error: $BOOTLOADER_ELF not found. Build bootloader first."
    exit 1
fi

# 获取大小信息
SIZE_INFO=$(arm-none-eabi-size $BOOTLOADER_ELF | tail -1)
TEXT=$(echo $SIZE_INFO | awk '{print $1}')
DATA=$(echo $SIZE_INFO | awk '{print $2}')
TOTAL_FLASH=$((TEXT + DATA))

echo "Bootloader Size Analysis:"
echo "  Text (code):     $TEXT bytes"
echo "  Data (init):     $DATA bytes"
echo "  Total Flash:     $TOTAL_FLASH bytes / $MAX_SIZE bytes"
echo "  Usage:           $((TOTAL_FLASH * 100 / MAX_SIZE))%"

if [ $TOTAL_FLASH -gt $MAX_SIZE ]; then
    echo ""
    echo "❌ ERROR: Bootloader exceeds 4KB limit!"
    echo "   Exceeded by: $((TOTAL_FLASH - MAX_SIZE)) bytes"
    exit 1
else
    echo ""
    echo "✅ OK: Bootloader is within 4KB limit"
    echo "   Remaining: $((MAX_SIZE - TOTAL_FLASH)) bytes"
    exit 0
fi
```

使用方法：
```bash
chmod +x check_bootloader_size.sh
./check_bootloader_size.sh
```

## 6. CMake 集成检查

在 CMakeLists.txt 中添加：

```cmake
# 编译后自动检查大小
add_custom_command(TARGET bootloader POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:bootloader>
    COMMENT "Checking bootloader size..."
)

# 或者添加自定义目标
add_custom_target(check_bootloader_size
    COMMAND ${CMAKE_COMMAND} -E echo "Bootloader size:"
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:bootloader>
    DEPENDS bootloader
)
```

## 7. 典型 Bootloader 大小参考

| 功能 | 典型大小 |
|------|---------|
| 基础 Bootloader（跳转功能） | ~1-2KB |
| + Flash 操作 | +0.5-1KB |
| + CAN 升级协议 | +1-2KB |
| + 看门狗 | +0.2KB |
| + 固件校验（CRC32） | +0.5KB |
| **总计** | **~3-6KB** |

**建议：**
- 基础功能保持在 2-3KB
- 为未来功能预留 1KB 空间
- 如果超过 4KB，考虑增加 Bootloader 区域大小（需要修改内存布局）

## 8. 如果必须超过 4KB

如果功能确实需要超过 4KB，可以：

1. **增加 Bootloader 区域大小**
   ```ld
   FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 8K  /* 改为 8KB */
   ```
   相应地调整应用程序起始地址：
   ```ld
   FLASH (rx) : ORIGIN = 0x8002000, LENGTH = 56K  /* 应用程序从 8KB 开始 */
   ```

2. **优化代码**
   - 移除不必要的功能
   - 使用更紧凑的算法
   - 减少代码重复

## 9. 快速检查命令

```bash
# 一键检查
arm-none-eabi-size build/bootloader.elf | tail -1 | awk '{if ($1+$2 > 4096) print "❌ Exceeds 4KB"; else print "✅ Within 4KB"}'
```

## 总结

**确保 Bootloader < 4KB 的方法：**

1. ✅ **链接脚本限制**：`LENGTH = 4K`（链接器会自动检查）
2. ✅ **编译后检查**：使用 `arm-none-eabi-size` 查看实际大小
3. ✅ **编译器优化**：使用 `-Os` 和 `--gc-sections`
4. ✅ **代码优化**：移除不必要的功能
5. ✅ **自动化检查**：添加脚本或 CMake 目标自动检查

**如果超过 4KB：**
- 链接器会报错（如果链接脚本正确配置）
- 需要优化代码或增加 Bootloader 区域大小

