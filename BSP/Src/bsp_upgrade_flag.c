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
    // Check magic number
    uint32_t magic = BSP_Flash_ReadWord(UPGRADE_FLAG_MAGIC_ADDRESS);
    if (magic != UPGRADE_FLAG_MAGIC)
    {
        return UPGRADE_MODE_NONE;
    }
    
    uint32_t flag_value = BSP_Flash_ReadWord(UPGRADE_FLAG_ADDRESS);
    
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
    
    status = BSP_Flash_Erase(CONFIG_AREA_ADDRESS, FLASH_PAGE_SIZE);
    if (status != HAL_OK)
    {
        return status;
    }
    
    HAL_FLASH_Unlock();
    
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                               UPGRADE_FLAG_MAGIC_ADDRESS,
                               UPGRADE_FLAG_MAGIC);
    if (status == HAL_OK)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                   UPGRADE_FLAG_ADDRESS,
                                   (uint32_t)mode);
    }
    
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



