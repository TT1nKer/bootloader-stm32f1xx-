/**
  ******************************************************************************
  * @file    bsp_power_monitor.h
  * @brief   Automotive-grade power monitoring for STM32F103 Bootloader
  ******************************************************************************
  * @attention
  *
  * This module provides power supply monitoring and protection for automotive
  * applications, ensuring safe OTA upgrades under varying voltage conditions.
  *
  * REQUIREMENTS:
  * - ADC must be enabled in stm32f1xx_hal_conf.h (#define HAL_ADC_MODULE_ENABLED)
  * - PA0 must be configured as analog input for voltage sensing
  *
  * Copyright (c) 2025 TT1nker (GitHub: TT1nker)
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  ******************************************************************************
  */

#ifndef __BSP_POWER_MONITOR_H
#define __BSP_POWER_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "main.h"

/* Power monitoring configuration */
#define POWER_MONITOR_ADC_CHANNEL     ADC_CHANNEL_0      // PA0 for voltage sensing
#define POWER_MONITOR_SAMPLE_COUNT    8                  // Number of ADC samples for averaging
#define POWER_MONITOR_UPDATE_INTERVAL 1000               // Update interval (ms)
#define POWER_MONITOR_STABILITY_TIME  5000               // Stability check time (ms)

/* Automotive voltage thresholds (in mV) */
#define AUTOMOTIVE_MIN_VOLTAGE        10000              // 10.0V minimum for upgrades
#define AUTOMOTIVE_MAX_VOLTAGE        16000              // 16.0V maximum for upgrades
#define AUTOMOTIVE_NOMINAL_VOLTAGE    12000              // 12.0V nominal
#define AUTOMOTIVE_LOW_WARNING        11000              // 11.0V warning threshold
#define AUTOMOTIVE_HIGH_WARNING       15000              // 15.0V warning threshold

/* Voltage divider configuration */
#define VOLTAGE_DIVIDER_RATIO         4.0f               // R1/(R1+R2) ratio
#define ADC_REFERENCE_VOLTAGE         3300               // ADC reference voltage (mV)
#define ADC_MAX_VALUE                 4095               // 12-bit ADC maximum value

/* Power status structure */
typedef struct {
    uint32_t timestamp;              // Last update timestamp
    uint32_t raw_voltage;            // Raw ADC reading
    float voltage_12v;              // Calculated 12V system voltage (V)
    float voltage_3v3;              // 3.3V system voltage (V)
    float current_consumption;      // Current consumption (mA) - if available
    bool power_stable;              // Power stability flag
    bool voltage_in_range;          // Voltage within safe range
    bool upgrade_safe;              // Safe for firmware upgrade
    uint32_t stability_counter;     // Stability monitoring counter
} PowerStatus_t;

/* Power event types */
typedef enum {
    POWER_EVENT_NORMAL = 0,
    POWER_EVENT_LOW_VOLTAGE = 1,
    POWER_EVENT_HIGH_VOLTAGE = 2,
    POWER_EVENT_UNSTABLE = 3,
    POWER_EVENT_CRITICAL_LOW = 4,
    POWER_EVENT_CRITICAL_HIGH = 5
} PowerEvent_t;

/* Function declarations */
HAL_StatusTypeDef BSP_PowerMonitor_Init(void);
HAL_StatusTypeDef BSP_PowerMonitor_DeInit(void);
PowerStatus_t BSP_PowerMonitor_ReadStatus(void);
bool BSP_PowerMonitor_IsUpgradeSafe(void);
bool BSP_PowerMonitor_CheckUpgradeConditions(void);
void BSP_PowerMonitor_UpgradeProtection(void);
HAL_StatusTypeDef BSP_PowerMonitor_LogEvent(PowerEvent_t event);
float BSP_PowerMonitor_GetVoltage(void);
bool BSP_PowerMonitor_IsStable(void);

/* ADC conversion utilities */
uint32_t BSP_PowerMonitor_ADCRawToVoltage(uint32_t adc_value);
uint32_t BSP_PowerMonitor_VoltageToADC(float voltage);

/* Configuration functions */
HAL_StatusTypeDef BSP_PowerMonitor_SetThresholds(uint32_t min_mv, uint32_t max_mv);
HAL_StatusTypeDef BSP_PowerMonitor_GetThresholds(uint32_t *min_mv, uint32_t *max_mv);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_POWER_MONITOR_H */
