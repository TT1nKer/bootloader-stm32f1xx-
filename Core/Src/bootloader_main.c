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
#include "bsp_upgrade_flag.h"
#include "bsp_watchdog.h"
#include <stdio.h>

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
void MX_GPIO_Init(void);

/* Private variables ---------------------------------------------------------*/
uint32_t last_watchdog_feed = 0;
uint32_t last_led_toggle = 0;

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
    
    /* Initialize Watchdog */
    if (BSP_Watchdog_Init(WATCHDOG_TIMEOUT_MS) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Check upgrade flag */
    UpgradeMode_t upgrade_mode = BSP_UpgradeFlag_Get();
    
    if (upgrade_mode == UPGRADE_MODE_NONE)
    {
        /* Normal boot: Check if application is valid */
        if (BSP_Jump_IsApplicationValid())
        {
            /* Jump to application */
            BSP_Jump_ToApplication();
        }
    }
    
    /* Enter upgrade mode */
    /* Clear upgrade flag */
    BSP_UpgradeFlag_Clear();
    
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
        
        /* TODO: Process upgrade messages */
        /* ProcessCANMessages(); */
        /* ProcessBLEMessages(); */
        /* ProcessRFMessages(); */
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



