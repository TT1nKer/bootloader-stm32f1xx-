# OTA Bootloader 实现要点总结

基于 `~/proTemperature/bootloader_ota_implementation.md` 的实现方案

## 一、核心架构设计

### 1.1 内存布局（STM32F103C8T6 - 64KB Flash）

```
┌─────────────────────────────────────┐
│ 0x08000000 - 0x08000FFF (4KB)       │  Bootloader区域（只读，永不更新）
│ 0x08001000 - 0x08003FFF (12KB)      │  应用程序区域A（主程序）
│ 0x08004000 - 0x08006FFF (12KB)      │  应用程序区域B（备份程序，可选）
│ 0x08007000 - 0x08007FFF (4KB)       │  配置数据区（升级标志、版本号等）
│ 0x08008000 - 0x0800FFFF (32KB)      │  数据存储区（可选）
└─────────────────────────────────────┘
```

### 1.2 升级方式

1. **CAN总线升级**（主要方式）
2. **蓝牙OTA升级**（通过GR5515IGND）
3. **射频OTA升级**（通过CMT2300A）

## 二、CAN升级协议设计

### 2.1 CAN帧格式

```
数据包结构（8字节CAN帧）：
┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
│ CMD  │ SEQ  │ LEN  │ DATA0│ DATA1│ DATA2│ DATA3│ CRC  │
└──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘

命令类型（CMD）：
- 0x01: 升级开始命令
- 0x02: 数据包传输
- 0x03: 升级结束命令
- 0x04: 校验请求
- 0x05: 跳转应用程序
- 0x06: 错误响应
- 0x07: 状态查询
```

### 2.2 升级流程

```
1. 升级工具发送升级开始命令
   └─> Bootloader响应：准备就绪
   └─> Bootloader检查电源电压（>10V才允许升级）
   └─> Bootloader备份当前固件到备份区

2. 升级工具发送固件数据包（分片传输）
   └─> Bootloader响应：接收确认
   └─> 如果丢失，请求重传（最多3次）
   └─> 每个数据包写入后立即校验

3. 升级工具发送升级结束命令
   └─> Bootloader响应：开始校验

4. Bootloader计算固件CRC32（双重校验）
   └─> 发送校验值

5. 升级工具验证校验值
   └─> 如果校验失败，发送回滚命令
   └─> 如果校验成功，发送跳转命令
   └─> Bootloader跳转到应用程序
```

## 三、关键实现模块

### 3.1 Bootloader主程序结构

```c
int main(void)
{
    // 1. 系统初始化
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_CAN_Init();
    MX_IWDG_Init();  // 看门狗初始化（3秒超时）
    
    // 2. 检查升级标志
    UpgradeMode_t upgrade_mode = GetUpgradeFlag();
    
    if (upgrade_mode == UPGRADE_MODE_NONE)
    {
        // 正常启动，跳转到应用程序
        JumpToApplication();
    }
    else
    {
        // 进入升级模式
        EnterUpgradeMode(upgrade_mode);
        
        while(1)
        {
            ProcessUpgradeMessages();  // 处理CAN/蓝牙/射频消息
            FeedWatchdog();            // 喂狗（每500ms）
        }
    }
}
```

### 3.2 CAN升级处理

```c
void HandleUpgradeCommand(uint8_t *data)
{
    uint8_t cmd = data[0];
    uint8_t seq = data[1];
    uint8_t len = data[2];
    
    switch(cmd)
    {
        case CMD_UPGRADE_START:
            // 检查电源电压
            if (!CheckPowerSupply()) {
                SendErrorResponse(ERR_LOW_VOLTAGE);
                return;
            }
            // 备份当前固件
            BackupCurrentFirmware();
            // 初始化升级状态
            InitUpgradeState();
            SendStatusResponse(STATUS_READY);
            break;
            
        case CMD_DATA_PACKET:
            // 检查序列号
            if (seq != expected_seq) {
                RequestRetransmit(expected_seq);
                return;
            }
            // 写入Flash
            WriteFirmwareToFlash(address, &data[3], len);
            // 验证写入
            if (!VerifyFlashWrite(address, &data[3], len)) {
                SendErrorResponse(ERR_FLASH_WRITE);
                return;
            }
            // 更新CRC32
            UpdateCRC32(&data[3], len);
            SendStatusResponse(STATUS_RECEIVING);
            break;
            
        case CMD_UPGRADE_END:
            // 计算最终CRC32
            uint32_t crc32 = GetCRC32();
            SendCRC32Response(crc32);
            break;
            
        case CMD_JUMP_APP:
            // 验证固件完整性
            if (!VerifyFirmware()) {
                RollbackFirmware();
                return;
            }
            JumpToApplication();
            break;
    }
}
```

### 3.3 Flash写入实现

```c
HAL_StatusTypeDef WriteFirmwareToFlash(uint32_t address, uint8_t *data, uint32_t length)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    
    // 1. 解锁Flash
    HAL_FLASH_Unlock();
    
    // 2. 擦除目标扇区
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = address;
    EraseInitStruct.NbPages = (length + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return status;
    }
    
    // 3. 写入数据（按半字写入）
    for (uint32_t i = 0; i < length; i += 2)
    {
        uint16_t halfword = data[i] | (data[i+1] << 8);
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 
                                   address + i, halfword);
        if (status != HAL_OK) {
            break;
        }
        
        // 每写入1KB喂一次狗
        if ((i % 1024) == 0) {
            FeedWatchdog();
        }
    }
    
    // 4. 锁定Flash
    HAL_FLASH_Lock();
    
    return status;
}
```

### 3.4 应用程序跳转

```c
void JumpToApplication(void)
{
    // 1. 关闭所有中断
    __disable_irq();
    
    // 2. 关闭看门狗
    HAL_IWDG_DeInit(&hiwdg);
    
    // 3. 关闭外设
    HAL_CAN_DeInit(&hcan);
    
    // 4. 设置堆栈指针
    uint32_t app_stack = *((volatile uint32_t*)APP_START_ADDRESS);
    __set_MSP(app_stack);
    
    // 5. 设置程序计数器
    uint32_t app_entry = *((volatile uint32_t*)(APP_START_ADDRESS + 4));
    
    // 6. 跳转到应用程序
    ((void(*)())app_entry)();
}
```

## 四、安全机制

### 4.1 看门狗保护（IWDG）

```c
// 初始化看门狗（3秒超时）
HAL_StatusTypeDef InitWatchdog(uint32_t timeout_ms)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;  // 64分频
    hiwdg.Init.Reload = 1249;                   // 约2秒超时
    
    return HAL_IWDG_Init(&hiwdg);
}

// 在主循环中定期喂狗
void FeedWatchdog(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}
```

### 4.2 固件校验（CRC32）

```c
uint32_t CalculateFirmwareCRC32(uint32_t start_addr, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    
    for (uint32_t i = 0; i < length; i++)
    {
        uint8_t byte = *((volatile uint8_t*)(start_addr + i));
        crc = CRC32_Update(crc, byte);
    }
    
    return crc ^ 0xFFFFFFFF;
}

bool VerifyFirmware(void)
{
    // 检查向量表
    uint32_t app_stack = *((volatile uint32_t*)APP_START_ADDRESS);
    uint32_t app_entry = *((volatile uint32_t*)(APP_START_ADDRESS + 4));
    
    // 检查有效性
    if (app_stack < 0x20000000 || app_stack > 0x20005000) return false;
    if (app_entry < 0x08001000 || app_entry > 0x08010000) return false;
    
    // 计算CRC32
    uint32_t calculated_crc = CalculateFirmwareCRC32(APP_START_ADDRESS, FIRMWARE_SIZE);
    uint32_t expected_crc = *((volatile uint32_t*)(APP_START_ADDRESS + FIRMWARE_SIZE - 4));
    
    return (calculated_crc == expected_crc);
}
```

### 4.3 升级失败回滚

```c
void BackupCurrentFirmware(void)
{
    // 在升级前备份当前固件到备份区
    uint32_t src = APP_START_ADDRESS;
    uint32_t dst = BACKUP_APP_ADDRESS;
    uint32_t size = FIRMWARE_SIZE;
    
    for (uint32_t i = 0; i < size; i += 4)
    {
        uint32_t data = *((volatile uint32_t*)(src + i));
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dst + i, data);
    }
}

void RollbackFirmware(void)
{
    // 从备份区恢复固件
    uint32_t src = BACKUP_APP_ADDRESS;
    uint32_t dst = APP_START_ADDRESS;
    uint32_t size = FIRMWARE_SIZE;
    
    // 擦除应用程序区域
    // ... 擦除代码 ...
    
    // 恢复固件
    for (uint32_t i = 0; i < size; i += 4)
    {
        uint32_t data = *((volatile uint32_t*)(src + i));
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dst + i, data);
    }
}
```

## 五、与当前BSP+App架构集成

### 5.1 在应用程序中触发升级

```c
// App/Src/app.c
void App_TriggerCANUpgrade(void)
{
    // 1. 设置升级标志
    SetUpgradeFlag(UPGRADE_MODE_CAN);
    
    // 2. 软件复位
    NVIC_SystemReset();
}
```

### 5.2 应用程序配置修改

```c
// Core/Src/system_stm32f1xx.c
void SystemInit(void)
{
    // 重映射中断向量表到应用程序起始地址
    SCB->VTOR = FLASH_BASE | 0x1000;  // 0x08001000
    
    // ... 其他初始化代码
}
```

### 5.3 链接脚本修改

```ld
/* STM32F103C8T6 Memory Layout for Bootloader */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08001000, LENGTH = 12K  /* Application */
    RAM (xrw)   : ORIGIN = 0x20000000, LENGTH = 20K
}
```

## 六、实现步骤

### 阶段1：Bootloader基础框架
1. 创建独立的Bootloader工程
2. 配置内存布局（链接脚本）
3. 实现Flash读写功能
4. 实现应用程序跳转功能

### 阶段2：CAN升级功能
1. 在BSP层添加CAN升级处理
2. 实现CAN升级协议
3. 实现数据包接收和重组
4. 实现固件写入Flash
5. 实现CRC校验

### 阶段3：安全机制
1. 实现看门狗保护
2. 实现固件校验
3. 实现升级失败回滚
4. 实现电源监控

### 阶段4：应用程序适配
1. 修改应用程序中断向量表
2. 实现升级触发功能
3. 测试端到端升级流程

## 七、关键注意事项

1. **看门狗配置**：Bootloader中必须配置看门狗，超时时间建议2-5秒
2. **Flash写入验证**：每次写入后立即验证，防止Flash损坏
3. **电源监控**：升级前和升级过程中持续监控电源电压
4. **升级状态保存**：在配置区保存升级状态，防止掉电后无法恢复
5. **数据包重传**：实现自动重传机制，最多3次
6. **CRC校验**：三级校验（数据包级、固件级、启动级）

## 八、测试要点

1. **功能测试**：CAN升级、蓝牙OTA、射频OTA
2. **可靠性测试**：掉电测试、通信故障测试、Flash写入失败测试
3. **环境测试**：高温、低温、振动、EMC干扰
4. **长时间运行测试**：连续运行1000小时

---

**参考文档**：`~/proTemperature/bootloader_ota_implementation.md`



