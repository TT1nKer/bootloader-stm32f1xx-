#!/bin/bash
#
# Bootloader Size Check Script
# Checks if bootloader.elf is within 4KB limit
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
MAX_SIZE=4096

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

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

# Get size information
SIZE_INFO=$(arm-none-eabi-size $BOOTLOADER_ELF 2>/dev/null | tail -1)

if [ -z "$SIZE_INFO" ]; then
    echo -e "${RED}Error: Failed to read bootloader size.${NC}"
    echo "Make sure arm-none-eabi-size is in your PATH"
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
    echo -e "${RED}❌ ERROR: Bootloader exceeds 4KB limit!${NC}"
    echo -e "${RED}   Exceeded by: $EXCEEDED bytes${NC}"
    echo ""
    echo "Suggestions:"
    echo "  1. Enable compiler optimizations (-Os)"
    echo "  2. Remove unused code (--gc-sections)"
    echo "  3. Remove debug code (printf, assert, etc.)"
    echo "  4. Simplify error handling"
    echo "  5. Consider increasing Bootloader size to 8KB"
    exit 1
elif [ $USAGE_PERCENT -gt 90 ]; then
    echo -e "${YELLOW}⚠️  WARNING: Bootloader is close to 4KB limit!${NC}"
    echo -e "${YELLOW}   Consider optimizing code or increasing size.${NC}"
    exit 0
else
    echo -e "${GREEN}✅ OK: Bootloader is within 4KB limit${NC}"
    echo -e "${GREEN}   $REMAINING bytes remaining for future features${NC}"
    exit 0
fi

