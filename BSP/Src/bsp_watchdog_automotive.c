/**
  ******************************************************************************
  * @file    bsp_watchdog_automotive.c
  * @brief   Automotive-grade multi-watchdog protection implementation
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
#include "bsp_watchdog_automotive.h"
#include "bsp_ota_meta_automotive.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define WATCHDOG_DEFAULT_SYSTEM_TIMEOUT      3000    // 3 seconds for system
#define WATCHDOG_DEFAULT_UPGRADE_TIMEOUT     30000   // 30 seconds for upgrades
#define WATCHDOG_DEFAULT_COMM_TIMEOUT        5000    // 5 seconds for communication

/* Private variables ---------------------------------------------------------*/
static IWDG_HandleTypeDef hiwdg;
static WatchdogConfig_t watchdog_configs[WATCHDOG_COUNT];
static ProtectionLevel_t current_protection_level = PROTECTION_LEVEL_AUTOMOTIVE;
static bool upgrade_mode_active = false;

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef IWDG_Init(uint32_t timeout_ms);
static void Watchdog_CheckTimeouts(void);
static void Watchdog_HandleTrigger(WatchdogType_t type);
static uint32_t TimeoutMsToIWDGValue(uint32_t timeout_ms);

/**
  * @brief  Initialize automotive watchdog system
  * @param  level: Protection level to initialize
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_Init(ProtectionLevel_t level)
{
    HAL_StatusTypeDef status = HAL_OK;

    // Initialize watchdog configurations
    memset(watchdog_configs, 0, sizeof(watchdog_configs));

    // Set protection level
    current_protection_level = level;

    // Initialize based on protection level
    switch (level) {
        case PROTECTION_LEVEL_AUTOMOTIVE:
            // Initialize all watchdogs
            status |= BSP_WatchdogAutomotive_Start(WATCHDOG_SYSTEM,
                                                  WATCHDOG_DEFAULT_SYSTEM_TIMEOUT);
            status |= BSP_WatchdogAutomotive_Start(WATCHDOG_UPGRADE,
                                                  WATCHDOG_DEFAULT_UPGRADE_TIMEOUT);
            status |= BSP_WatchdogAutomotive_Start(WATCHDOG_COMMUNICATION,
                                                  WATCHDOG_DEFAULT_COMM_TIMEOUT);
            break;

        case PROTECTION_LEVEL_STANDARD:
            // Initialize system and upgrade watchdogs
            status |= BSP_WatchdogAutomotive_Start(WATCHDOG_SYSTEM,
                                                  WATCHDOG_DEFAULT_SYSTEM_TIMEOUT);
            status |= BSP_WatchdogAutomotive_Start(WATCHDOG_UPGRADE,
                                                  WATCHDOG_DEFAULT_UPGRADE_TIMEOUT);
            break;

        case PROTECTION_LEVEL_BASIC:
        default:
            // Initialize only system watchdog
            status |= BSP_WatchdogAutomotive_Start(WATCHDOG_SYSTEM,
                                                  WATCHDOG_DEFAULT_SYSTEM_TIMEOUT);
            break;
    }

    return status;
}

/**
  * @brief  Deinitialize automotive watchdog system
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_DeInit(void)
{
    // Stop all watchdogs
    for (WatchdogType_t type = 0; type < WATCHDOG_COUNT; type++) {
        BSP_WatchdogAutomotive_Stop(type);
    }

    return HAL_OK;
}

/**
  * @brief  Start specific watchdog timer
  * @param  type: Watchdog type to start
  * @param  timeout_ms: Timeout period in milliseconds
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_Start(WatchdogType_t type, uint32_t timeout_ms)
{
    if (type >= WATCHDOG_COUNT) {
        return HAL_ERROR;
    }

    // Initialize IWDG if not already done
    if (hiwdg.Instance == NULL) {
        HAL_StatusTypeDef status = IWDG_Init(timeout_ms);
        if (status != HAL_OK) {
            return status;
        }
    }

    // Configure watchdog
    watchdog_configs[type].timeout_ms = timeout_ms;
    watchdog_configs[type].last_feed_time = HAL_GetTick();
    watchdog_configs[type].state = WATCHDOG_STATE_ENABLED;
    watchdog_configs[type].trigger_count = 0;
    watchdog_configs[type].auto_feed_enabled = false;

    return HAL_OK;
}

/**
  * @brief  Stop specific watchdog timer
  * @param  type: Watchdog type to stop
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_Stop(WatchdogType_t type)
{
    if (type >= WATCHDOG_COUNT) {
        return HAL_ERROR;
    }

    watchdog_configs[type].state = WATCHDOG_STATE_DISABLED;
    return HAL_OK;
}

/**
  * @brief  Feed specific watchdog timer
  * @param  type: Watchdog type to feed
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_Feed(WatchdogType_t type)
{
    if (type >= WATCHDOG_COUNT) {
        return HAL_ERROR;
    }

    if (watchdog_configs[type].state != WATCHDOG_STATE_ENABLED) {
        return HAL_ERROR;
    }

    watchdog_configs[type].last_feed_time = HAL_GetTick();

    // For system watchdog, also feed hardware IWDG
    if (type == WATCHDOG_SYSTEM) {
        HAL_IWDG_Refresh(&hiwdg);
    }

    return HAL_OK;
}

/**
  * @brief  Feed system watchdog
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_FeedSystem(void)
{
    return BSP_WatchdogAutomotive_Feed(WATCHDOG_SYSTEM);
}

/**
  * @brief  Feed upgrade watchdog
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_FeedUpgrade(void)
{
    return BSP_WatchdogAutomotive_Feed(WATCHDOG_UPGRADE);
}

/**
  * @brief  Feed communication watchdog
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_FeedCommunication(void)
{
    return BSP_WatchdogAutomotive_Feed(WATCHDOG_COMMUNICATION);
}

/**
  * @brief  Check if specific watchdog has been triggered
  * @param  type: Watchdog type to check
  * @retval true if triggered, false otherwise
  */
bool BSP_WatchdogAutomotive_IsTriggered(WatchdogType_t type)
{
    if (type >= WATCHDOG_COUNT) {
        return false;
    }

    return (watchdog_configs[type].state == WATCHDOG_STATE_TRIGGERED);
}

/**
  * @brief  Get state of specific watchdog
  * @param  type: Watchdog type
  * @retval WatchdogState_t
  */
WatchdogState_t BSP_WatchdogAutomotive_GetState(WatchdogType_t type)
{
    if (type >= WATCHDOG_COUNT) {
        return WATCHDOG_STATE_DISABLED;
    }

    return watchdog_configs[type].state;
}

/**
  * @brief  Get trigger count for specific watchdog
  * @param  type: Watchdog type
  * @retval Trigger count
  */
uint32_t BSP_WatchdogAutomotive_GetTriggerCount(WatchdogType_t type)
{
    if (type >= WATCHDOG_COUNT) {
        return 0;
    }

    return watchdog_configs[type].trigger_count;
}

/**
  * @brief  Set protection level
  * @param  level: New protection level
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_SetProtectionLevel(ProtectionLevel_t level)
{
    if (level > PROTECTION_LEVEL_AUTOMOTIVE) {
        return HAL_ERROR;
    }

    // Stop current configuration
    BSP_WatchdogAutomotive_DeInit();

    // Start new configuration
    return BSP_WatchdogAutomotive_Init(level);
}

/**
  * @brief  Get current protection level
  * @retval ProtectionLevel_t
  */
ProtectionLevel_t BSP_WatchdogAutomotive_GetProtectionLevel(void)
{
    return current_protection_level;
}

/**
  * @brief  Enable upgrade mode (extended timeouts)
  */
void BSP_WatchdogAutomotive_UpgradeMode_Enable(void)
{
    upgrade_mode_active = true;

    // Extend upgrade watchdog timeout during upgrade
    if (watchdog_configs[WATCHDOG_UPGRADE].state == WATCHDOG_STATE_ENABLED) {
        BSP_WatchdogAutomotive_Start(WATCHDOG_UPGRADE, 60000); // 60 seconds during upgrade
    }
}

/**
  * @brief  Disable upgrade mode (restore normal timeouts)
  */
void BSP_WatchdogAutomotive_UpgradeMode_Disable(void)
{
    upgrade_mode_active = false;

    // Restore normal upgrade watchdog timeout
    if (watchdog_configs[WATCHDOG_UPGRADE].state == WATCHDOG_STATE_ENABLED) {
        BSP_WatchdogAutomotive_Start(WATCHDOG_UPGRADE, WATCHDOG_DEFAULT_UPGRADE_TIMEOUT);
    }
}

/**
  * @brief  Check if upgrade mode is active
  * @retval true if upgrade mode is active, false otherwise
  */
bool BSP_WatchdogAutomotive_IsUpgradeMode(void)
{
    return upgrade_mode_active;
}

/**
  * @brief  Get watchdog statistics
  * @param  type: Watchdog type
  * @param  trigger_count: Pointer to store trigger count
  * @param  last_feed_time: Pointer to store last feed time
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_WatchdogAutomotive_GetStatistics(WatchdogType_t type,
                                                      uint32_t *trigger_count,
                                                      uint32_t *last_feed_time)
{
    if (type >= WATCHDOG_COUNT || trigger_count == NULL || last_feed_time == NULL) {
        return HAL_ERROR;
    }

    *trigger_count = watchdog_configs[type].trigger_count;
    *last_feed_time = watchdog_configs[type].last_feed_time;

    return HAL_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize IWDG hardware
  * @param  timeout_ms: Timeout period in milliseconds
  * @retval HAL_StatusTypeDef
  */
static HAL_StatusTypeDef IWDG_Init(uint32_t timeout_ms)
{
    // Calculate IWDG parameters based on timeout
    uint32_t iwdg_value = TimeoutMsToIWDGValue(timeout_ms);

    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
    hiwdg.Init.Reload = iwdg_value;

    return HAL_IWDG_Init(&hiwdg);
}

/**
  * @brief  Check for watchdog timeouts
  * @note   This function should be called periodically from main loop
  */
static void Watchdog_CheckTimeouts(void)
{
    uint32_t current_time = HAL_GetTick();

    for (WatchdogType_t type = 0; type < WATCHDOG_COUNT; type++) {
        if (watchdog_configs[type].state == WATCHDOG_STATE_ENABLED) {
            uint32_t time_since_feed = current_time - watchdog_configs[type].last_feed_time;

            if (time_since_feed >= watchdog_configs[type].timeout_ms) {
                Watchdog_HandleTrigger(type);
            }
        }
    }
}

/**
  * @brief  Handle watchdog trigger event
  * @param  type: Watchdog type that triggered
  */
static void Watchdog_HandleTrigger(WatchdogType_t type)
{
    // Mark as triggered
    watchdog_configs[type].state = WATCHDOG_STATE_TRIGGERED;
    watchdog_configs[type].trigger_count++;

    // Log diagnostic event
    DiagnosticEvent_t diag_event = DIAG_EVENT_WATCHDOG_RESET;
    BSP_AutomotiveMeta_LogEvent(diag_event, (uint32_t)type);

    // Handle based on watchdog type
    switch (type) {
        case WATCHDOG_SYSTEM:
            // System watchdog triggered - critical failure
            // Hardware IWDG will reset the system
            break;

        case WATCHDOG_UPGRADE:
            // Upgrade watchdog triggered - abort upgrade
            if (upgrade_mode_active) {
                // TODO: Integrate with OTA abort mechanism
                // OTA_AbortUpgrade();
            }
            break;

        case WATCHDOG_COMMUNICATION:
            // Communication watchdog triggered - reset communication
            // TODO: Reset communication interfaces
            break;

        default:
            break;
    }
}

/**
  * @brief  Convert timeout in milliseconds to IWDG reload value
  * @param  timeout_ms: Timeout in milliseconds
  * @retval IWDG reload value
  */
static uint32_t TimeoutMsToIWDGValue(uint32_t timeout_ms)
{
    // IWDG frequency is LSI/256 (approx 40kHz/256 = 156.25 Hz)
    // Timeout = (Reload + 1) / (LSI/256)
    // Reload = (Timeout * LSI / 256) - 1

    const uint32_t LSI_FREQ = 40000; // 40kHz typical
    const uint32_t PRESCALER = 256;

    uint32_t reload = (timeout_ms * LSI_FREQ / PRESCALER / 1000) - 1;

    // Clamp to valid range
    if (reload > 0xFFF) {
        reload = 0xFFF;
    }

    return reload;
}

/**
  * @brief  Periodic watchdog monitoring function
  * @note   This function should be called from the main loop
  */
void BSP_WatchdogAutomotive_Process(void)
{
    Watchdog_CheckTimeouts();
}
