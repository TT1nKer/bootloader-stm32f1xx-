/**
  ******************************************************************************
  * @file    bsp_ota_meta_automotive.h
  * @brief   Automotive-grade OTA metadata extensions for STM32F103 Bootloader
  ******************************************************************************
  * @attention
  *
  * This file extends the existing OtaMetadata_t with automotive-grade features
  * while maintaining backward compatibility with the original implementation.
  *
  * Copyright (c) 2025 TT1nker (GitHub: TT1nker)
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  ******************************************************************************
  */

#ifndef __BSP_OTA_META_AUTOMOTIVE_H
#define __BSP_OTA_META_AUTOMOTIVE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "bootloader_config.h"
#include "bsp_ota_meta.h"  // Include original metadata structure

/* Automotive-grade error codes (extend existing OTA_ERROR_*) */
typedef enum
{
    // Keep original errors for compatibility
    AUTOMOTIVE_ERROR_NONE = OTA_ERROR_NONE,
    AUTOMOTIVE_ERROR_INVALID_IMAGE = OTA_ERROR_INVALID_IMAGE,
    AUTOMOTIVE_ERROR_CRC_MISMATCH = OTA_ERROR_CRC_MISMATCH,
    AUTOMOTIVE_ERROR_STORAGE_FAIL = OTA_ERROR_STORAGE_FAIL,

    // Automotive-specific errors
    AUTOMOTIVE_ERROR_POWER_FAILURE = 10,      // Power supply failure during upgrade
    AUTOMOTIVE_ERROR_ENVIRONMENTAL = 11,      // Environmental conditions unsafe
    AUTOMOTIVE_ERROR_AUTHORIZATION = 12,      // Authorization failure
    AUTOMOTIVE_ERROR_SECURITY = 13,           // Security verification failure
    AUTOMOTIVE_ERROR_COMMUNICATION = 14,      // Communication failure
    AUTOMOTIVE_ERROR_THERMAL = 15,            // Thermal protection triggered
    AUTOMOTIVE_ERROR_VIBRATION = 16,          // Excessive vibration detected
    AUTOMOTIVE_ERROR_EMC = 17                 // EMC interference detected
} AutomotiveError_t;

/* Automotive-grade diagnostic events */
typedef enum
{
    DIAG_EVENT_POWER_CYCLE = 0,
    DIAG_EVENT_WATCHDOG_RESET = 1,
    DIAG_EVENT_UNEXPECTED_RESET = 2,
    DIAG_EVENT_THERMAL_EVENT = 3,
    DIAG_EVENT_CAN_ERROR = 4,
    DIAG_EVENT_BLE_ERROR = 5,
    DIAG_EVENT_RF_ERROR = 6,
    DIAG_EVENT_OVER_VOLTAGE = 7,
    DIAG_EVENT_UNDER_VOLTAGE = 8,
    DIAG_EVENT_EMC_EVENT = 9
} DiagnosticEvent_t;

/* Extended metadata structure with automotive features */
typedef struct
{
    /* ===== Original OtaMetadata_t fields (for compatibility) ===== */
    uint32_t magic;                      // OTA_METADATA_MAGIC
    uint32_t format_version;             // OTA_METADATA_FORMAT_VERSION
    uint32_t active_bank;                // Current active bank (0 or 1)
    uint32_t staged_bank;                // Bank containing new firmware
    uint32_t staged_size;                // Bytes written to staged bank
    uint32_t staged_crc;                 // CRC32 of staged firmware
    uint32_t staged_version;             // Firmware version
    uint32_t state;                      // OtaState_t
    uint32_t last_error;                 // AutomotiveError_t
    uint32_t reserved[5];                // Reserved for future use

    /* ===== Automotive-grade extensions ===== */

    // Vehicle identification (17-character VIN stored as 4x 32-bit words)
    uint32_t vehicle_vin[4];             // Vehicle identification number
    uint32_t ecu_serial_number;          // ECU serial number

    // Manufacturer and authorization
    uint32_t manufacturer_id;            // Manufacturer identifier
    uint32_t authorization_level;        // Authorization level (1=Service, 2=OEM)
    uint32_t authorized_dealer_id;       // Authorized dealer identifier

    // Installation and maintenance
    uint32_t install_date;               // Installation date (Unix timestamp)
    uint32_t warranty_expiry;            // Warranty expiry date
    uint32_t last_service_date;          // Last service date

    /* ===== Diagnostic and statistics ===== */

    // Upgrade statistics
    uint32_t total_upgrades;             // Total number of upgrades performed
    uint32_t successful_upgrades;        // Number of successful upgrades
    uint32_t failed_upgrades;            // Number of failed upgrades
    uint32_t last_successful_update;     // Timestamp of last successful update

    // System reliability statistics
    uint32_t power_cycles;               // Number of power cycles
    uint32_t watchdog_resets;            // Number of watchdog resets
    uint32_t unexpected_resets;          // Number of unexpected resets
    uint32_t thermal_events;             // Number of thermal events

    // Communication statistics
    uint32_t can_bus_errors;             // CAN bus error count
    uint32_t ble_connection_count;       // BLE connection attempts
    uint32_t rf_retransmission_count;    // RF retransmission count

    // Environmental statistics
    uint32_t over_voltage_events;        // Over-voltage event count
    uint32_t under_voltage_events;       // Under-voltage event count
    uint32_t emc_events;                 // EMC interference event count
    uint32_t vibration_events;           // Vibration event count

    // Security statistics
    uint32_t authorization_failures;     // Authorization failure count
    uint32_t signature_verification_failures; // Signature verification failures
    uint32_t tampering_attempts;         // Detected tampering attempts

    // Environmental thresholds (configurable)
    uint32_t min_voltage_threshold;      // Minimum voltage threshold (mV)
    uint32_t max_voltage_threshold;      // Maximum voltage threshold (mV)
    uint32_t max_temperature_threshold;  // Maximum temperature threshold (°C)
    uint32_t max_vibration_threshold;    // Maximum vibration threshold (g)

    // Diagnostic log (circular buffer - last 16 entries)
    struct {
        uint32_t timestamp;              // Event timestamp
        uint32_t event_type;             // DiagnosticEvent_t
        uint32_t event_data;             // Event-specific data
    } diagnostic_log[16];

    uint32_t diagnostic_log_index;       // Current log entry index
    uint32_t diagnostic_log_count;       // Total entries in log

    /* ===== Security extensions ===== */
    uint8_t security_key_hash[32];       // Hash of current security key
    uint32_t security_version;           // Security protocol version
    uint32_t last_security_update;       // Last security update timestamp

    uint32_t checksum;                   // Checksum (must be last field)
} AutomotiveOtaMetadata_t;

/* Static assertions for compatibility */
_Static_assert(offsetof(AutomotiveOtaMetadata_t, magic) == offsetof(OtaMetadata_t, magic),
               "Automotive metadata must maintain compatibility with original structure");
_Static_assert(offsetof(AutomotiveOtaMetadata_t, checksum) > offsetof(OtaMetadata_t, checksum),
               "Automotive metadata must extend original structure");

/* Function declarations */
bool BSP_AutomotiveMeta_Load(AutomotiveOtaMetadata_t *meta);
HAL_StatusTypeDef BSP_AutomotiveMeta_Save(const AutomotiveOtaMetadata_t *meta);
HAL_StatusTypeDef BSP_AutomotiveMeta_Clear(void);
HAL_StatusTypeDef BSP_AutomotiveMeta_LogEvent(DiagnosticEvent_t event, uint32_t data);
HAL_StatusTypeDef BSP_AutomotiveMeta_UpdateStatistics(void);

/* Compatibility macros */
#define BSP_OtaMeta_Load(meta) BSP_AutomotiveMeta_Load((AutomotiveOtaMetadata_t*)(meta))
#define BSP_OtaMeta_Save(meta) BSP_AutomotiveMeta_Save((const AutomotiveOtaMetadata_t*)(meta))

/* Utility functions */
bool BSP_AutomotiveMeta_IsCompatible(void);  // Check if current metadata is automotive-compatible
HAL_StatusTypeDef BSP_AutomotiveMeta_MigrateFromLegacy(void);  // Migrate from legacy metadata
HAL_StatusTypeDef BSP_AutomotiveMeta_ValidateIntegrity(const AutomotiveOtaMetadata_t *meta);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_OTA_META_AUTOMOTIVE_H */
