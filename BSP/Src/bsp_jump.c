/**
  ******************************************************************************
  * @file    bsp_jump.c
  * @brief   Board Support Package - Application jump implementation
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
#include "bsp_jump.h"
#include "bsp_flash.h"

/* Private function prototypes -----------------------------------------------*/
static void JumpToAddress(uint32_t address);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Check if application is valid
  * @retval true if valid, false otherwise
  */
bool BSP_Jump_IsApplicationValid(void)
{
    // Read application vector table
    uint32_t app_stack = BSP_Flash_ReadWord(APP_START_ADDRESS);
    uint32_t app_entry = BSP_Flash_ReadWord(APP_START_ADDRESS + 4);
    
    // Check stack pointer validity (should be in RAM range)
    if (app_stack < 0x20000000 || app_stack > 0x20005000)
    {
        return false;
    }
    
    // Check program entry validity (should be in Flash range)
    if (app_entry < APP_START_ADDRESS || app_entry > APP_END_ADDRESS)
    {
        return false;
    }
    
    // Check if entry address is odd (Thumb mode indicator for ARM Cortex-M)
    if ((app_entry & 0x01) == 0)
    {
        return false;
    }
    
    return true;
}

/**
  * @brief  Jump to application
  * @retval None (function never returns)
  */
void BSP_Jump_ToApplication(void)
{
    // 1. Disable all interrupts
    __disable_irq();
    
    // 2. Deinitialize all peripherals
    // Note: In bootloader, we may not have initialized all peripherals
    // Only deinitialize what we used
    
    // 3. Reset SysTick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    
    // 4. Set vector table offset (application should set this, but we do it here for safety)
    SCB->VTOR = APP_START_ADDRESS;
    
    // 5. Set stack pointer
    uint32_t app_stack = BSP_Flash_ReadWord(APP_START_ADDRESS);
    __set_MSP(app_stack);
    
    // 6. Get application entry point
    uint32_t app_entry = BSP_Flash_ReadWord(APP_START_ADDRESS + 4);
    
    // 7. Jump to application
    JumpToAddress(app_entry);
}

/**
  * @brief  Internal function to jump to address
  * @param  address: Target address
  * @retval None (function never returns)
  */
static void JumpToAddress(uint32_t address)
{
    // Create function pointer
    void (*app_reset_handler)(void) = (void (*)(void))address;
    
    // Jump to application
    app_reset_handler();
    
    // Should never reach here
    while(1);
}

