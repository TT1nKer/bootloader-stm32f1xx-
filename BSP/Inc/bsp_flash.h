/**
  ******************************************************************************
  * @file    bsp_flash.h
  * @brief   Board Support Package - Flash operations
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

#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "main.h"
#include "bootloader_config.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
HAL_StatusTypeDef BSP_Flash_Erase(uint32_t address, uint32_t size);
HAL_StatusTypeDef BSP_Flash_Write(uint32_t address, uint8_t *data, uint32_t length);
HAL_StatusTypeDef BSP_Flash_Read(uint32_t address, uint8_t *data, uint32_t length);
bool BSP_Flash_Verify(uint32_t address, uint8_t *expected_data, uint32_t length);
uint32_t BSP_Flash_ReadWord(uint32_t address);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_FLASH_H */

