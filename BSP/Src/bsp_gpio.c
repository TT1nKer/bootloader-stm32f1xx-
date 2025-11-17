/**
  ******************************************************************************
  * @file    bsp_gpio.c
  * @brief   Board Support Package - GPIO implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp_gpio.h"

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initialize GPIO peripherals (wrapper)
  * @note   GPIO is already initialized by MX_GPIO_Init() in main.c
  *         This function is kept for compatibility, does nothing
  * @retval None
  */
void BSP_GPIO_Init(void)
{
    /* GPIO is already initialized by MX_GPIO_Init() in main.c */
    /* This function is kept for compatibility with BSP_Init() */
}

/**
  * @brief  Initialize LED GPIO
  * @retval None
  */
void BSP_LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(BSP_LED_PORT, BSP_LED_PIN, GPIO_PIN_RESET);

    /*Configure GPIO pin : PC13 */
    GPIO_InitStruct.Pin = BSP_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BSP_LED_PORT, &GPIO_InitStruct);
}

/**
  * @brief  Turn LED on
  * @retval None
  */
void BSP_LED_On(void)
{
    HAL_GPIO_WritePin(BSP_LED_PORT, BSP_LED_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Turn LED off
  * @retval None
  */
void BSP_LED_Off(void)
{
    HAL_GPIO_WritePin(BSP_LED_PORT, BSP_LED_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  Toggle LED
  * @retval None
  */
void BSP_LED_Toggle(void)
{
    HAL_GPIO_TogglePin(BSP_LED_PORT, BSP_LED_PIN);
}

