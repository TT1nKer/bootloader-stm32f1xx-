cmake_minimum_required(VERSION 3.22)

#
# STM32F103 Bootloader CMakeLists.txt
# Standalone Bootloader Project
#
# Copyright (c) 2025 TT1nker (GitHub: TT1nker)
# All rights reserved.
#
# Contact: hostsjim22@gmail.com
#
# This software component is provided AS-IS, without any warranty of any kind.
# User is responsible for the proper use of this software.
#

# ============================================================================
# Toolchain Configuration (MUST be before project() call)
# ============================================================================
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

# Some default GCC settings
# arm-none-eabi- must be part of path environment
set(TOOLCHAIN_PREFIX                arm-none-eabi-)

# Set compiler BEFORE project() call
set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_LINKER                    ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Set the project name
set(PROJECT_NAME bootloader)

# Disable Windows-specific linker libraries before project() call
set(CMAKE_C_STANDARD_LIBRARIES "")
set(CMAKE_CXX_STANDARD_LIBRARIES "")

# Override default linker command to avoid Windows-specific options
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings (must be after compiler is set)
project(${PROJECT_NAME} C ASM)
message("Build type: " ${CMAKE_BUILD_TYPE})

# Override Windows-specific linker options after project() call
# Note: We still need nano.specs for printf, memset, etc.
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_C_STANDARD_LIBRARIES "")
set(CMAKE_CXX_STANDARD_LIBRARIES "")

# MCU specific flags
set(TARGET_FLAGS "-mcpu=cortex-m3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")

# Optimization flags (size optimization for Bootloader)
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")

set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/STM32F103XX_BOOTLOADER.ld\"")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_NAME}.map -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")

# Remove wrong libob.a library dependency
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Note: We use --specs=nano.specs instead of -nostdlib to get minimal stdlib
# This provides printf, memset, etc. without Windows-specific libraries

# ============================================================================
# Source Files
# ============================================================================

# Core source files
set(CORE_SOURCES
    Core/Src/bootloader_main.c
    Core/Src/system_stm32f1xx.c
    Core/Inc/stm32f1xx_it.c
    Core/Src/stm32f1xx_hal_msp.c
    Core/Src/syscalls.c
    Core/Src/sysmem.c
)

# BSP source files
set(BSP_SOURCES
    BSP/Src/bsp_flash.c
    BSP/Src/bsp_jump.c
    BSP/Src/bsp_ota_meta.c
    BSP/Src/bsp_upgrade_flag.c
    BSP/Src/bsp_watchdog.c
    BSP/Src/ota_download.c
    BSP/Src/ota_security.c
    BSP/Src/ota_transport_ble.c
    BSP/Src/ota_transport_rf.c
)

# HAL source files (only needed modules)
set(HAL_SOURCES
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_iwdg.c
)

# Startup file
set(STARTUP_SOURCE
    startup_stm32f103xb.s
)

# ============================================================================
# Create Executable
# ============================================================================

add_executable(${PROJECT_NAME}
    ${CORE_SOURCES}
    ${BSP_SOURCES}
    ${HAL_SOURCES}
    ${STARTUP_SOURCE}
)

# Note: Windows-specific linker options are disabled via CMAKE_C_LINK_EXECUTABLE override
# We keep --specs=nano.specs for minimal stdlib support (printf, memset, etc.)

# ============================================================================
# Include Directories
# ============================================================================

target_include_directories(${PROJECT_NAME} PRIVATE
    Core/Inc
    BSP/Inc
    Drivers/STM32F1xx_HAL_Driver/Inc
    Drivers/CMSIS/Device/ST/STM32F1xx/Include
    Drivers/CMSIS/Core/Include
)

# ============================================================================
# Compile Definitions
# ============================================================================

target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_HAL_DRIVER
    STM32F103xB
)

# ============================================================================
# Post-build: Generate binary and hex files
# ============================================================================

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Generating binary and hex files, and showing size..."
)

# ============================================================================
# Custom target: Check Bootloader size
# ============================================================================

add_custom_target(check_size
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    DEPENDS ${PROJECT_NAME}
    COMMENT "Checking Bootloader size (must be < 8KB)..."
)

