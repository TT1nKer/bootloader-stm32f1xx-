# STM32F103 Bootloader

STM32F103 微控制器的极简引导程序，Flash 限制 8KB。

## 功能特性

- ✅ 双 Bank 布局（24KB 运行区 + 24KB OTA 缓存）
- ✅ OTA 元数据（版本、CRC32、状态机、错误码）
- ✅ BLE/RF 传输桩：分片写入 + 序号校验 + 重传钩子
- ✅ Flash 帮助函数：Bank 级擦写、CRC32 流式计算
- ✅ 3 秒看门狗保护 + PC13 LED 指示
- ✅ SWD 调试保留（禁用 JTAG 释放 PA15/PB3/PB4）

## 内存布局

```
0x08000000 - 0x08001FFF (  8KB) : Bootloader（本项目）
0x08002000 - 0x08007FFF ( 24KB) : 应用 Bank A（默认活动）
0x08008000 - 0x0800DFFF ( 24KB) : 应用 Bank B（OTA 缓存/回退）
0x0800E000 - 0x0800EFFF (  4KB) : OTA 元数据（版本、CRC、状态）
0x0800F000 - 0x0800FFFF (  4KB) : 设备配置/保留
```

**Bootloader 占用**: 4012 字节 / 8192 字节 (48.97%)

## OTA 工作流

1. 应用设置升级标志（或检测到固件无效）并复位。
2. Bootloader 选择非活动 Bank，执行整 Bank 擦除，并把元数据状态置为 `DOWNLOADING`。
3. BLE 或 RF 传输先发送 1 个 `OTA_PACKET_CONTROL`（大小、版本、CRC、签名），随后发送多个 `OTA_PACKET_DATA` 分片（≤256 字节），序号严格递增。
4. `ota_download` 通过 `BSP_Flash_WriteChunk` 写入分片，持续累加 CRC32，并记录进度。
5. 当所有字节写入完成后，执行 CRC + 签名校验，元数据更新为 `READY`。
6. 下一次上电时，Bootloader 校验缓存 Bank，若通过则自动切换并跳转，若失败立即回滚。

## 快速开始

### 1. 编译

```bash
# Windows
mkdir build
cd build
cmake ..
ninja bootloader
```

### 2. 烧录

**使用脚本（推荐）**:
```cmd
flash_bootloader.bat
```

**手动使用 STM32CubeProgrammer**:
1. 打开 STM32CubeProgrammer
2. 通过 ST-LINK (SWD) 连接
3. 加载 `build/bootloader.hex`
4. 烧录到地址 `0x08000000`

### 3. 验证

- LED (PC13) 以 1Hz 频率闪烁 → Bootloader 正常运行
- 可随时重新连接调试器 → SWD 已启用

## 传输集成（BLE / RF）

`ota_transport.h` 定义了统一的数据包：

```c
typedef struct {
    OtaTransportType_t transport;   // BLE 或自研 RF
    OtaPacketType_t    type;        // CONTROL / DATA / ABORT
    uint32_t sequence;              // DATA 分片的序号
    uint32_t offset;                // 分片在固件中的偏移
    uint32_t length;                // 分片长度（≤256 字节）
    uint32_t total_size;            // CONTROL 包携带的固件总长度
    uint32_t version;               // 固件版本号 / 时间戳
    uint32_t crc32;                 // 预期 CRC32（CONTROL 包）
    uint32_t signature_length;      // 可选签名长度（≤64 字节）
    uint8_t  signature[64];         // 可选签名
    uint8_t  payload[256];          // DATA 分片数据
} OtaTransportPacket_t;
```

BLE 案例：

```c
OtaTransportPacket_t ctrl = {
    .type = OTA_PACKET_CONTROL,
    .total_size = image_size,
    .version = 0x20250101,
    .crc32 = fw_crc,
    .signature_length = sig_len,
};
memcpy(ctrl.signature, sig_data, ctrl.signature_length);
OTA_TransportBle_Submit(&ctrl);

for (uint32_t seq = 0; seq < num_chunks; ++seq) {
    OtaTransportPacket_t chunk = {
        .type = OTA_PACKET_DATA,
        .sequence = seq,
        .offset = seq * OTA_MAX_CHUNK_SIZE,
    };
    uint32_t remaining = image_size - chunk.offset;
    chunk.length = (remaining > OTA_MAX_CHUNK_SIZE) ? OTA_MAX_CHUNK_SIZE : remaining;
    memcpy(chunk.payload, image_ptr + chunk.offset, chunk.length);
    OTA_TransportBle_Submit(&chunk);
}
```

RF 通道使用同一套接口（`OTA_TransportRf_*`），并共享 `OTA_DownloadService`，因此可以在传输侧实现重传/限速/加密策略，而无需修改 bootloader 核心。

## 硬件连接

```
ST-Link    →    STM32F103
SWDIO      →    PA13
SWCLK      →    PA14
GND        →    GND
3.3V       →    VCC（如板子有独立供电则可选）
```

## 环境要求

- **CMake** >= 3.22
- **ARM GCC 工具链** (arm-none-eabi-gcc)
- **Ninja** 构建系统
- **STM32CubeProgrammer** 用于烧录

## 重要说明

### ⚠️ SWD 调试接口

本 bootloader **保持 SWD 启用**以便调试，同时**禁用 JTAG** 以释放引脚：
- ✅ **PA13, PA14** (SWD) - 用于调试/烧录
- ✅ **PA15, PB3, PB4** (JTAG) - 释放用于 GPIO

**为什么？** 早期版本使用 `__HAL_AFIO_REMAP_SWJ_DISABLE()` 完全禁用了调试接口。已修复为 `__HAL_AFIO_REMAP_SWJ_NOJTAG()`。

### OTA 元数据状态

| 状态           | 说明                                      |
|----------------|-------------------------------------------|
| `IDLE`         | 无升级任务，直接跳转活动 Bank             |
| `DOWNLOADING`  | 已擦除缓存区，等待 BLE/RF 分片             |
| `READY`        | 下载完成且 CRC/签名通过，等待复位          |
| `APPLYING`     | 新 Bank 已提升为活动 Bank，等待应用确认     |
| `ROLLBACK`     | 校验失败，Bootloader 已回滚至旧固件        |

元数据固定放在 `0x0800E000`（4KB），包含版本、CRC、下载大小以及最后一次错误代码，便于问题追溯。

### 应用程序开发

您的应用固件需要：
1. **根据 Bank 链接**  
   - Bank A（默认运行）：`ORIGIN = 0x08002000`，`LENGTH = 24K`  
   - Bank B（OTA 缓存）：`ORIGIN = 0x08008000`，`LENGTH = 24K`
2. 在 `SystemInit` 或初始化阶段设置 `SCB->VTOR = BANK_START`
3. 严格控制大小 **≤ 24KB**（确保双 Bank OTA 可行）

Bank A 示例链接脚本：
```ld
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08002000, LENGTH = 24K
  RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 20K
}
```

## 故障排除

### 烧录后无法连接调试器

**解决方案**: 已修复！请确保使用最新版本，SWD 接口已正确启用。

### LED 不闪烁

1. 检查 bootloader 是否烧录到 `0x08000000`
2. 检查 LED 连接（PC13）
3. 检查电源供电（3.3V）

### 看门狗复位循环

如果 LED 每 3 秒快速闪烁一次，说明 bootloader 卡在 `Error_Handler()` 中。通常原因：
- `0x08002000` 处的应用程序无效
- Flash 损坏

**修复**: 擦除应用区域并确保烧录有效固件。

## 项目结构

```
.
├── Core/
│   ├── Inc/           # 主头文件和配置
│   └── Src/           # 主 bootloader 代码
├── BSP/               # 板级支持包
│   ├── Inc/           # Flash/看门狗/OTA 元数据/传输接口
│   └── Src/           # bsp_flash/jump/watchdog + ota_download/security/transports
├── Drivers/           # STM32 HAL 和 CMSIS
├── build/             # 编译输出（自动生成）
├── CMakeLists.txt     # 构建配置
├── flash_bootloader.bat      # 烧录脚本
├── test_connection.bat       # 测试 ST-Link 连接
└── check_bootloader_size.sh  # 大小检查脚本
```

## 版本历史

### v1.2.0 (2025-02-XX)
- ✅ 双 Bank 布局 + 元数据页
- ✅ OTA 下载服务 + BLE/RF 传输桩
- ✅ CRC32 流式校验 + 签名校验占位
- ✅ Flash Bank 擦写/写入保护
- ✅ Bootloader 自动晋升 + 失败回滚

### v1.1.0 (2025-01-XX)
- 看门狗与升级标志安全性增强
- 升级标志移至配置页，正常启动不触发擦写

### v1.0.0 (2025-01-XX)
- ✅ 核心 bootloader 功能
- ✅ 修复 SWD 调试接口（关键 bug 修复）
- ✅ 优化至 4KB（50% Flash 使用率）
- ✅ 看门狗保护
- ✅ 应用程序验证和跳转

## 许可证

Copyright (c) 2025 TT1nker  
GitHub: [TT1nker](https://github.com/TT1nker)  
联系方式: hostsjim22@gmail.com

本软件按"原样"提供，不提供任何形式的担保。

## 贡献

欢迎贡献！请确保：
- 代码编译无警告
- Bootloader 大小保持在 8KB 以下
- SWD 调试功能正常
