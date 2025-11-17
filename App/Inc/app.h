/**
  ******************************************************************************
  * @file    app.h
  * @brief   Application layer header file
  ******************************************************************************
  */

#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void App_Init(void);
void App_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */

