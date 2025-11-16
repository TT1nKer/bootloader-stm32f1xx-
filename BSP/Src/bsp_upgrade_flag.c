/**
  ******************************************************************************
  * @file    bsp_upgrade_flag.c
  * @brief   Board Support Package - Upgrade flag management implementation
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
#include "bsp_upgrade_flag.h"
#include "bsp_flash.h"

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Get upgrade flag
  * @retval UpgradeMode_t
  */
UpgradeMode_t BSP_UpgradeFlag_Get(void)
{
    uint32_t flag_value = BSP_Flash_ReadWord(UPGRADE_FLAG_ADDRESS);
    
    // Check magic number
    uint32_t magic = BSP_Flash_ReadWord(UPGRADE_FLAG_ADDRESS - 4);
    if (magic != UPGRADE_FLAG_MAGIC)
    {
        return UPGRADE_MODE_NONE;
    }
    
    // Return upgrade mode (only lower 8 bits)
    return (UpgradeMode_t)(flag_value & 0xFF);
}

/**
  * @brief  Set upgrade flag
  * @param  mode: Upgrade mode
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_UpgradeFlag_Set(UpgradeMode_t mode)
{
    HAL_StatusTypeDef status;
    
    // Erase the configuration area first (if needed)
    // Note: In practice, we might want to erase only the specific word
    // For simplicity, we erase the whole page
    
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
  * @brief  Clear upgrade flag
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_UpgradeFlag_Clear(void)
{
    return BSP_UpgradeFlag_Set(UPGRADE_MODE_NONE);
}



