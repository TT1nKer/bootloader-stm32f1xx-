/**
  ******************************************************************************
  * @file    ota_transport.h
  * @brief   OTA transport abstractions (BLE / RF) and download manager API
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  ******************************************************************************
  */

#ifndef __OTA_TRANSPORT_H
#define __OTA_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

#define OTA_MAX_CHUNK_SIZE           256U
#define OTA_SIGNATURE_MAX_BYTES      64U

typedef enum
{
    OTA_TRANSPORT_BLE = 0,
    OTA_TRANSPORT_RF = 1
} OtaTransportType_t;

typedef enum
{
    OTA_PACKET_CONTROL = 0,
    OTA_PACKET_DATA = 1,
    OTA_PACKET_ABORT = 2
} OtaPacketType_t;

typedef struct
{
    OtaTransportType_t transport;
    OtaPacketType_t type;
    uint32_t sequence;
    uint32_t offset;
    uint32_t length;
    uint32_t total_size;
    uint32_t version;
    uint32_t crc32;
    uint32_t signature_length;
    uint8_t signature[OTA_SIGNATURE_MAX_BYTES];
    uint8_t payload[OTA_MAX_CHUNK_SIZE];
} OtaTransportPacket_t;

void OTA_DownloadService_Init(void);
bool OTA_DownloadService_HandlePacket(const OtaTransportPacket_t *packet);
void OTA_DownloadService_Abort(OtaTransportType_t transport);
bool OTA_DownloadService_IsActive(void);
uint32_t OTA_DownloadService_GetExpectedSequence(void);

void OTA_TransportBle_Init(void);
void OTA_TransportBle_Poll(void);
bool OTA_TransportBle_Submit(const OtaTransportPacket_t *packet);

void OTA_TransportRf_Init(void);
void OTA_TransportRf_Poll(void);
bool OTA_TransportRf_Submit(const OtaTransportPacket_t *packet);

#ifdef __cplusplus
}
#endif

#endif /* __OTA_TRANSPORT_H */


