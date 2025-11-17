@echo off
chcp 65001 >nul
REM 测试 ST-Link 连接

echo ==========================================
echo 测试 ST-Link 连接
echo ==========================================
echo.

REM 使用完整路径
set STM32_CLI="C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"

echo 1. 检查 ST-Link 设备...
%STM32_CLI% -l
echo.

echo 2. 尝试连接...
%STM32_CLI% -c port=SWD
echo.

if %ERRORLEVEL% EQU 0 (
    echo ==========================================
    echo 连接成功！
    echo ==========================================
    echo.
    echo 3. 读取芯片信息...
    %STM32_CLI% -c port=SWD -r32 0xE0042000 1
) else (
    echo ==========================================
    echo 连接失败！
    echo ==========================================
    echo.
    echo 可能的原因:
    echo   1. ST-Link 未连接
    echo   2. 开发板未上电
    echo   3. SWD 连接线问题
    echo   4. 芯片被锁定
    echo.
    echo 尝试解锁芯片:
    %STM32_CLI% -c port=SWD -ob nSWBOOT0=1 nBOOT0=1
)

echo.
pause

