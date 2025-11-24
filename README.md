# STM32F103 Bootloader

Minimalist bootloader for STM32F103 microcontroller with 8KB flash constraint.

## Features

- ✅ Dual-bank layout (24KB active + 24KB OTA staging)
- ✅ OTA metadata (version, CRC32, state machine, last error)
- ✅ BLE/RF transport stubs with chunked writes + sequence guard
- ✅ Flash helpers with per-bank erase/write + CRC32 streaming
- ✅ Independent watchdog protection (3s timeout) and PC13 LED
- ✅ SWD debugging support (JTAG disabled to free pins)

## Memory Layout

```
0x08000000 - 0x08001FFF (  8KB) : Bootloader (this repo)
0x08002000 - 0x08007FFF ( 24KB) : Application Bank A (active by default)
0x08008000 - 0x0800DFFF ( 24KB) : Application Bank B (OTA staging / fallback)
0x0800E000 - 0x0800EFFF (  4KB) : OTA metadata (version, CRC, state)
0x0800F000 - 0x0800FFFF (  4KB) : Device configuration / reserved
```

**Bootloader usage**: 4012 bytes / 8192 bytes (48.97%)

## OTA Workflow

1. Application sets upgrade flag (or bootloader detects invalid firmware) and resets.
2. Bootloader selects the inactive bank, erases it, and marks metadata state as `DOWNLOADING`.
3. BLE or RF transport sends one `OTA_PACKET_CONTROL` (size, version, CRC, optional signature), followed by many `OTA_PACKET_DATA` chunks (≤256 bytes) with monotonically increasing sequence IDs.
4. `ota_download` writes each chunk via `BSP_Flash_WriteChunk`, maintains a streaming CRC32, and tracks progress.
5. When all bytes arrive, CRC + signature scaffolding run. Metadata is updated to `READY` with size/version/CRC.
6. On the next reset, `bootloader_main` validates the staged bank, promotes it to active, and jumps to the new firmware. If validation fails, it rolls back automatically.

## Quick Start

### 1. Build

```bash
# Windows
mkdir build
cd build
cmake ..
ninja bootloader
```

### 2. Flash

**Using script (recommended)**:
```cmd
flash_bootloader.bat
```

**Using STM32CubeProgrammer manually**:
1. Open STM32CubeProgrammer
2. Connect via ST-LINK (SWD)
3. Load `build/bootloader.hex`
4. Flash to address `0x08000000`

### 3. Verify

- LED (PC13) blinks at 1Hz → Bootloader running normally
- Can reconnect debugger anytime → SWD enabled

## Transport Integration (BLE / RF)

Packets are defined in `ota_transport.h`:

```c
typedef struct {
    OtaTransportType_t transport;   // BLE or RF
    OtaPacketType_t    type;        // CONTROL / DATA / ABORT
    uint32_t sequence;              // Monotonic counter (DATA only)
    uint32_t offset;                // Byte offset within firmware image
    uint32_t length;                // Bytes in payload (≤ 256)
    uint32_t total_size;            // Populated in CONTROL packet
    uint32_t version;               // Semantic version or build ID
    uint32_t crc32;                 // Expected CRC32 (CONTROL packet)
    uint32_t signature_length;      // Optional signature (≤ 64 bytes)
    uint8_t  signature[64];         // Optional signature blob
    uint8_t  payload[256];          // Raw firmware bytes (DATA packet)
} OtaTransportPacket_t;
```

Typical BLE flow:

```c
OtaTransportPacket_t ctrl = {
    .type = OTA_PACKET_CONTROL,
    .total_size = image_size,
    .version = 0x20250101,
    .crc32 = firmware_crc,
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

RF integration mirrors the BLE helpers (`OTA_TransportRf_*`). Both transports share the `OTA_DownloadService`, so retransmission, throttling, and authentication policies can live on the transport side without touching the bootloader core.

## Hardware Connection

```
ST-Link    →    STM32F103
SWDIO      →    PA13
SWCLK      →    PA14
GND        →    GND
3.3V       →    VCC (optional if board has power)
```

## Requirements

- **CMake** >= 3.22
- **ARM GCC toolchain** (arm-none-eabi-gcc)
- **Ninja** build system
- **STM32CubeProgrammer** for flashing

## Important Notes

### ⚠️ SWD Debug Interface

This bootloader **keeps SWD enabled** for debugging while **disabling JTAG** to free pins:
- ✅ **PA13, PA14** (SWD) - Active for debugging/flashing
- ✅ **PA15, PB3, PB4** (JTAG) - Freed for GPIO use

**Why?** Previous versions had `__HAL_AFIO_REMAP_SWJ_DISABLE()` which completely disabled debug access. This has been fixed to `__HAL_AFIO_REMAP_SWJ_NOJTAG()`.

### OTA Metadata States

| State          | Meaning                                                            |
|----------------|--------------------------------------------------------------------|
| `IDLE`         | No upgrade pending; bootloader will jump to active bank            |
| `DOWNLOADING`  | Staging bank erased and waiting for data chunks                    |
| `READY`        | New firmware fully downloaded, CRC/sig verified, awaiting reboot   |
| `APPLYING`     | New bank promoted; application should confirm success if desired   |
| `ROLLBACK`     | Verification failed (CRC/signature/invalid vector) – fall back     |

Metadata lives at `0x0800E000` (4KB). `ota_download` updates size, CRC32, version, and last error codes to simplify post-mortem debugging.

### Application Development

Your application firmware should:
1. **Link against the bank start you plan to occupy**  
   - Bank A (active): `ORIGIN = 0x08002000`, `LENGTH = 24K`  
   - Bank B (staging): `ORIGIN = 0x08008000`, `LENGTH = 24K`
2. **Set vector table offset** in `SystemInit` or early init: `SCB->VTOR = BANK_START`
3. **Respect 24KB size limit** per bank (dual-bank OTA requirement)

Example linker script (Bank A):
```ld
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08002000, LENGTH = 24K
  RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 20K
}
```

## Troubleshooting

### Cannot connect debugger after flashing

**Solution**: Already fixed! Ensure you have the latest version where SWD is properly enabled.

### LED not blinking

1. Check bootloader is flashed at `0x08000000`
2. Check LED connection (PC13)
3. Check power supply (3.3V)

### Watchdog reset loop

If LED blinks rapidly every 3 seconds, the bootloader is stuck in `Error_Handler()`. This usually means:
- Invalid application at `0x08002000`
- Flash corruption

**Fix**: Erase application area and ensure valid firmware.

## Project Structure

```
.
├── Core/
│   ├── Inc/           # Main headers and config
│   └── Src/           # Main bootloader code
├── BSP/               # Board support package
│   ├── Inc/           # Flash, watchdog, OTA metadata, transports
│   └── Src/           # bsp_flash/jump/watchdog + ota_download/security/transports
├── Drivers/           # STM32 HAL and CMSIS
├── build/             # Build output (auto-generated)
├── CMakeLists.txt     # Build configuration
├── flash_bootloader.bat      # Flash script
├── test_connection.bat       # Test ST-Link connection
└── check_bootloader_size.sh  # Size check script
```

## Version History

### v1.2.0 (2025-02-XX)
- ✅ Dual-bank flash layout (24KB banks) + metadata region
- ✅ OTA download service with BLE/RF transport stubs
- ✅ CRC32 streaming + signature verification scaffolding
- ✅ Flash guardrails (per-bank erase/write helpers)
- ✅ Bootloader flow: staged image promotion + automatic rollback

### v1.1.0 (2025-01-XX)
- Watchdog + upgrade flag hardening
- Config-page based flag storage, inactive during normal boot

### v1.0.0 (2025-01-XX)
- ✅ Core bootloader functionality
- ✅ Fixed SWD debug interface (critical bug fix)
- ✅ Optimized to 4KB (50% flash usage)
- ✅ Watchdog protection
- ✅ Application validation and jump

## License

Copyright (c) 2025 TT1nker  
GitHub: [TT1nker](https://github.com/TT1nker)  
Contact: hostsjim22@gmail.com

This software is provided AS-IS without warranty of any kind.

## Contributing

Contributions welcome! Please ensure:
- Code compiles without warnings
- Bootloader size stays under 8KB
- SWD debugging remains functional
