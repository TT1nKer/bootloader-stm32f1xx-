/**
  ******************************************************************************
  * @file    app.c
  * @brief   Application layer implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app.h"
#include "app_upgrade.h"

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Application initialization
  * @retval None
  */
void App_Init(void)
{
    /* Start CAN */
    BSP_CAN_Start();

    /* Add your application initialization code here */
}

/**
  * @brief  Application main process (called in main loop)
  * @retval None
  */
void App_Process(void)
{
    /* Add your application main loop code here */
    
    /* Example: Toggle LED */
    // BSP_LED_Toggle();
    // HAL_Delay(500);
    
    /* Example: Check for upgrade command via CAN */
    // CAN_RxHeaderTypeDef rx_header;
    // uint8_t rx_data[8];
    // if (HAL_CAN_GetRxFifoMsg(&hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK)
    // {
    //     // Check if upgrade command (ID: 0x123, Data[0]: 0xAA)
    //     if (rx_header.StdId == 0x123 && rx_data[0] == 0xAA)
    //     {
    //         // Trigger upgrade via CAN
    //         App_TriggerUpgrade(UPGRADE_MODE_CAN);
    //     }
    // }
}

