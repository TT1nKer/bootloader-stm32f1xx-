/**
  ******************************************************************************
  * @file    ota_transport_can.c
  * @brief   CAN transport layer implementation for OTA firmware updates
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

/* Includes ------------------------------------------------------------------*/
#include "ota_transport_can.h"
#include "ota_transport.h"
#include "bootloader_config.h"
#include <string.h>

#if !ENABLE_CAN_TRANSPORT
#error "CAN transport is disabled. Set ENABLE_CAN_TRANSPORT to 1 in bootloader_config.h"
#endif

/* Private variables ---------------------------------------------------------*/
static CAN_HandleTypeDef hcan1;
static CanOtaContext_t can_ota_ctx;
static bool can_initialized = false;

/* CAN Filter Configuration */
#define CAN_OTA_FILTER_BANK          0U
#define CAN_OTA_FILTER_ID_HIGH       0x18FF0000U
#define CAN_OTA_FILTER_ID_LOW        0x18FF00FFU
#define CAN_OTA_FILTER_MASK_HIGH     0x1FFFFFFFU
#define CAN_OTA_FILTER_MASK_LOW      0x1FFFFFFFU

/* Device ID (can be configured via config area) */
#define CAN_OTA_DEVICE_ID            0x01U

/**
  * @brief  Initialize CAN transport layer for OTA
  * @retval None
  */
void OTA_TransportCan_Init(void)
{
    // Initialize CAN peripheral
    if (OTA_TransportCan_InitCAN() != HAL_OK) {
        can_initialized = false;
        return;
    }

    // Initialize context
    memset(&can_ota_ctx, 0, sizeof(CanOtaContext_t));
    can_ota_ctx.can_id = CAN_OTA_BASE_ID | CAN_OTA_DEVICE_ID;
    can_ota_ctx.active = false;

    can_initialized = true;
}

/**
  * @brief  Initialize CAN peripheral
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef OTA_TransportCan_InitCAN(void)
{
    CAN_FilterTypeDef sFilterConfig;
    HAL_StatusTypeDef status;

    // Configure CAN
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 18;                    // 250kbps at 36MHz APB1
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;

    status = HAL_CAN_Init(&hcan1);
    if (status != HAL_OK) {
        return status;
    }

    // Configure CAN filter for OTA messages
    sFilterConfig.FilterBank = CAN_OTA_FILTER_BANK;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = CAN_OTA_FILTER_ID_HIGH;
    sFilterConfig.FilterIdLow = CAN_OTA_FILTER_ID_LOW;
    sFilterConfig.FilterMaskIdHigh = CAN_OTA_FILTER_MASK_HIGH;
    sFilterConfig.FilterMaskIdLow = CAN_OTA_FILTER_MASK_LOW;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    status = HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
    if (status != HAL_OK) {
        HAL_CAN_DeInit(&hcan1);
        return status;
    }

    // Start CAN
    status = HAL_CAN_Start(&hcan1);
    if (status != HAL_OK) {
        HAL_CAN_DeInit(&hcan1);
        return status;
    }

    return HAL_OK;
}

/**
  * @brief  Check if CAN bus is active
  * @retval true if CAN bus is active, false otherwise
  */
bool OTA_TransportCan_IsBusActive(void)
{
    if (!can_initialized) {
        return false;
    }

    // Check CAN error status
    uint32_t error_code = HAL_CAN_GetError(&hcan1);
    if (error_code != HAL_CAN_ERROR_NONE) {
        // Try to recover from BUS OFF
        if (error_code & HAL_CAN_ERROR_BOF) {
            HAL_CAN_ResetError(&hcan1);
            HAL_CAN_Start(&hcan1);
        }
        return false;
    }

    return true;
}

/**
  * @brief  Poll CAN transport layer for incoming packets
  * @note   Should be called periodically from main loop
  * @retval None
  */
void OTA_TransportCan_Poll(void)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    CanOtaFrame_t frame;

    if (!can_initialized || !OTA_TransportCan_IsBusActive()) {
        return;
    }

    // Check for received messages
    if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
        // Check if this is an OTA message
        if ((rx_header.StdId & CAN_OTA_PGN_MASK) == CAN_OTA_PGN_VALUE) {
            // Parse frame
            frame.frame_type = rx_data[0];
            frame.sequence = rx_data[1];
            frame.flags = rx_data[2];
            memcpy(frame.data, &rx_data[3], 5);

            // Process frame
            CanOta_ProcessFrame(&frame, rx_header.StdId);
        }
    }

    // Check for fragment timeout
    if (can_ota_ctx.active) {
        uint32_t current_time = HAL_GetTick();
        if ((current_time - can_ota_ctx.last_fragment_time) > CAN_OTA_FRAGMENT_TIMEOUT_MS) {
            // Timeout - reset context
            memset(&can_ota_ctx, 0, sizeof(CanOtaContext_t));
            can_ota_ctx.can_id = CAN_OTA_BASE_ID | CAN_OTA_DEVICE_ID;
        }
    }
}

/**
  * @brief  Submit OTA packet received via CAN transport
  * @param  packet: Pointer to transport packet
  * @retval true if packet was accepted, false otherwise
  */
bool OTA_TransportCan_Submit(const OtaTransportPacket_t *packet)
{
    if (packet == NULL || !can_initialized) {
        return false;
    }

    OtaTransportPacket_t local_packet = *packet;
    local_packet.transport = OTA_TRANSPORT_CAN;
    return OTA_DownloadService_HandlePacket(&local_packet);
}

/**
  * @brief  Process incoming CAN OTA frame
  * @param  frame: Pointer to CAN frame
  * @param  can_id: CAN ID of received frame
  * @retval None
  */
void CanOta_ProcessFrame(const CanOtaFrame_t *frame, uint32_t can_id)
{
    (void)can_id;

    switch (frame->frame_type) {
        case CAN_OTA_FRAME_CONTROL:
            // Control packet - start new download
            // Note: Control packet contains metadata, needs multiple frames
            // For now, reset context and expect data frames
            memset(&can_ota_ctx, 0, sizeof(CanOtaContext_t));
            can_ota_ctx.can_id = CAN_OTA_BASE_ID | CAN_OTA_DEVICE_ID;
            can_ota_ctx.active = true;
            can_ota_ctx.expected_sequence = 0;
            can_ota_ctx.received_fragments = 0;
            can_ota_ctx.reassembly_offset = 0;
            can_ota_ctx.last_fragment_time = HAL_GetTick();
            can_ota_ctx.packet.transport = OTA_TRANSPORT_CAN;
            can_ota_ctx.packet.type = OTA_PACKET_CONTROL;

            // Store control data (first 5 bytes of control info)
            if (can_ota_ctx.reassembly_offset + 5 <= sizeof(can_ota_ctx.reassembly_buffer)) {
                memcpy(&can_ota_ctx.reassembly_buffer[can_ota_ctx.reassembly_offset],
                       frame->data, 5);
                can_ota_ctx.reassembly_offset += 5;
            }

            // Send ACK
            CanOta_SendFrame(CAN_OTA_FRAME_ACK, 0, 0, NULL, 0);
            break;

        case CAN_OTA_FRAME_DATA:
            // Data packet
            if (!can_ota_ctx.active) {
                // Not expecting data - send NACK
                CanOta_SendFrame(CAN_OTA_FRAME_NACK, frame->sequence, 0, NULL, 0);
                return;
            }

            // Check sequence
            if (frame->sequence != can_ota_ctx.expected_sequence) {
                // Out of sequence - send NACK with expected sequence
                uint8_t expected_seq = can_ota_ctx.expected_sequence;
                CanOta_SendFrame(CAN_OTA_FRAME_NACK, expected_seq, 0, NULL, 0);
                return;
            }

            // Copy data to reassembly buffer
            if (can_ota_ctx.reassembly_offset + 5 <= OTA_MAX_CHUNK_SIZE) {
                memcpy(&can_ota_ctx.reassembly_buffer[can_ota_ctx.reassembly_offset],
                       frame->data, 5);
                can_ota_ctx.reassembly_offset += 5;
            }

            can_ota_ctx.expected_sequence++;
            can_ota_ctx.received_fragments++;
            can_ota_ctx.last_fragment_time = HAL_GetTick();

            // Check if last fragment
            if (frame->flags & 0x01) {  // Last fragment flag
                // Reassemble packet
                can_ota_ctx.packet.transport = OTA_TRANSPORT_CAN;
                can_ota_ctx.packet.type = OTA_PACKET_DATA;
                can_ota_ctx.packet.length = can_ota_ctx.reassembly_offset;
                memcpy(can_ota_ctx.packet.payload, can_ota_ctx.reassembly_buffer,
                       can_ota_ctx.reassembly_offset);

                // Submit to download service
                OTA_DownloadService_HandlePacket(&can_ota_ctx.packet);

                // Reset context
                memset(&can_ota_ctx, 0, sizeof(CanOtaContext_t));
                can_ota_ctx.can_id = CAN_OTA_BASE_ID | CAN_OTA_DEVICE_ID;
            } else {
                // Send ACK
                CanOta_SendFrame(CAN_OTA_FRAME_ACK, frame->sequence, 0, NULL, 0);
            }
            break;

        case CAN_OTA_FRAME_ABORT:
            // Abort packet
            memset(&can_ota_ctx, 0, sizeof(CanOtaContext_t));
            can_ota_ctx.can_id = CAN_OTA_BASE_ID | CAN_OTA_DEVICE_ID;
            OTA_DownloadService_Abort(OTA_TRANSPORT_CAN);
            break;

        default:
            break;
    }
}

/**
  * @brief  Send CAN OTA frame
  * @param  frame_type: Frame type
  * @param  sequence: Sequence number
  * @param  flags: Flags
  * @param  data: Data payload
  * @param  length: Data length (max 5 bytes)
  * @retval None
  */
void CanOta_SendFrame(uint8_t frame_type, uint8_t sequence, uint8_t flags,
                      const uint8_t *data, uint8_t length)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[8];
    uint32_t tx_mailbox;

    if (!can_initialized || length > 5) {
        return;
    }

    // Prepare frame
    tx_data[0] = frame_type;
    tx_data[1] = sequence;
    tx_data[2] = flags;
    memset(&tx_data[3], 0, 5);
    if (data != NULL && length > 0) {
        memcpy(&tx_data[3], data, length);
    }

    // Configure CAN header
    tx_header.StdId = can_ota_ctx.can_id;
    tx_header.ExtId = 0x01;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 8;
    tx_header.TransmitGlobalTime = DISABLE;

    // Send frame
    HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox);
}

/**
  * @brief  Calculate CRC16 for CAN frame
  * @param  data: Data buffer
  * @param  length: Data length
  * @retval CRC16 value
  */
uint16_t CanOta_CalculateCRC16(const uint8_t *data, uint8_t length)
{
    uint16_t crc = 0xFFFF;
    const uint16_t polynomial = 0x1021;

    for (uint8_t i = 0; i < length; i++) {
        crc ^= ((uint16_t)data[i] << 8);
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}
