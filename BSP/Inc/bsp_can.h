/**
  ******************************************************************************
  * @file    bsp_can.h
  * @brief   Board Support Package - CAN interface
  ******************************************************************************
  */

#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
HAL_StatusTypeDef BSP_CAN_Init(void);
HAL_StatusTypeDef BSP_CAN_Start(void);
HAL_StatusTypeDef BSP_CAN_Stop(void);
HAL_StatusTypeDef BSP_CAN_Send(uint32_t id, uint8_t *data, uint8_t len);
HAL_StatusTypeDef BSP_CAN_Receive(uint8_t *rx_data, uint8_t *rx_len);
CAN_HandleTypeDef* BSP_CAN_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_CAN_H */

