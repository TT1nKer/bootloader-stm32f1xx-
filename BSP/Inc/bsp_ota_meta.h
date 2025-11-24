/**
  ******************************************************************************
  * @file    bsp_ota_meta.h
  * @brief   Board Support Package - OTA metadata helpers
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 TT1nker
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  ******************************************************************************
  */

#ifndef __BSP_OTA_META_H
#define __BSP_OTA_META_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "bootloader_config.h"

/* Exported constants --------------------------------------------------------*/
#define OTA_METADATA_MAGIC              0x4F54414DU   /* "OTAM" */
#define OTA_METADATA_FORMAT_VERSION     0x00010000U
#define OTA_BANK_INVALID                0xFFFFFFFFU

/* OTA states stored in metadata */
typedef enum
{
    OTA_STATE_IDLE = 0,
    OTA_STATE_DOWNLOADING = 1,
    OTA_STATE_READY = 2,
    OTA_STATE_APPLYING = 3,
    OTA_STATE_ROLLBACK = 4,
    OTA_STATE_INVALID = 0xFFFFFFFFU
} OtaState_t;

typedef enum
{
    OTA_ERROR_NONE = 0,
    OTA_ERROR_INVALID_IMAGE = 1,
    OTA_ERROR_CRC_MISMATCH = 2,
    OTA_ERROR_STORAGE_FAIL = 3
} OtaError_t;

/* Metadata layout persisted in Flash */
typedef struct
{
    uint32_t magic;
    uint32_t format_version;
    uint32_t active_bank;      /* Confirmed bank to boot next */
    uint32_t staged_bank;      /* Bank containing new firmware */
    uint32_t staged_size;      /* Bytes written to staged bank */
    uint32_t staged_crc;       /* CRC32 of staged firmware */
    uint32_t staged_version;   /* Semantic/app version */
    uint32_t state;            /* OtaState_t */
    uint32_t last_error;       /* OtaError_t */
    uint32_t reserved[5];
    uint32_t checksum;         /* Must be last member */
} OtaMetadata_t;

_Static_assert((sizeof(OtaMetadata_t) % 4U) == 0U, "Metadata must be 32-bit aligned");

/* Exported functions --------------------------------------------------------*/
bool BSP_OtaMeta_Load(OtaMetadata_t *meta);
HAL_StatusTypeDef BSP_OtaMeta_Save(const OtaMetadata_t *meta);
HAL_StatusTypeDef BSP_OtaMeta_Clear(void);
uint32_t BSP_OtaMeta_GetBankStart(uint32_t bank_index);
uint32_t BSP_OtaMeta_GetBankEnd(uint32_t bank_index);
uint32_t BSP_OtaMeta_GetInactiveBank(uint32_t bank_index);
bool BSP_OtaMeta_IsBankIndexValid(uint32_t bank_index);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_OTA_META_H */


