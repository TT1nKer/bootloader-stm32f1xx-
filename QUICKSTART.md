# Bootloader 快速开始指南

## 🚀 5 步快速编译

```bash
# 1. 进入项目目录
cd ~/bootloaderfolder

# 2. 创建构建目录
mkdir -p build && cd build

# 3. 配置 CMake
cmake ..

# 4. 编译
make bootloader

# 5. 检查大小（必须 < 8KB）
make check_size
```

## ✅ 验证编译成功

编译成功后，在 `build/` 目录下会生成：
- `bootloader.elf` - ELF 格式
- `bootloader.bin` - 二进制格式
- `bootloader.hex` - HEX 格式（用于烧录）

## 📏 检查大小

```bash
# 方法1：使用 CMake 目标
make check_size

# 方法2：使用脚本
../check_bootloader_size.sh

# 方法3：手动检查
arm-none-eabi-size bootloader.elf
```

**标准**：`text + data < 8192` 字节（8KB）
**当前使用**：约 4KB（50% 占用率）

## 🔥 烧录

### Windows (推荐)
```cmd
# 返回项目根目录
cd ..

# 使用脚本烧录
flash_bootloader.bat
```

### Linux/macOS
```bash
# 使用 st-flash
st-flash write build/bootloader.bin 0x08000000

# 或使用 OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
    -c "program build/bootloader.elf verify reset exit"
```

### 验证烧录成功
- LED (PC13) 应该以 **1Hz** 频率闪烁
- 可以随时使用 STM32CubeProgrammer 连接和调试

## OTA 测试入口

1. 在上位机/应用侧包含 `ota_transport.h`。
2. 发送 `OTA_PACKET_CONTROL`：
   - `total_size`: 固件总字节数（≤ 24KB）
   - `crc32`: 预期 CRC32（可为 0 表示跳过）
   - `signature` + `signature_length`: 可选签名，占位长度 64 字节
3. 循环发送 `OTA_PACKET_DATA`：
   - `sequence`: 从 0 递增
   - `offset`: `sequence * OTA_MAX_CHUNK_SIZE`
   - `length`: 本次分片长度（≤ 256 字节）
4. 等待 bootloader 串口/BLE/RF 回传完成事件，或在下次复位时观察自动切换 Bank。

> `OTA_MAX_CHUNK_SIZE = 256`，`OTA_SIGNATURE_MAX_BYTES = 64`。可根据实际传输层做二次封装。

## 📋 前置要求

- **CMake** >= 3.22
- **ARM GCC** (arm-none-eabi-gcc)

安装：
```bash
# macOS
brew install cmake arm-none-eabi-gcc

# Linux
sudo apt-get install cmake gcc-arm-none-eabi
```

## 📚 更多信息

- 详细编译指南：`BUILD.md`
- 项目说明：`README.md`
- 独立性说明：`STANDALONE_INFO.md`

