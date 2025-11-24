#!/bin/bash
#
# Bootloader Size Check Script (Cross-Platform)
# Checks if bootloader.elf is within 8KB limit
# Works on Windows (Git Bash/WSL), macOS, and Linux
#
# Copyright (c) 2025 TT1nker (GitHub: TT1nker)
# All rights reserved.
#
# Contact: hostsjim22@gmail.com
#
# This software component is provided AS-IS, without any warranty of any kind.
# User is responsible for the proper use of this software.
#

BOOTLOADER_ELF="build/bootloader.elf"
MAX_SIZE=8192

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Detect OS
OS="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    OS="windows"
elif [[ -d "/mnt/c" ]]; then
    OS="wsl"  # Windows Subsystem for Linux
fi

echo "=========================================="
echo "Bootloader Size Check"
echo "=========================================="

# Check if file exists
if [ ! -f "$BOOTLOADER_ELF" ]; then
    echo -e "${RED}Error: $BOOTLOADER_ELF not found.${NC}"
    echo "Please build bootloader first:"
    echo "  cd build && cmake .. && make bootloader"
    exit 1
fi

# Try to find arm-none-eabi-size (cross-platform)
SIZE_CMD=""

# First, try standard PATH
if command -v arm-none-eabi-size >/dev/null 2>&1; then
    SIZE_CMD="arm-none-eabi-size"
elif command -v arm-none-eabi-size.exe >/dev/null 2>&1; then
    SIZE_CMD="arm-none-eabi-size.exe"
else
    # Platform-specific search paths
    case "$OS" in
        "windows"|"wsl")
            # Windows paths (both native and WSL)
            WINDOWS_PATHS=(
                "/c/Program Files/ARM/gcc-arm-none-eabi-"*/bin/arm-none-eabi-size.exe
                "/c/Program Files (x86)/ARM/gcc-arm-none-eabi-"*/bin/arm-none-eabi-size.exe
                "/mnt/c/Program Files/ARM/gcc-arm-none-eabi-"*/bin/arm-none-eabi-size.exe
                "/mnt/c/Program Files (x86)/ARM/gcc-arm-none-eabi-"*/bin/arm-none-eabi-size.exe
                "$HOME/AppData/Local/Programs/ARM/gcc-arm-none-eabi-"*/bin/arm-none-eabi-size.exe
            )
            for path_pattern in "${WINDOWS_PATHS[@]}"; do
                # Expand glob pattern
                for path in $path_pattern; do
                    if [ -f "$path" ]; then
                        SIZE_CMD="$path"
                        break 2
                    fi
                done
            done
            ;;
        "macos")
            # macOS paths (Homebrew, MacPorts, manual install)
            MACOS_PATHS=(
                "/opt/homebrew/bin/arm-none-eabi-size"
                "/usr/local/bin/arm-none-eabi-size"
                "/opt/local/bin/arm-none-eabi-size"
                "$HOME/homebrew/bin/arm-none-eabi-size"
                "/Applications/ARM/bin/arm-none-eabi-size"
            )
            for path in "${MACOS_PATHS[@]}"; do
                if [ -f "$path" ]; then
                    SIZE_CMD="$path"
                    break
                fi
            done
            ;;
        "linux")
            # Linux paths (standard install locations)
            LINUX_PATHS=(
                "/usr/bin/arm-none-eabi-size"
                "/usr/local/bin/arm-none-eabi-size"
                "/opt/gcc-arm-none-eabi-"*/bin/arm-none-eabi-size
            )
            for path_pattern in "${LINUX_PATHS[@]}"; do
                for path in $path_pattern; do
                    if [ -f "$path" ]; then
                        SIZE_CMD="$path"
                        break 2
                    fi
                done
            done
            ;;
    esac
fi

if [ -z "$SIZE_CMD" ]; then
    echo -e "${RED}Error: arm-none-eabi-size not found.${NC}"
    echo ""
    echo "Please install ARM GCC toolchain:"
    echo ""
    case "$OS" in
        "macos")
            echo "  macOS (using Homebrew):"
            echo "    brew install arm-none-eabi-gcc"
            echo ""
            echo "  Or download from:"
            echo "    https://developer.arm.com/downloads/-/gnu-rm"
            ;;
        "linux"|"wsl")
            echo "  Linux/WSL:"
            echo "    sudo apt-get update"
            echo "    sudo apt-get install gcc-arm-none-eabi"
            echo ""
            echo "  Or download from:"
            echo "    https://developer.arm.com/downloads/-/gnu-rm"
            ;;
        "windows")
            echo "  Windows (using Chocolatey):"
            echo "    choco install gcc-arm-embedded"
            echo ""
            echo "  Or download from:"
            echo "    https://developer.arm.com/downloads/-/gnu-rm"
            echo ""
            echo "  Then add to PATH:"
            echo "    C:\\Program Files\\ARM\\gcc-arm-none-eabi-xxx\\bin"
            ;;
        *)
            echo "  Download from: https://developer.arm.com/downloads/-/gnu-rm"
            echo "  Add to PATH after installation"
            ;;
    esac
    exit 1
fi

# Get size information
SIZE_INFO=$($SIZE_CMD $BOOTLOADER_ELF 2>/dev/null | tail -1)

if [ -z "$SIZE_INFO" ]; then
    echo -e "${RED}Error: Failed to read bootloader size.${NC}"
    echo "Command used: $SIZE_CMD"
    echo "File: $BOOTLOADER_ELF"
    exit 1
fi

# Parse size information
TEXT=$(echo $SIZE_INFO | awk '{print $1}')
DATA=$(echo $SIZE_INFO | awk '{print $2}')
BSS=$(echo $SIZE_INFO | awk '{print $3}')
TOTAL_FLASH=$((TEXT + DATA))
USAGE_PERCENT=$((TOTAL_FLASH * 100 / MAX_SIZE))
REMAINING=$((MAX_SIZE - TOTAL_FLASH))

# Display results
echo ""
echo "Size Breakdown:"
echo "  Text (code):        $(printf "%6d" $TEXT) bytes"
echo "  Data (initialized): $(printf "%6d" $DATA) bytes"
echo "  BSS (uninitialized): $(printf "%6d" $BSS) bytes (RAM only)"
echo ""
echo "Flash Usage:"
echo "  Total Flash:        $(printf "%6d" $TOTAL_FLASH) bytes / $(printf "%6d" $MAX_SIZE) bytes"
echo "  Usage:              $(printf "%6d" $USAGE_PERCENT)%%"
echo "  Remaining:          $(printf "%6d" $REMAINING) bytes"
echo ""

# Check if exceeds limit
if [ $TOTAL_FLASH -gt $MAX_SIZE ]; then
    EXCEEDED=$((TOTAL_FLASH - MAX_SIZE))
    echo -e "${RED}❌ ERROR: Bootloader exceeds 8KB limit!${NC}"
    echo -e "${RED}   Exceeded by: $EXCEEDED bytes${NC}"
    echo ""
    echo "Suggestions:"
    echo "  1. Enable compiler optimizations (-Os)"
    echo "  2. Remove unused code (--gc-sections)"
    echo "  3. Remove debug code (printf, assert, etc.)"
    echo "  4. Simplify error handling"
    echo "  5. Consider increasing Bootloader size limit"
    exit 1
elif [ $USAGE_PERCENT -gt 90 ]; then
    echo -e "${YELLOW}⚠️  WARNING: Bootloader is close to 8KB limit!${NC}"
    echo -e "${YELLOW}   Consider optimizing code or increasing size.${NC}"
    exit 0
else
    echo -e "${GREEN}✅ OK: Bootloader is within 8KB limit${NC}"
    echo -e "${GREEN}   $REMAINING bytes remaining for future features${NC}"
    exit 0
fi

