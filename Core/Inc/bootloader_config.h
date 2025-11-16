/**
  ******************************************************************************
  * @file    bootloader_config.h
  * @brief   Bootloader configuration file
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

#ifndef __BOOTLOADER_CONFIG_H
#define __BOOTLOADER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Memory Layout Configuration */
#define BOOTLOADER_START_ADDRESS    0x08000000
#define BOOTLOADER_SIZE             (4 * 1024)      // 4KB
#define BOOTLOADER_END_ADDRESS      (BOOTLOADER_START_ADDRESS + BOOTLOADER_SIZE - 1)

#define APP_START_ADDRESS           0x08001000      // Application starts at 4KB offset
#define APP_SIZE                    (12 * 1024)     // 12KB for application
#define APP_END_ADDRESS             (APP_START_ADDRESS + APP_SIZE - 1)

#define BACKUP_APP_ADDRESS          0x08004000      // Backup application area (optional)
#define BACKUP_APP_SIZE              (12 * 1024)     // 12KB

#define CONFIG_AREA_ADDRESS         0x08007000      // Configuration area
#define CONFIG_AREA_SIZE            (4 * 1024)       // 4KB

/* Upgrade Flag Address */
#define UPGRADE_FLAG_ADDRESS        (CONFIG_AREA_ADDRESS + 0x00)
#define FIRMWARE_VERSION_ADDRESS    (CONFIG_AREA_ADDRESS + 0x04)
#define UPGRADE_STATE_ADDRESS       (CONFIG_AREA_ADDRESS + 0x08)

/* Flash Page Size */
#ifdef FLASH_PAGE_SIZE
#undef FLASH_PAGE_SIZE
#endif
#define FLASH_PAGE_SIZE             1024            // STM32F103 page size is 1KB

/* Upgrade Mode */
typedef enum {
    UPGRADE_MODE_NONE = 0x00,
    UPGRADE_MODE_CAN = 0x01,
    UPGRADE_MODE_BLE = 0x02,
    UPGRADE_MODE_RF = 0x03
} UpgradeMode_t;

/* Magic Numbers */
#define UPGRADE_FLAG_MAGIC          0xDEADBEEF
#define FIRMWARE_VALID_MAGIC        0x12345678

/* Watchdog Configuration */
#define WATCHDOG_TIMEOUT_MS         3000            // 3 seconds

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_CONFIG_H */



