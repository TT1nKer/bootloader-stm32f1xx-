/**
  ******************************************************************************
  * @file    bsp_ota_meta.c
  * @brief   OTA metadata helper implementation
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

/* Includes ------------------------------------------------------------------*/
#include "bsp_ota_meta.h"
#include <stddef.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define OTA_METADATA_NUM_PAGES      (OTA_METADATA_SIZE / FLASH_PAGE_SIZE)

/* Private function prototypes -----------------------------------------------*/
static uint32_t CalculateChecksum(const OtaMetadata_t *meta);
static void SetDefaults(OtaMetadata_t *meta);
static void Normalize(OtaMetadata_t *meta);

/* Exported functions --------------------------------------------------------*/

bool BSP_OtaMeta_Load(OtaMetadata_t *meta)
{
    if (meta == NULL)
    {
        return false;
    }

    const OtaMetadata_t *flash_meta = (const OtaMetadata_t *)OTA_METADATA_ADDRESS;
    memcpy(meta, flash_meta, sizeof(OtaMetadata_t));

    bool basic_valid = (meta->magic == OTA_METADATA_MAGIC) &&
                       (meta->format_version == OTA_METADATA_FORMAT_VERSION) &&
                       (meta->checksum == CalculateChecksum(meta));

    if (!basic_valid)
    {
        SetDefaults(meta);
        return false;
    }

    Normalize(meta);
    return true;
}

HAL_StatusTypeDef BSP_OtaMeta_Save(const OtaMetadata_t *meta)
{
    if (meta == NULL)
    {
        return HAL_ERROR;
    }

    OtaMetadata_t buffer;
    memcpy(&buffer, meta, sizeof(OtaMetadata_t));
    Normalize(&buffer);

    buffer.magic = OTA_METADATA_MAGIC;
    buffer.format_version = OTA_METADATA_FORMAT_VERSION;
    buffer.checksum = CalculateChecksum(&buffer);

    FLASH_EraseInitTypeDef erase = {0};
    uint32_t page_error = 0U;
    HAL_StatusTypeDef status;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = OTA_METADATA_ADDRESS;
    erase.NbPages = OTA_METADATA_NUM_PAGES;

    HAL_FLASH_Unlock();
    status = HAL_FLASHEx_Erase(&erase, &page_error);

    if (status == HAL_OK)
    {
        const uint32_t *words = (const uint32_t *)&buffer;
        size_t total_words = sizeof(OtaMetadata_t) / sizeof(uint32_t);

        for (size_t i = 0; i < total_words; ++i)
        {
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                       OTA_METADATA_ADDRESS + (i * sizeof(uint32_t)),
                                       words[i]);
            if (status != HAL_OK)
            {
                break;
            }
        }
    }

    HAL_FLASH_Lock();
    return status;
}

HAL_StatusTypeDef BSP_OtaMeta_Clear(void)
{
    OtaMetadata_t meta;
    SetDefaults(&meta);
    return BSP_OtaMeta_Save(&meta);
}

uint32_t BSP_OtaMeta_GetBankStart(uint32_t bank_index)
{
    switch (bank_index)
    {
        case APP_BANK0_INDEX:
            return OTA_BANK0_START_ADDRESS;
        case APP_BANK1_INDEX:
            return OTA_BANK1_START_ADDRESS;
        default:
            return 0U;
    }
}

uint32_t BSP_OtaMeta_GetBankEnd(uint32_t bank_index)
{
    switch (bank_index)
    {
        case APP_BANK0_INDEX:
            return OTA_BANK0_END_ADDRESS;
        case APP_BANK1_INDEX:
            return OTA_BANK1_END_ADDRESS;
        default:
            return 0U;
    }
}

uint32_t BSP_OtaMeta_GetInactiveBank(uint32_t bank_index)
{
    if (bank_index == APP_BANK0_INDEX)
    {
        return APP_BANK1_INDEX;
    }
    else if (bank_index == APP_BANK1_INDEX)
    {
        return APP_BANK0_INDEX;
    }

    return APP_BANK0_INDEX;
}

bool BSP_OtaMeta_IsBankIndexValid(uint32_t bank_index)
{
    return (bank_index == APP_BANK0_INDEX) || (bank_index == APP_BANK1_INDEX);
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Calculate checksum for OTA metadata structure
  * @param  meta: Pointer to metadata structure
  * @retval Calculated checksum value
  */
static uint32_t CalculateChecksum(const OtaMetadata_t *meta)
{
    const uint32_t *words = (const uint32_t *)meta;
    const size_t total_words = sizeof(OtaMetadata_t) / sizeof(uint32_t);
    uint32_t acc = 0xA5A5A5A5U;

    for (size_t i = 0; i < (total_words - 1U); ++i)
    {
        acc ^= words[i];
        acc = (acc << 5) | (acc >> 27);
    }

    return acc;
}

/**
  * @brief  Set default values for OTA metadata
  * @param  meta: Pointer to metadata structure to initialize
  * @retval None
  */
static void SetDefaults(OtaMetadata_t *meta)
{
    if (meta == NULL)
    {
        return;
    }

    memset(meta, 0xFF, sizeof(OtaMetadata_t));
    meta->magic = OTA_METADATA_MAGIC;
    meta->format_version = OTA_METADATA_FORMAT_VERSION;
    meta->active_bank = APP_BANK0_INDEX;
    meta->staged_bank = OTA_BANK_INVALID;
    meta->staged_size = 0U;
    meta->staged_crc = 0U;
    meta->staged_version = 0U;
    meta->state = OTA_STATE_IDLE;
    meta->last_error = 0U;
    memset(meta->reserved, 0x00, sizeof(meta->reserved));
    meta->checksum = CalculateChecksum(meta);
}

/**
  * @brief  Normalize metadata by validating and correcting invalid fields
  * @param  meta: Pointer to metadata structure to normalize
  * @retval None
  */
static void Normalize(OtaMetadata_t *meta)
{
    if (meta == NULL)
    {
        return;
    }

    if (!BSP_OtaMeta_IsBankIndexValid(meta->active_bank))
    {
        meta->active_bank = APP_BANK0_INDEX;
    }

    if (!BSP_OtaMeta_IsBankIndexValid(meta->staged_bank))
    {
        meta->staged_bank = OTA_BANK_INVALID;
    }

    if (meta->state != OTA_STATE_IDLE &&
        meta->state != OTA_STATE_DOWNLOADING &&
        meta->state != OTA_STATE_READY &&
        meta->state != OTA_STATE_APPLYING &&
        meta->state != OTA_STATE_ROLLBACK)
    {
        meta->state = OTA_STATE_IDLE;
    }
}


