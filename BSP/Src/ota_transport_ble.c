/**
  ******************************************************************************
  * @file    ota_transport_ble.c
  * @brief   BLE transport shim for OTA packets
  ******************************************************************************
  */

#include "ota_transport.h"

/**
  * @brief  Initialize BLE transport layer for OTA
  * @note   Placeholder for BLE stack and GATT services initialization
  * @retval None
  */
void OTA_TransportBle_Init(void)
{
    /* Placeholder: initialize BLE stack / GATT services */
}

/**
  * @brief  Poll BLE transport layer for incoming packets
  * @note   Should be called periodically from main loop
  * @retval None
  */
void OTA_TransportBle_Poll(void)
{
    /* Placeholder: pump BLE events and forward OTA packets */
}

/**
  * @brief  Submit OTA packet received via BLE transport
  * @param  packet: Pointer to transport packet
  * @retval true if packet was accepted, false otherwise
  */
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


