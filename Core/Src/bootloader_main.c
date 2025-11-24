/**
  ******************************************************************************
  * @file    bootloader_main.c
  * @brief   Bootloader main program
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 TT1nker (GitHub: TT1nker)
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  * This software component is provided AS-IS, without any warranty of any kind.
  * User is responsible for the proper use of this software.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bootloader_config.h"
#include "bsp_flash.h"
#include "bsp_jump.h"
#include "bsp_ota_meta.h"
#include "bsp_upgrade_flag.h"
#include "bsp_watchdog.h"
#include "ota_transport.h"

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
void MX_GPIO_Init(void);
static bool TryPromoteStagedImage(OtaMetadata_t *meta, bool forced_upgrade);
static void PrepareUpgradeWindow(OtaMetadata_t *meta, bool reset_window);

/* Main function -------------------------------------------------------------*/

/**
  * @brief  Bootloader main function
  * @retval int (never returns)
  */
int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    
    /* Force set VTOR to bootloader address (prevent debugger from changing it) */
    SCB->VTOR = BOOTLOADER_START_ADDRESS;
    
    /* Initialize GPIO (for LED indication, etc.) */
    MX_GPIO_Init();
    
    /* Load OTA metadata and resolve desired action */
    OtaMetadata_t ota_meta;
    BSP_OtaMeta_Load(&ota_meta);
    
    UpgradeMode_t upgrade_mode = BSP_UpgradeFlag_Get();
    bool forced_upgrade = (upgrade_mode != UPGRADE_MODE_NONE);
    
    if (!forced_upgrade)
    {
        TryPromoteStagedImage(&ota_meta, false);
    }
    else
    {
        PrepareUpgradeWindow(&ota_meta, true);
        BSP_UpgradeFlag_Clear();
    }
    
    uint32_t active_bank_address = BSP_OtaMeta_GetBankStart(ota_meta.active_bank);
    bool active_valid = BSP_Jump_IsApplicationValid(active_bank_address);
    
    bool stay_in_bootloader = (!active_valid) || forced_upgrade;
    
    if (!stay_in_bootloader)
    {
        BSP_Jump_ToApplication(active_bank_address);
    }
    else
    {
        /* No valid app or upgrade requested -> stay in bootloader */
        PrepareUpgradeWindow(&ota_meta, stay_in_bootloader);
    }
    
    OTA_DownloadService_Init();
    OTA_TransportBle_Init();
    OTA_TransportRf_Init();
    
    /* Enter upgrade mode */
    if (BSP_Watchdog_Init(WATCHDOG_TIMEOUT_MS) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Main loop for upgrade mode */
    uint32_t last_watchdog_feed = HAL_GetTick();
    uint32_t last_led_toggle = HAL_GetTick();
    
    while (1)
    {
        uint32_t current_time = HAL_GetTick();
        
        /* Feed watchdog every 500ms */
        if (current_time - last_watchdog_feed >= 500)
        {
            BSP_Watchdog_Feed();
            last_watchdog_feed = current_time;
        }
        
        /* Blink LED every 500ms to indicate bootloader is running */
        if (current_time - last_led_toggle >= 500)
        {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            last_led_toggle = current_time;
        }
        
        OTA_TransportBle_Poll();
        OTA_TransportRf_Poll();
    }
}

/**
  * @brief  Attempt to promote staged firmware image to active bank
  * @param  meta: Pointer to OTA metadata
  * @param  forced_upgrade: If true, skip promotion logic
  * @retval true if promotion was attempted (success or failure), false if skipped
  */
static bool TryPromoteStagedImage(OtaMetadata_t *meta, bool forced_upgrade)
{
    if ((meta == NULL) || forced_upgrade)
    {
        return false;
    }
    
    bool has_ready_image = (meta->state == OTA_STATE_READY) &&
                           BSP_OtaMeta_IsBankIndexValid(meta->staged_bank) &&
                           (meta->staged_size > 0U);
    
    if (!has_ready_image)
    {
        return false;
    }
    
    uint32_t staged_address = BSP_OtaMeta_GetBankStart(meta->staged_bank);
    if (!BSP_Jump_IsApplicationValid(staged_address))
    {
        meta->last_error = OTA_ERROR_INVALID_IMAGE;
        meta->state = OTA_STATE_ROLLBACK;
        meta->staged_bank = OTA_BANK_INVALID;
        BSP_OtaMeta_Save(meta);
        return true;
    }
    
    meta->active_bank = meta->staged_bank;
    meta->staged_bank = OTA_BANK_INVALID;
    meta->state = OTA_STATE_IDLE;
    meta->last_error = OTA_ERROR_NONE;
    BSP_OtaMeta_Save(meta);
    return true;
}

/**
  * @brief  Prepare system for OTA upgrade window
  * @param  meta: Pointer to OTA metadata
  * @param  reset_window: If true, reset download context and staged bank
  * @retval None
  */
static void PrepareUpgradeWindow(OtaMetadata_t *meta, bool reset_window)
{
    if (meta == NULL)
    {
        return;
    }
    
    bool modified = false;
    
    if (reset_window || !BSP_OtaMeta_IsBankIndexValid(meta->staged_bank))
    {
        meta->staged_bank = BSP_OtaMeta_GetInactiveBank(meta->active_bank);
        meta->staged_size = 0U;
        meta->staged_crc = 0U;
        meta->staged_version = 0U;
        meta->last_error = OTA_ERROR_NONE;
        modified = true;
    }
    
    if (meta->state != OTA_STATE_DOWNLOADING)
    {
        meta->state = OTA_STATE_DOWNLOADING;
        modified = true;
    }
    
    if (modified)
    {
        BSP_OtaMeta_Save(meta);
    }
}

/**
  * @brief  System Clock Configuration
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    
    /*Configure GPIO pin : PC13 (LED) */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* Don't disable interrupts - we need SysTick for HAL_Delay and watchdog feeding */
    while (1)
    {
        /* Blink LED to indicate error */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(200);
        
        /* Feed watchdog to prevent reset loop */
        BSP_Watchdog_Feed();
    }
}



