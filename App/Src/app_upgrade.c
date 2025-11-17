/**
  ******************************************************************************
  * @file    app_upgrade.c
  * @brief   Application upgrade trigger implementation
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 TT1nker (GitHub: TT1nker)
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_upgrade.h"
#include "stm32f1xx_hal.h"

/* Private function prototypes -----------------------------------------------*/
static uint32_t ReadFlashWord(uint32_t address);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Read a word from Flash
  * @param  address: Flash address
  * @retval Word value
  */
static uint32_t ReadFlashWord(uint32_t address)
{
    return *((volatile uint32_t *)address);
}

/**
  * @brief  Get current upgrade flag
  * @retval UpgradeMode_t
  */
UpgradeMode_t App_UpgradeFlag_Get(void)
{
    uint32_t flag_value = ReadFlashWord(UPGRADE_FLAG_ADDRESS);
    
    // Check magic number
    uint32_t magic = ReadFlashWord(UPGRADE_FLAG_ADDRESS - 4);
    if (magic != UPGRADE_FLAG_MAGIC)
    {
        return UPGRADE_MODE_NONE;
    }
    
    // Return upgrade mode (only lower 8 bits)
    return (UpgradeMode_t)(flag_value & 0xFF);
}

/**
  * @brief  Set upgrade flag to Flash
  * @param  mode: Upgrade mode
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef App_UpgradeFlag_Set(UpgradeMode_t mode)
{
    HAL_StatusTypeDef status;
    
    // Unlock Flash
    HAL_FLASH_Unlock();
    
    // Write magic number
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 
                               UPGRADE_FLAG_ADDRESS - 4, 
                               UPGRADE_FLAG_MAGIC);
    if (status != HAL_OK)
    {
        HAL_FLASH_Lock();
        return status;
    }
    
    // Write upgrade mode
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 
                               UPGRADE_FLAG_ADDRESS, 
                               (uint32_t)mode);
    
    // Lock Flash
    HAL_FLASH_Lock();
    
    return status;
}

/**
  * @brief  Trigger firmware upgrade
  * @param  mode: Upgrade mode (CAN/BLE/RF)
  * @retval HAL_StatusTypeDef
  * @note   This function will set upgrade flag and reset MCU
  */
HAL_StatusTypeDef App_TriggerUpgrade(UpgradeMode_t mode)
{
    HAL_StatusTypeDef status;
    
    // 1. Set upgrade flag to Flash
    status = App_UpgradeFlag_Set(mode);
    if (status != HAL_OK)
    {
        return status;
    }
    
    // 2. Wait for Flash write to complete (important!)
    HAL_Delay(100);
    
    // 3. Software reset (MCU will boot into Bootloader)
    NVIC_SystemReset();
    
    // Should never reach here
    return HAL_ERROR;
}

