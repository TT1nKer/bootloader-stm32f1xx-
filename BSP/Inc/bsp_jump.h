/**
  ******************************************************************************
  * @file    bsp_jump.h
  * @brief   Board Support Package - Application jump
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

#ifndef __BSP_JUMP_H
#define __BSP_JUMP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "main.h"
#include "bootloader_config.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
bool BSP_Jump_IsApplicationValid(uint32_t app_start_address);
void BSP_Jump_ToApplication(uint32_t app_start_address);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_JUMP_H */



