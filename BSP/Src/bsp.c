/**
  ******************************************************************************
  * @file    bsp.c
  * @brief   Board Support Package - Main implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"

/**
  * @brief  Initialize all BSP modules
  * @note   Peripherals are already initialized by MX_xxx_Init() functions in main.c
  *         This function only wraps HAL operations and starts peripherals if needed
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_Init(void)
{
    HAL_StatusTypeDef status = HAL_OK;

    /* GPIO is already initialized by MX_GPIO_Init() in main.c */
    BSP_GPIO_Init();  /* Does nothing, kept for compatibility */

    /* CAN is already initialized by MX_CAN_Init() in main.c */
    /* BSP_CAN_Init() will start the CAN peripheral */
    status = BSP_CAN_Init();
    if (status != HAL_OK)
    {
        return status;
    }

    return HAL_OK;
}

