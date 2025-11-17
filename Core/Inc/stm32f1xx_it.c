/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  * Bootloader Modifications Copyright (c) 2025 TT1nker (GitHub: TT1nker)
  * Contact: hostsjim22@gmail.com
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
#include "bsp_watchdog.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */
  SCB->VTOR = 0x08000000;
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
  GPIOC->CRH |= GPIO_CRH_MODE13_0;
  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    BSP_Watchdog_Feed();
    for (volatile uint32_t i = 0; i < 100000; i++);
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  /* Force set VTOR to bootloader address */
  SCB->VTOR = 0x08000000;
  
  /* Enable GPIOC clock for LED */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  /* Configure PC13 as output (LED) */
  GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
  GPIOC->CRH |= GPIO_CRH_MODE13_0;  // Output mode, 10MHz
  
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* Blink LED rapidly to indicate HardFault */
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    
    /* Feed watchdog to prevent reset loop */
    BSP_Watchdog_Feed();
    
    /* Simple delay */
    for (volatile uint32_t i = 0; i < 100000; i++);
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  SCB->VTOR = 0x08000000;
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
  GPIOC->CRH |= GPIO_CRH_MODE13_0;
  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    BSP_Watchdog_Feed();
    for (volatile uint32_t i = 0; i < 100000; i++);
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */
  SCB->VTOR = 0x08000000;
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
  GPIOC->CRH |= GPIO_CRH_MODE13_0;
  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    BSP_Watchdog_Feed();
    for (volatile uint32_t i = 0; i < 100000; i++);
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
  SCB->VTOR = 0x08000000;
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
  GPIOC->CRH |= GPIO_CRH_MODE13_0;
  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    BSP_Watchdog_Feed();
    for (volatile uint32_t i = 0; i < 100000; i++);
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
