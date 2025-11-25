/**
  ******************************************************************************
  * @file    bsp_watchdog_automotive.h
  * @brief   Automotive-grade multi-watchdog protection system
  ******************************************************************************
  * @attention
  *
  * This module provides multiple watchdog timers for different protection levels
  * in automotive applications, ensuring system reliability during OTA upgrades.
  *
  * Copyright (c) 2025 TT1nker (GitHub: TT1nker)
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  ******************************************************************************
  */

#ifndef __BSP_WATCHDOG_AUTOMOTIVE_H
#define __BSP_WATCHDOG_AUTOMOTIVE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "main.h"

/* Watchdog types for different protection levels */
typedef enum {
    WATCHDOG_SYSTEM = 0,           // System-level watchdog (3s timeout)
    WATCHDOG_UPGRADE = 1,          // Upgrade-specific watchdog (30s timeout)
    WATCHDOG_COMMUNICATION = 2,    // Communication timeout watchdog (5s timeout)
    WATCHDOG_COUNT = 3             // Total number of watchdogs
} WatchdogType_t;

/* Watchdog states */
typedef enum {
    WATCHDOG_STATE_DISABLED = 0,
    WATCHDOG_STATE_ENABLED = 1,
    WATCHDOG_STATE_TRIGGERED = 2
} WatchdogState_t;

/* Watchdog configuration */
typedef struct {
    uint32_t timeout_ms;           // Timeout period in milliseconds
    uint32_t last_feed_time;       // Last feed timestamp
    WatchdogState_t state;         // Current state
    bool auto_feed_enabled;        // Auto-feed during certain operations
    uint32_t trigger_count;        // Number of times triggered
} WatchdogConfig_t;

/* System protection levels */
typedef enum {
    PROTECTION_LEVEL_BASIC = 0,    // Basic system protection only
    PROTECTION_LEVEL_STANDARD = 1, // Standard protection with upgrade monitoring
    PROTECTION_LEVEL_AUTOMOTIVE = 2 // Full automotive-grade protection
} ProtectionLevel_t;

/* Function declarations */
HAL_StatusTypeDef BSP_WatchdogAutomotive_Init(ProtectionLevel_t level);
HAL_StatusTypeDef BSP_WatchdogAutomotive_DeInit(void);

HAL_StatusTypeDef BSP_WatchdogAutomotive_Start(WatchdogType_t type, uint32_t timeout_ms);
HAL_StatusTypeDef BSP_WatchdogAutomotive_Stop(WatchdogType_t type);
HAL_StatusTypeDef BSP_WatchdogAutomotive_Feed(WatchdogType_t type);

HAL_StatusTypeDef BSP_WatchdogAutomotive_FeedSystem(void);
HAL_StatusTypeDef BSP_WatchdogAutomotive_FeedUpgrade(void);
HAL_StatusTypeDef BSP_WatchdogAutomotive_FeedCommunication(void);

bool BSP_WatchdogAutomotive_IsTriggered(WatchdogType_t type);
WatchdogState_t BSP_WatchdogAutomotive_GetState(WatchdogType_t type);
uint32_t BSP_WatchdogAutomotive_GetTriggerCount(WatchdogType_t type);

HAL_StatusTypeDef BSP_WatchdogAutomotive_SetProtectionLevel(ProtectionLevel_t level);
ProtectionLevel_t BSP_WatchdogAutomotive_GetProtectionLevel(void);

/* Upgrade-specific functions */
void BSP_WatchdogAutomotive_UpgradeMode_Enable(void);
void BSP_WatchdogAutomotive_UpgradeMode_Disable(void);
bool BSP_WatchdogAutomotive_IsUpgradeMode(void);

/* Diagnostic functions */
HAL_StatusTypeDef BSP_WatchdogAutomotive_GetStatistics(WatchdogType_t type,
                                                      uint32_t *trigger_count,
                                                      uint32_t *last_feed_time);

/* Compatibility functions (map to existing BSP functions) */
#define BSP_Watchdog_Init(timeout) BSP_WatchdogAutomotive_Start(WATCHDOG_SYSTEM, timeout)
#define BSP_Watchdog_Feed() BSP_WatchdogAutomotive_FeedSystem()

#ifdef __cplusplus
}
#endif

#endif /* __BSP_WATCHDOG_AUTOMOTIVE_H */
