/**
  ******************************************************************************
  * @file    ota_transport_can.h
  * @brief   CAN transport layer for OTA firmware updates
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

#ifndef __OTA_TRANSPORT_CAN_H
#define __OTA_TRANSPORT_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "ota_transport.h"

/* CAN OTA Protocol Configuration */
#define CAN_OTA_BASE_ID              0x18FF0000U  /* Base CAN ID for OTA */
#define CAN_OTA_DEVICE_ID_MASK       0x000000FFU  /* Device ID mask */
#define CAN_OTA_PGN_MASK             0x00FFFF00U  /* PGN mask (J1939-style) */
#define CAN_OTA_PGN_VALUE            0x00FF00U    /* OTA PGN */

/* CAN Frame Types */
#define CAN_OTA_FRAME_CONTROL        0x01U        /* Control frame */
#define CAN_OTA_FRAME_DATA           0x02U        /* Data frame */
#define CAN_OTA_FRAME_ABORT          0x03U        /* Abort frame */
#define CAN_OTA_FRAME_ACK            0x04U        /* Acknowledgment frame */
#define CAN_OTA_FRAME_NACK           0x05U        /* Negative acknowledgment */

/* CAN Frame Structure (8 bytes) */
#define CAN_OTA_FRAME_HEADER_SIZE    3U           /* Frame type + sequence + flags */
#define CAN_OTA_FRAME_DATA_SIZE      5U           /* Max data bytes per frame */
#define CAN_OTA_FRAME_CRC_SIZE       2U           /* CRC16 for frame integrity */

/* Packet Fragmentation */
#define CAN_OTA_MAX_FRAGMENTS        512U         /* Max fragments per packet */
#define CAN_OTA_FRAGMENT_TIMEOUT_MS  1000U        /* Timeout per fragment */

/* CAN OTA Frame Structure */
typedef struct
{
    uint8_t frame_type;      /* Frame type (CONTROL/DATA/ABORT) */
    uint8_t sequence;        /* Fragment sequence number */
    uint8_t flags;           /* Flags (last fragment, etc.) */
    uint8_t data[5];        /* Data payload (5 bytes max) */
} CanOtaFrame_t;

/* CAN OTA Context */
typedef struct
{
    bool active;                      /* Transport active */
    uint32_t can_id;                 /* CAN ID for this device */
    uint32_t expected_sequence;      /* Expected fragment sequence */
    uint32_t received_fragments;      /* Number of fragments received */
    uint8_t reassembly_buffer[OTA_MAX_CHUNK_SIZE]; /* Reassembly buffer */
    uint32_t reassembly_offset;      /* Current offset in buffer */
    uint32_t last_fragment_time;     /* Timestamp of last fragment */
    OtaTransportPacket_t packet;     /* Reassembled packet */
} CanOtaContext_t;

/* Function declarations */
void OTA_TransportCan_Init(void);
void OTA_TransportCan_Poll(void);
bool OTA_TransportCan_Submit(const OtaTransportPacket_t *packet);
HAL_StatusTypeDef OTA_TransportCan_InitCAN(void);
bool OTA_TransportCan_IsBusActive(void);

/* Private helpers (internal use only) */
void CanOta_ProcessFrame(const CanOtaFrame_t *frame, uint32_t can_id);
void CanOta_SendFrame(uint8_t frame_type, uint8_t sequence, uint8_t flags, const uint8_t *data, uint8_t length);
uint16_t CanOta_CalculateCRC16(const uint8_t *data, uint8_t length);

#ifdef __cplusplus
}
#endif

#endif /* __OTA_TRANSPORT_CAN_H */
