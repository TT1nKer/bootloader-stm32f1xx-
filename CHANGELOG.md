# Changelog

All notable changes to this project will be documented in this file.

## [1.0.0] - 2025-01-XX

### Added
- Essential bootloader functionality
- Application validation and boot
- Watchdog timer protection (3s timeout)
- Upgrade flag management in Flash
- LED status indication (PC13, 1Hz blink in upgrade mode)
- Flash operations (erase, write, read, verify)
- Support for CAN/BLE/RF upgrade modes (framework)

### Fixed
- **Critical**: Fixed SWD debugging interface disabled issue
  - Changed from `__HAL_AFIO_REMAP_SWJ_DISABLE()` to `__HAL_AFIO_REMAP_SWJ_NOJTAG()`
  - SWD (PA13/PA14) now remains active for debugging and flashing
  - JTAG (PB3/PB4/PA15) disabled to free GPIO pins

### Technical Details
- **Flash Usage**: 4012 bytes / 8KB (48.97%)
- **RAM Usage**: 1632 bytes / 20KB (7.97%)
- **Memory Layout**:
  - Bootloader: `0x08000000 - 0x08001FFF` (8KB)
  - Application: `0x08002000 - 0x0800EFFF` (52KB)
  - Config Area: `0x0800F000 - 0x0800FFFF` (4KB)

### Build Requirements
- CMake >= 3.22
- ARM GCC Toolchain (arm-none-eabi-gcc)
- Ninja or Make

### Platform Support
- Windows (native + WSL)
- Linux
- macOS
