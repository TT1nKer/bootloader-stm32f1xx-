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
#define FLASH_BASE_ADDRESS          0x08000000U
#define FLASH_TOTAL_SIZE            (64 * 1024U)
#define FLASH_END_ADDRESS           (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE - 1U)

#define BOOTLOADER_START_ADDRESS    FLASH_BASE_ADDRESS
#define BOOTLOADER_SIZE             (8 * 1024U)     /* 8KB */
#define BOOTLOADER_END_ADDRESS      (BOOTLOADER_START_ADDRESS + BOOTLOADER_SIZE - 1U)

#define OTA_BANK_SIZE               (24 * 1024U)    /* 24KB per bank */
#define OTA_BANK0_START_ADDRESS     (BOOTLOADER_START_ADDRESS + BOOTLOADER_SIZE)        /* 0x08002000 */
#define OTA_BANK0_END_ADDRESS       (OTA_BANK0_START_ADDRESS + OTA_BANK_SIZE - 1U)
#define OTA_BANK1_START_ADDRESS     (OTA_BANK0_END_ADDRESS + 1U)                        /* 0x08008000 */
#define OTA_BANK1_END_ADDRESS       (OTA_BANK1_START_ADDRESS + OTA_BANK_SIZE - 1U)       /* 0x0800DFFF */

/* OTA Metadata + Config areas (keep separate pages to avoid erase collisions) */
#define OTA_METADATA_ADDRESS        0x0800E000U
#define OTA_METADATA_SIZE           (4 * 1024U)
#define OTA_METADATA_END_ADDRESS    (OTA_METADATA_ADDRESS + OTA_METADATA_SIZE - 1U)

#define CONFIG_AREA_ADDRESS         0x0800F000U
#define CONFIG_AREA_SIZE            (4 * 1024U)
#define CONFIG_AREA_END_ADDRESS     (CONFIG_AREA_ADDRESS + CONFIG_AREA_SIZE - 1U)

/* Legacy upgrade-flag addresses (used by watchdog/upgrade BSP helpers) */
#define UPGRADE_FLAG_MAGIC_ADDRESS  (CONFIG_AREA_ADDRESS + 0x00U)
#define UPGRADE_FLAG_ADDRESS        (CONFIG_AREA_ADDRESS + 0x04U)
#define FIRMWARE_VERSION_ADDRESS    (CONFIG_AREA_ADDRESS + 0x08U)
#define UPGRADE_STATE_ADDRESS       (CONFIG_AREA_ADDRESS + 0x0CU)

/* Public key storage for signature verification */
#define ECDSA_PUBLIC_KEY_ADDRESS    (CONFIG_AREA_ADDRESS + 0x10U)  /* 64 bytes */

/* Application helpers (defaults to Bank0 until metadata promotes Bank1) */
#define APP_BANK_COUNT              2U
#define APP_BANK0_INDEX             0U
#define APP_BANK1_INDEX             1U
#define APP_START_ADDRESS           OTA_BANK0_START_ADDRESS
#define APP_END_ADDRESS             OTA_BANK0_END_ADDRESS

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

/* Feature Enable Flags */
#define ENABLE_POWER_MONITOR        1               // Enable power monitoring
#define ENABLE_CAN_TRANSPORT        1               // Enable CAN transport
#define ENABLE_SIGNATURE_VERIFY     0               // Enable signature verification (optional - adds ~8KB)

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_CONFIG_H */



