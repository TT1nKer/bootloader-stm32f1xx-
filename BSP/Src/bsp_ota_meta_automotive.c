/**
  ******************************************************************************
  * @file    bsp_ota_meta_automotive.c
  * @brief   Automotive-grade OTA metadata implementation
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
#include "bsp_ota_meta_automotive.h"
#include "bsp_flash.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define AUTOMOTIVE_META_SIZE          sizeof(AutomotiveOtaMetadata_t)
#define AUTOMOTIVE_CHECKSUM_SEED      0xDEADBEEF

/* Private function prototypes -----------------------------------------------*/
static uint32_t CalculateAutomotiveChecksum(const AutomotiveOtaMetadata_t *meta);
static bool ValidateAutomotiveChecksum(const AutomotiveOtaMetadata_t *meta);
static void InitializeAutomotiveDefaults(AutomotiveOtaMetadata_t *meta);

/**
  * @brief  Load automotive OTA metadata from flash
  * @param  meta: Pointer to automotive metadata structure
  * @retval true if successful, false otherwise
  */
bool BSP_AutomotiveMeta_Load(AutomotiveOtaMetadata_t *meta)
{
    if (meta == NULL) {
        return false;
    }

    // Read metadata from flash
    memcpy(meta, (const void*)OTA_METADATA_ADDRESS, AUTOMOTIVE_META_SIZE);

    // Check magic number
    if (meta->magic != OTA_METADATA_MAGIC) {
        // Initialize with defaults if invalid
        InitializeAutomotiveDefaults(meta);
        return false;
    }

    // Validate checksum
    if (!ValidateAutomotiveChecksum(meta)) {
        // Checksum invalid, initialize with defaults
        InitializeAutomotiveDefaults(meta);
        return false;
    }

    return true;
}

/**
  * @brief  Save automotive OTA metadata to flash
  * @param  meta: Pointer to automotive metadata structure
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_AutomotiveMeta_Save(const AutomotiveOtaMetadata_t *meta)
{
    HAL_StatusTypeDef status;

    if (meta == NULL) {
        return HAL_ERROR;
    }

    // Update checksum before saving
    AutomotiveOtaMetadata_t temp_meta = *meta;
    temp_meta.checksum = CalculateAutomotiveChecksum(meta);

    // Unlock flash
    HAL_FLASH_Unlock();

    // Erase metadata sector
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error;

    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = OTA_METADATA_ADDRESS;
    erase_init.NbPages = OTA_METADATA_SIZE / FLASH_PAGE_SIZE;

    status = HAL_FLASHEx_Erase(&erase_init, &page_error);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return status;
    }

    // Write metadata to flash
    uint32_t *src = (uint32_t*)&temp_meta;
    uint32_t address = OTA_METADATA_ADDRESS;

    for (uint32_t i = 0; i < AUTOMOTIVE_META_SIZE / 4; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *src);
        if (status != HAL_OK) {
            break;
        }
        src++;
        address += 4;
    }

    HAL_FLASH_Lock();
    return status;
}

/**
  * @brief  Clear automotive OTA metadata (reset to defaults)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_AutomotiveMeta_Clear(void)
{
    AutomotiveOtaMetadata_t default_meta;
    InitializeAutomotiveDefaults(&default_meta);
    return BSP_AutomotiveMeta_Save(&default_meta);
}

/**
  * @brief  Log diagnostic event to automotive metadata
  * @param  event: Diagnostic event type
  * @param  data: Event-specific data
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_AutomotiveMeta_LogEvent(DiagnosticEvent_t event, uint32_t data)
{
    AutomotiveOtaMetadata_t meta;

    // Load current metadata
    if (!BSP_AutomotiveMeta_Load(&meta)) {
        return HAL_ERROR;
    }

    // Update statistics based on event type
    switch (event) {
        case DIAG_EVENT_POWER_CYCLE:
            meta.power_cycles++;
            break;
        case DIAG_EVENT_WATCHDOG_RESET:
            meta.watchdog_resets++;
            break;
        case DIAG_EVENT_UNEXPECTED_RESET:
            meta.unexpected_resets++;
            break;
        case DIAG_EVENT_THERMAL_EVENT:
            meta.thermal_events++;
            break;
        case DIAG_EVENT_CAN_ERROR:
            meta.can_bus_errors++;
            break;
        case DIAG_EVENT_OVER_VOLTAGE:
            meta.over_voltage_events++;
            break;
        case DIAG_EVENT_UNDER_VOLTAGE:
            meta.under_voltage_events++;
            break;
        case DIAG_EVENT_EMC_EVENT:
            meta.emc_events++;
            break;
        default:
            break;
    }

    // Add to diagnostic log (circular buffer)
    uint32_t log_index = meta.diagnostic_log_index;
    meta.diagnostic_log[log_index].timestamp = HAL_GetTick();
    meta.diagnostic_log[log_index].event_type = event;
    meta.diagnostic_log[log_index].event_data = data;

    meta.diagnostic_log_index = (log_index + 1) % 16;
    meta.diagnostic_log_count++;

    // Save updated metadata
    return BSP_AutomotiveMeta_Save(&meta);
}

/**
  * @brief  Update upgrade statistics in automotive metadata
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_AutomotiveMeta_UpdateStatistics(void)
{
    AutomotiveOtaMetadata_t meta;

    // Load current metadata
    if (!BSP_AutomotiveMeta_Load(&meta)) {
        return HAL_ERROR;
    }

    // Update upgrade statistics based on current state
    if (meta.state == OTA_STATE_READY) {
        meta.successful_upgrades++;
        meta.last_successful_update = HAL_GetTick();
    } else if (meta.last_error != AUTOMOTIVE_ERROR_NONE) {
        meta.failed_upgrades++;
    }

    meta.total_upgrades = meta.successful_upgrades + meta.failed_upgrades;

    // Save updated metadata
    return BSP_AutomotiveMeta_Save(&meta);
}

/**
  * @brief  Check if current metadata is automotive-compatible
  * @retval true if compatible, false otherwise
  */
bool BSP_AutomotiveMeta_IsCompatible(void)
{
    // Check if the stored metadata size matches automotive version
    // This is a simple compatibility check
    volatile uint32_t magic = *((volatile uint32_t*)OTA_METADATA_ADDRESS);
    return (magic == OTA_METADATA_MAGIC);
}

/**
  * @brief  Migrate from legacy metadata to automotive format
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_AutomotiveMeta_MigrateFromLegacy(void)
{
    // Load legacy metadata
    OtaMetadata_t legacy_meta;
    memcpy(&legacy_meta, (const void*)OTA_METADATA_ADDRESS, sizeof(OtaMetadata_t));

    // Create automotive metadata with legacy data
    AutomotiveOtaMetadata_t automotive_meta;
    InitializeAutomotiveDefaults(&automotive_meta);

    // Copy legacy fields
    automotive_meta.magic = legacy_meta.magic;
    automotive_meta.format_version = legacy_meta.format_version;
    automotive_meta.active_bank = legacy_meta.active_bank;
    automotive_meta.staged_bank = legacy_meta.staged_bank;
    automotive_meta.staged_size = legacy_meta.staged_size;
    automotive_meta.staged_crc = legacy_meta.staged_crc;
    automotive_meta.staged_version = legacy_meta.staged_version;
    automotive_meta.state = legacy_meta.state;
    automotive_meta.last_error = legacy_meta.last_error;

    // Copy reserved fields
    memcpy(automotive_meta.reserved, legacy_meta.reserved,
           sizeof(legacy_meta.reserved));

    // Save automotive metadata
    return BSP_AutomotiveMeta_Save(&automotive_meta);
}

/**
  * @brief  Validate automotive metadata integrity
  * @param  meta: Pointer to automotive metadata
  * @retval true if valid, false otherwise
  */
HAL_StatusTypeDef BSP_AutomotiveMeta_ValidateIntegrity(const AutomotiveOtaMetadata_t *meta)
{
    if (meta == NULL) {
        return HAL_ERROR;
    }

    // Check magic number
    if (meta->magic != OTA_METADATA_MAGIC) {
        return HAL_ERROR;
    }

    // Check format version
    if (meta->format_version != OTA_METADATA_FORMAT_VERSION) {
        return HAL_ERROR;
    }

    // Validate checksum
    if (!ValidateAutomotiveChecksum(meta)) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Calculate checksum for automotive metadata
  * @param  meta: Pointer to automotive metadata
  * @retval Calculated checksum
  */
static uint32_t CalculateAutomotiveChecksum(const AutomotiveOtaMetadata_t *meta)
{
    if (meta == NULL) {
        return 0;
    }

    uint32_t checksum = AUTOMOTIVE_CHECKSUM_SEED;
    const uint32_t *data = (const uint32_t*)meta;

    // Calculate checksum over all fields except the checksum field itself
    uint32_t size = AUTOMOTIVE_META_SIZE - sizeof(uint32_t); // Exclude checksum field
    uint32_t words = size / 4;

    for (uint32_t i = 0; i < words; i++) {
        checksum ^= data[i];
        checksum = (checksum << 1) | (checksum >> 31); // Rotate left
    }

    return checksum;
}

/**
  * @brief  Validate automotive metadata checksum
  * @param  meta: Pointer to automotive metadata
  * @retval true if checksum is valid, false otherwise
  */
static bool ValidateAutomotiveChecksum(const AutomotiveOtaMetadata_t *meta)
{
    if (meta == NULL) {
        return false;
    }

    uint32_t calculated = CalculateAutomotiveChecksum(meta);
    return (calculated == meta->checksum);
}

/**
  * @brief  Initialize automotive metadata with default values
  * @param  meta: Pointer to automotive metadata structure
  */
static void InitializeAutomotiveDefaults(AutomotiveOtaMetadata_t *meta)
{
    if (meta == NULL) {
        return;
    }

    memset(meta, 0, sizeof(AutomotiveOtaMetadata_t));

    // Set magic and version
    meta->magic = OTA_METADATA_MAGIC;
    meta->format_version = OTA_METADATA_FORMAT_VERSION;

    // Default bank configuration
    meta->active_bank = 0;
    meta->staged_bank = OTA_BANK_INVALID;

    // Default state
    meta->state = OTA_STATE_IDLE;
    meta->last_error = AUTOMOTIVE_ERROR_NONE;

    // Default thresholds (automotive-grade)
    meta->min_voltage_threshold = 10000;     // 10V minimum
    meta->max_voltage_threshold = 16000;     // 16V maximum
    meta->max_temperature_threshold = 85;    // 85°C maximum
    meta->max_vibration_threshold = 20;      // 2g maximum (scaled)

    // Default authorization level (service level for initial setup)
    meta->authorization_level = 1;  // Service level

    // Set checksum
    meta->checksum = CalculateAutomotiveChecksum(meta);
}
