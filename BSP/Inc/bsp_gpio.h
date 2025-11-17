/**
  ******************************************************************************
  * @file    bsp_gpio.h
  * @brief   Board Support Package - GPIO interface
  ******************************************************************************
  */

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* LED definitions */
#define BSP_LED_PIN                GPIO_PIN_13
#define BSP_LED_PORT               GPIOC

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void BSP_GPIO_Init(void);
void BSP_LED_Init(void);
void BSP_LED_On(void);
void BSP_LED_Off(void);
void BSP_LED_Toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_GPIO_H */

