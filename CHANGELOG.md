# Changelog

All notable changes to this project will be documented in this file.

# Changelog

All notable changes to this project will be documented in this file.

## [1.2.0] - 2025-02-XX

### Added
- Dual-bank flash layout (24KB active + 24KB staging) with dedicated metadata page
- OTA download service with BLE/RF transport shims (control/data/abort packets)
- Streaming CRC32 helper + signature verification scaffolding (`ota_security`)
- BSP flash helpers for per-bank erase/write + chunked programming API
- OTA metadata module (version, CRC, size, state machine, last error field)

### Changed
- Bootloader flow promotes staged firmware, validates it, and rolls back on failure
- Upgrade preparation only arms watchdog when staying in bootloader
- Documentation updated with OTA workflow, transport API, and testing guidance

### Technical Details
- **Flash Usage**: ~4.2KB / 8KB (varies with toolchain)
- **Memory Layout**:
  - Bootloader: `0x08000000 - 0x08001FFF` (8KB)
  - Bank A (active): `0x08002000 - 0x08007FFF` (24KB)
  - Bank B (staging): `0x08008000 - 0x0800DFFF` (24KB)
  - OTA metadata: `0x0800E000 - 0x0800EFFF` (4KB)
  - Config area: `0x0800F000 - 0x0800FFFF` (4KB)

## [1.1.0] - 2025-01-XX

### Added
- Watchdog initialization guards + status tracking
- Upgrade flag relocated to config page with safe erase/write sequence
- Flash BSP range checks to prevent accidental bootloader erasure

### Fixed
- IWDG only armed while staying in upgrade mode (legacy apps remain stable)
- Upgrade flag no longer cleared spuriously in normal boot path

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

