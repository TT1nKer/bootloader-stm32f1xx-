/**
  ******************************************************************************
  * @file    app_upgrade.h
  * @brief   Application upgrade trigger interface
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

#ifndef __APP_UPGRADE_H
#define __APP_UPGRADE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Upgrade Mode (must match Bootloader definition) */
typedef enum {
    UPGRADE_MODE_NONE = 0x00,
    UPGRADE_MODE_CAN = 0x01,
    UPGRADE_MODE_BLE = 0x02,
    UPGRADE_MODE_RF = 0x03
} UpgradeMode_t;

/* Exported constants --------------------------------------------------------*/
/* Upgrade Flag Address (must match Bootloader) */
#define CONFIG_AREA_ADDRESS         0x08007000
#define UPGRADE_FLAG_ADDRESS        (CONFIG_AREA_ADDRESS + 0x00)
#define UPGRADE_FLAG_MAGIC          0xDEADBEEF

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Trigger firmware upgrade
  * @param  mode: Upgrade mode (CAN/BLE/RF)
  * @retval HAL_StatusTypeDef
  * @note   This function will set upgrade flag and reset MCU
  */
HAL_StatusTypeDef App_TriggerUpgrade(UpgradeMode_t mode);

/**
  * @brief  Set upgrade flag to Flash
  * @param  mode: Upgrade mode
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef App_UpgradeFlag_Set(UpgradeMode_t mode);

/**
  * @brief  Get current upgrade flag
  * @retval UpgradeMode_t
  */
UpgradeMode_t App_UpgradeFlag_Get(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_UPGRADE_H */

