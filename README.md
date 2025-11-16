# STM32F103 Bootloader

Standalone firmware bootloader for STM32F103 microcontroller with 4KB flash constraint.

## Features

- Application boot and validation
- Firmware upgrade via CAN/BLE/RF
- Flash operations (erase, write, read, verify)
- Watchdog protection (3s timeout)
- Upgrade flag management

## Memory Layout

```
0x08000000 - 0x08000FFF (4KB)   : Bootloader
0x08001000 - 0x0800EFFF (56KB)  : Application
0x08007000 - 0x08007FFF (4KB)   : Configuration
```

## Build

```bash
mkdir build && cd build
cmake ..
make bootloader
st-flash write bootloader.bin 0x08000000
```

## Requirements

- CMake >= 3.22
- ARM GCC toolchain (arm-none-eabi-gcc)

## Status

- ✅ Core functionality complete
- ⚠️ Upgrade protocol pending

## License

Copyright (c) 2025 TT1nker  
Contact: hostsjim22@gmail.com

Provided AS-IS without warranty.
