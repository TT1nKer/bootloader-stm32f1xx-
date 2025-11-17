/**
  ******************************************************************************
  * @file    bsp.h
  * @brief   Board Support Package - Main header file
  ******************************************************************************
  */

#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp_can.h"
#include "bsp_gpio.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
HAL_StatusTypeDef BSP_Init(void);

#ifdef __cplusplus·
}
#endif

#endif /* __BSP_H */

