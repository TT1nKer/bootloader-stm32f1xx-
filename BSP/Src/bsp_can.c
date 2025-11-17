/**
  ******************************************************************************
  * @file    bsp_can.c
  * @brief   Board Support Package - CAN implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp_can.h"
#include "main.h"

/* External variables ---------------------------------------------------------*/
/* Reference to CAN handle from main.c (initialized by MX_CAN_Init) */
extern CAN_HandleTypeDef hcan;

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initialize CAN peripheral (wrapper)
  * @note   CAN is already initialized by MX_CAN_Init() in main.c
  *         This function only starts the CAN peripheral
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_CAN_Init(void)
{
    /* CAN is already initialized by MX_CAN_Init() in main.c */
    /* Just start the CAN peripheral */
    return HAL_CAN_Start(&hcan);
}

/**
  * @brief  Start CAN peripheral
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_CAN_Start(void)
{
    return HAL_CAN_Start(&hcan);
}

/**
  * @brief  Stop CAN peripheral
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_CAN_Stop(void)
{
    return HAL_CAN_Stop(&hcan);
}

/**
  * @brief  Send CAN message
  * @param  id: CAN message ID
  * @param  data: Pointer to data buffer
  * @param  len: Data length (max 8)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_CAN_Send(uint32_t id, uint8_t *data, uint8_t len)
{
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;

    if (len > 8)
    {
        return HAL_ERROR;
    }

    tx_header.StdId = id;
    tx_header.ExtId = 0x00;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = len;
    tx_header.TransmitGlobalTime = DISABLE;

    return HAL_CAN_AddTxMessage(&hcan, &tx_header, data, &tx_mailbox);
}

/**
  * @brief  Receive CAN message
  * @param  rx_data: Pointer to receive data buffer
  * @param  rx_len: Pointer to receive data length
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_CAN_Receive(uint8_t *rx_data, uint8_t *rx_len)
{
    CAN_RxHeaderTypeDef rx_header;

    if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0)
    {
        if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK)
        {
            *rx_len = rx_header.DLC;
            return HAL_OK;
        }
    }

    return HAL_ERROR;
}

/**
  * @brief  Get CAN handle
  * @retval CAN_HandleTypeDef pointer
  */
CAN_HandleTypeDef* BSP_CAN_GetHandle(void)
{
    return &hcan;
}

