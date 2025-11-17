/**
  ******************************************************************************
  * @file    bsp_watchdog.h
  * @brief   Board Support Package - Watchdog
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

#ifndef __BSP_WATCHDOG_H
#define __BSP_WATCHDOG_H

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
HAL_StatusTypeDef BSP_Watchdog_Init(uint32_t timeout_ms);
void BSP_Watchdog_Feed(void);
bool BSP_Watchdog_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_WATCHDOG_H */



