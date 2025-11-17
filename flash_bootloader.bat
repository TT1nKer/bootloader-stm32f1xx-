@echo off
chcp 65001 >nul
REM Bootloader Flash Script
REM Use STM32CubeProgrammer to flash bootloader.hex

echo ==========================================
echo Bootloader Flash Script
echo ==========================================
echo.

REM Check if file exists
if not exist "build\bootloader.hex" (
    echo Error: bootloader.hex not found
    echo Please build first: cd build ^&^& ninja bootloader
    pause
    exit /b 1
)

echo Flashing bootloader.hex to address 0x08000000...
echo.
echo IMPORTANT: If chip is in reset loop (LED flashes every 3 seconds)
echo            HOLD RESET BUTTON while running this script!
echo.
pause

REM Use STM32CubeProgrammer to flash
REM Parameters:
REM   -c port=SWD    : Use SWD interface
REM   -w             : Write file
REM   -v             : Verify
REM   -rst           : Reset after flash

"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=SWD -w build\bootloader.hex -v -rst

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ==========================================
    echo Flash SUCCESS!
    echo ==========================================
    echo.
    echo Bootloader flashed successfully.
    echo If LED keeps flashing (200ms interval), bootloader is working correctly.
) else (
    echo.
    echo ==========================================
    echo Flash FAILED!
    echo ==========================================
    echo.
    echo Possible causes:
    echo   1. ST-Link not connected
    echo   2. Board not powered
    echo   3. Chip in reset loop - HOLD RESET BUTTON and try again
    echo   4. STM32CubeProgrammer not installed correctly
    echo.
    echo Solution for reset loop:
    echo   1. HOLD the reset button on your board
    echo   2. Run this script again
    echo   3. Release reset button after flash completes
    echo ==========================================
)

pause

