/**
  ******************************************************************************
  * @file    bsp_watchdog.c
  * @brief   Board Support Package - Watchdog implementation
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
#include "bsp_watchdog.h"

/* Private variables ---------------------------------------------------------*/
static IWDG_HandleTypeDef hiwdg;
static bool watchdog_initialized = false;

/* Private function prototypes -----------------------------------------------*/
static uint8_t CalculatePrescaler(uint32_t timeout_ms);
static uint16_t CalculateReload(uint32_t timeout_ms, uint8_t prescaler);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initialize Independent Watchdog
  * @param  timeout_ms: Timeout in milliseconds
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_Watchdog_Init(uint32_t timeout_ms)
{
    uint8_t prescaler;
    uint16_t reload;
    
    // Calculate optimal prescaler and reload values
    prescaler = CalculatePrescaler(timeout_ms);
    reload = CalculateReload(timeout_ms, prescaler);
    
    // Configure watchdog
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = prescaler;
    hiwdg.Init.Reload = reload;
    
    // Initialize watchdog
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        watchdog_initialized = false;
        return HAL_ERROR;
    }
    
    watchdog_initialized = true;
    return HAL_OK;
}

/**
  * @brief  Feed watchdog (reload counter)
  * @retval None
  */
void BSP_Watchdog_Feed(void)
{
    if (!watchdog_initialized)
    {
        return;
    }
    
    HAL_IWDG_Refresh(&hiwdg);
}

bool BSP_Watchdog_IsInitialized(void)
{
    return watchdog_initialized;
}

/* Private functions --------------------------------------------------------*/

/**
  * @brief  Calculate optimal prescaler for given timeout
  * @param  timeout_ms: Desired timeout in milliseconds
  * @retval Prescaler value
  */
static uint8_t CalculatePrescaler(uint32_t timeout_ms)
{
    // LSI frequency is approximately 40kHz (range: 30-60kHz)
    // Timeout = (RLR + 1) × (4 × 2^PR) / LSI_freq
    
    if (timeout_ms <= 409)
    {
        return IWDG_PRESCALER_4;      // 4分频
    }
    else if (timeout_ms <= 819)
    {
        return IWDG_PRESCALER_8;      // 8分频
    }
    else if (timeout_ms <= 1638)
    {
        return IWDG_PRESCALER_16;     // 16分频
    }
    else if (timeout_ms <= 3277)
    {
        return IWDG_PRESCALER_32;     // 32分频
    }
    else if (timeout_ms <= 6554)
    {
        return IWDG_PRESCALER_64;     // 64分频
    }
    else if (timeout_ms <= 13107)
    {
        return IWDG_PRESCALER_128;    // 128分频
    }
    else
    {
        return IWDG_PRESCALER_256;   // 256分频
    }
}

/**
  * @brief  Calculate reload value for given timeout and prescaler
  * @param  timeout_ms: Desired timeout in milliseconds
  * @param  prescaler: Prescaler value
  * @retval Reload value
  */
static uint16_t CalculateReload(uint32_t timeout_ms, uint8_t prescaler)
{
    // Formula: timeout_ms = (RLR + 1) × (4 × 2^PR) / LSI_freq × 1000
    // Assuming LSI = 40kHz
    // timeout_ms = (RLR + 1) × (2^PR) / 10
    // RLR = (timeout_ms × 10 / 2^PR) - 1
    
    uint32_t prescaler_value = 4 * (1 << (prescaler & 0x07));
    uint16_t reload = (timeout_ms * 40000) / (prescaler_value * 1000) - 1;
    
    // Limit to valid range
    if (reload > 0xFFF)
    {
        reload = 0xFFF;
    }
    
    return reload;
}



