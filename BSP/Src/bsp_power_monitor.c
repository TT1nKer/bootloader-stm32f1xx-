/**
  ******************************************************************************
  * @file    bsp_power_monitor.c
  * @brief   Automotive-grade power monitoring implementation
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
#include "bsp_power_monitor.h"
#include "bsp_ota_meta_automotive.h"
#include "bootloader_config.h"
#include <string.h>
#include <math.h>

/* Private variables ---------------------------------------------------------*/
static ADC_HandleTypeDef hadc1;
static PowerStatus_t current_power_status;
static uint32_t voltage_min_threshold = AUTOMOTIVE_MIN_VOLTAGE;
static uint32_t voltage_max_threshold = AUTOMOTIVE_MAX_VOLTAGE;
static uint32_t last_update_time = 0;

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef ADC_Init(void);
static uint32_t ADC_ReadAverage(uint8_t samples);
static float CalculateSystemVoltage(uint32_t adc_value);
static bool CheckVoltageStability(void);

/**
  * @brief  Initialize power monitoring system
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_PowerMonitor_Init(void)
{
#if ENABLE_POWER_MONITOR
    HAL_StatusTypeDef status;

    // Initialize ADC
    status = ADC_Init();
    if (status != HAL_OK) {
        return status;
    }

    // Initialize power status structure
    memset(&current_power_status, 0, sizeof(PowerStatus_t));
    current_power_status.timestamp = HAL_GetTick();
    current_power_status.power_stable = false;
    current_power_status.upgrade_safe = false;

    // Perform initial reading
    BSP_PowerMonitor_ReadStatus();

    return HAL_OK;
#else
    // Power monitor disabled - return OK but functionality unavailable
    memset(&current_power_status, 0, sizeof(PowerStatus_t));
    current_power_status.upgrade_safe = true;  // Assume safe if monitoring disabled
    return HAL_OK;
#endif
}

/**
  * @brief  Deinitialize power monitoring system
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_PowerMonitor_DeInit(void)
{
    if (hadc1.Instance != NULL) {
        HAL_ADC_DeInit(&hadc1);
    }

    return HAL_OK;
}

/**
  * @brief  Read current power status
  * @retval PowerStatus_t Current power status
  */
PowerStatus_t BSP_PowerMonitor_ReadStatus(void)
{
    uint32_t current_time = HAL_GetTick();

    // Update readings periodically
    if ((current_time - last_update_time) >= POWER_MONITOR_UPDATE_INTERVAL) {
        // Read ADC value
        current_power_status.raw_voltage = ADC_ReadAverage(POWER_MONITOR_SAMPLE_COUNT);

        // Calculate system voltage
        current_power_status.voltage_12v = CalculateSystemVoltage(current_power_status.raw_voltage);

        // Convert to millivolts for comparison
        uint32_t voltage_mv = (uint32_t)(current_power_status.voltage_12v * 1000.0f);

        // Check voltage range
        current_power_status.voltage_in_range =
            (voltage_mv >= voltage_min_threshold) && (voltage_mv <= voltage_max_threshold);

        // Check stability
        current_power_status.power_stable = CheckVoltageStability();

        // Determine upgrade safety
        current_power_status.upgrade_safe =
            current_power_status.voltage_in_range && current_power_status.power_stable;

        // Update timestamp
        current_power_status.timestamp = current_time;
        last_update_time = current_time;
    }

    return current_power_status;
}

/**
  * @brief  Check if system is safe for firmware upgrade
  * @retval true if safe for upgrade, false otherwise
  */
bool BSP_PowerMonitor_IsUpgradeSafe(void)
{
    PowerStatus_t status = BSP_PowerMonitor_ReadStatus();
    return status.upgrade_safe;
}

/**
  * @brief  Check upgrade conditions (stricter than regular safety check)
  * @retval true if conditions are met for upgrade, false otherwise
  */
bool BSP_PowerMonitor_CheckUpgradeConditions(void)
{
    PowerStatus_t status = BSP_PowerMonitor_ReadStatus();

    // Stricter check for upgrades - require stable power for longer period
    if (!status.upgrade_safe) {
        return false;
    }

    // Check if voltage has been stable for required period
    uint32_t stable_time = POWER_MONITOR_STABILITY_TIME;
    if (status.stability_counter < (stable_time / POWER_MONITOR_UPDATE_INTERVAL)) {
        return false;
    }

    return true;
}

/**
  * @brief  Power monitoring protection during upgrade
  * @note   This function should be called regularly during firmware upgrades
  */
void BSP_PowerMonitor_UpgradeProtection(void)
{
    static uint32_t last_check = 0;
    uint32_t current_time = HAL_GetTick();

    // Check every 1 second during upgrade
    if ((current_time - last_check) >= 1000) {
        if (!BSP_PowerMonitor_IsUpgradeSafe()) {
            // Log power failure event
            BSP_PowerMonitor_LogEvent(POWER_EVENT_UNSTABLE);

            // TODO: Integrate with OTA abort mechanism
            // OTA_AbortUpgrade();
            // Log_PowerFailure();
        }
        last_check = current_time;
    }
}

/**
  * @brief  Log power-related event
  * @param  event: Power event type
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_PowerMonitor_LogEvent(PowerEvent_t event)
{
    // Convert power event to diagnostic event
    DiagnosticEvent_t diag_event;

    switch (event) {
        case POWER_EVENT_LOW_VOLTAGE:
            diag_event = DIAG_EVENT_UNDER_VOLTAGE;
            break;
        case POWER_EVENT_HIGH_VOLTAGE:
            diag_event = DIAG_EVENT_OVER_VOLTAGE;
            break;
        case POWER_EVENT_CRITICAL_LOW:
        case POWER_EVENT_CRITICAL_HIGH:
        case POWER_EVENT_UNSTABLE:
            diag_event = DIAG_EVENT_POWER_CYCLE;  // Use as generic power event
            break;
        default:
            diag_event = DIAG_EVENT_POWER_CYCLE;
            break;
    }

    // Log to automotive metadata
    return BSP_AutomotiveMeta_LogEvent(diag_event, (uint32_t)event);
}

/**
  * @brief  Get current system voltage
  * @retval Current voltage in volts
  */
float BSP_PowerMonitor_GetVoltage(void)
{
    BSP_PowerMonitor_ReadStatus();
    return current_power_status.voltage_12v;
}

/**
  * @brief  Check if power supply is stable
  * @retval true if stable, false otherwise
  */
bool BSP_PowerMonitor_IsStable(void)
{
    BSP_PowerMonitor_ReadStatus();
    return current_power_status.power_stable;
}

/**
  * @brief  Set voltage thresholds
  * @param  min_mv: Minimum voltage threshold (mV)
  * @param  max_mv: Maximum voltage threshold (mV)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_PowerMonitor_SetThresholds(uint32_t min_mv, uint32_t max_mv)
{
    if (min_mv >= max_mv) {
        return HAL_ERROR;
    }

    voltage_min_threshold = min_mv;
    voltage_max_threshold = max_mv;

    return HAL_OK;
}

/**
  * @brief  Get current voltage thresholds
  * @param  min_mv: Pointer to store minimum threshold
  * @param  max_mv: Pointer to store maximum threshold
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_PowerMonitor_GetThresholds(uint32_t *min_mv, uint32_t *max_mv)
{
    if (min_mv == NULL || max_mv == NULL) {
        return HAL_ERROR;
    }

    *min_mv = voltage_min_threshold;
    *max_mv = voltage_max_threshold;

    return HAL_OK;
}

/**
  * @brief  Convert ADC raw value to voltage
  * @param  adc_value: Raw ADC value (0-4095)
  * @retval Voltage in millivolts
  */
uint32_t BSP_PowerMonitor_ADCRawToVoltage(uint32_t adc_value)
{
    // Convert ADC reading to voltage using voltage divider
    float adc_voltage = ((float)adc_value * ADC_REFERENCE_VOLTAGE) / (float)ADC_MAX_VALUE;
    float system_voltage_mv = adc_voltage / VOLTAGE_DIVIDER_RATIO;

    return (uint32_t)system_voltage_mv;
}

/**
  * @brief  Convert voltage to ADC value
  * @param  voltage: Voltage in volts
  * @retval Expected ADC value
  */
uint32_t BSP_PowerMonitor_VoltageToADC(float voltage)
{
    // Convert voltage to expected ADC reading
    float adc_voltage = voltage * VOLTAGE_DIVIDER_RATIO;
    uint32_t adc_value = (adc_voltage * ADC_MAX_VALUE) / ADC_REFERENCE_VOLTAGE;

    return adc_value;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize ADC for power monitoring
  * @retval HAL_StatusTypeDef
  */
static HAL_StatusTypeDef ADC_Init(void)
{
    HAL_StatusTypeDef status;
    ADC_ChannelConfTypeDef sConfig = {0};

    // Configure ADC
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;

    status = HAL_ADC_Init(&hadc1);
    if (status != HAL_OK) {
        return status;
    }

    // Calibrate ADC (required for STM32F1)
    status = HAL_ADCEx_Calibration_Start(&hadc1);
    if (status != HAL_OK) {
        HAL_ADC_DeInit(&hadc1);
        return status;
    }

    // Configure ADC channel
    sConfig.Channel = POWER_MONITOR_ADC_CHANNEL;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    status = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    if (status != HAL_OK) {
        HAL_ADC_DeInit(&hadc1);
        return status;
    }

    // Perform a dummy read to ensure ADC is ready
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    HAL_ADC_Stop(&hadc1);

    return HAL_OK;
}

/**
  * @brief  Read average ADC value over multiple samples
  * @param  samples: Number of samples to average
  * @retval Average ADC value
  */
static uint32_t ADC_ReadAverage(uint8_t samples)
{
    uint32_t sum = 0;

    for (uint8_t i = 0; i < samples; i++) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
        sum += HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);

        // Small delay between samples
        HAL_Delay(1);
    }

    return sum / samples;
}

/**
  * @brief  Calculate system voltage from ADC reading
  * @param  adc_value: Raw ADC value
  * @retval System voltage in volts
  */
static float CalculateSystemVoltage(uint32_t adc_value)
{
    uint32_t voltage_mv = BSP_PowerMonitor_ADCRawToVoltage(adc_value);
    return voltage_mv / 1000.0f;
}

/**
  * @brief  Check voltage stability over time
  * @retval true if voltage is stable, false otherwise
  */
static bool CheckVoltageStability(void)
{
    static uint32_t stable_count = 0;
    static float last_voltage = 0.0f;
    static uint32_t last_stable_check = 0;

    uint32_t current_time = HAL_GetTick();
    float current_voltage = current_power_status.voltage_12v;

    // Check stability periodically
    if ((current_time - last_stable_check) >= POWER_MONITOR_UPDATE_INTERVAL) {
        // Calculate absolute difference (inline implementation to avoid math.h dependency)
        float voltage_diff = current_voltage - last_voltage;
        if (voltage_diff < 0.0f) {
            voltage_diff = -voltage_diff;
        }

        // Consider stable if voltage change is less than 0.5V
        if (voltage_diff < 0.5f) {
            stable_count++;
            // Require stability for several consecutive readings
            if (stable_count >= 5) {  // Stable for ~5 seconds
                current_power_status.stability_counter = stable_count;
                last_stable_check = current_time;
                return true;
            }
        } else {
            stable_count = 0;
        }

        last_voltage = current_voltage;
        last_stable_check = current_time;
    }

    return false;
}
