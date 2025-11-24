/**
  ******************************************************************************
  * @file    ota_transport_ble.c
  * @brief   BLE transport shim for OTA packets
  ******************************************************************************
  */

#include "ota_transport.h"

void OTA_TransportBle_Init(void)
{
    /* Placeholder: initialize BLE stack / GATT services */
}

void OTA_TransportBle_Poll(void)
{
    /* Placeholder: pump BLE events and forward OTA packets */
}

bool OTA_TransportBle_Submit(const OtaTransportPacket_t *packet)
{
    if (packet == NULL)
    {
        return false;
    }

    OtaTransportPacket_t local_packet = *packet;
    local_packet.transport = OTA_TRANSPORT_BLE;
    return OTA_DownloadService_HandlePacket(&local_packet);
}


