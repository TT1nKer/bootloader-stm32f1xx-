/**
  ******************************************************************************
  * @file    bsp_upgrade_flag.h
  * @brief   Board Support Package - Upgrade flag management
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

#ifndef __BSP_UPGRADE_FLAG_H
#define __BSP_UPGRADE_FLAG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bootloader_config.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
UpgradeMode_t BSP_UpgradeFlag_Get(void);
HAL_StatusTypeDef BSP_UpgradeFlag_Set(UpgradeMode_t mode);
HAL_StatusTypeDef BSP_UpgradeFlag_Clear(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_UPGRADE_FLAG_H */



