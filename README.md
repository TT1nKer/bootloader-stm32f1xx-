# STM32F103 Bootloader

Minimalist bootloader for STM32F103 microcontroller with 8KB flash constraint.

## Features

- ✅ Application boot and validation
- ✅ Firmware upgrade via CAN/BLE/RF (framework ready)
- ✅ Flash operations (erase, write, read, verify)
- ✅ Independent watchdog protection (3s timeout)
- ✅ Upgrade flag management
- ✅ SWD debugging support (JTAG disabled to free pins)

## Memory Layout

```
0x08000000 - 0x08001FFF (8KB)   : Bootloader
0x08002000 - 0x0800EFFF (52KB)  : Application
0x0800F000 - 0x0800FFFF (4KB)   : Configuration
```

**Current size**: 4012 bytes / 8192 bytes (48.97%)

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

### Application Development

Your application firmware should:
1. **Start at `0x08002000`** (8KB offset)
2. **Set vector table offset**: `SCB->VTOR = 0x08002000;`
3. **Size limit**: 52KB maximum

Example linker script:
```ld
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08002000, LENGTH = 52K
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
│   ├── Inc/           # BSP headers
│   └── Src/           # Flash, jump, watchdog, upgrade flag
├── Drivers/           # STM32 HAL and CMSIS
├── build/             # Build output (auto-generated)
├── CMakeLists.txt     # Build configuration
├── flash_bootloader.bat      # Flash script
├── test_connection.bat       # Test ST-Link connection
└── check_bootloader_size.sh  # Size check script
```

## Version History

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
