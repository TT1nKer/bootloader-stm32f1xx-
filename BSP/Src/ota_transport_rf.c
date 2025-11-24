/**
  ******************************************************************************
  * @file    ota_transport_rf.c
  * @brief   Proprietary RF transport shim for OTA packets
  ******************************************************************************
  */

#include "ota_transport.h"

/**
  * @brief  Initialize proprietary RF transport layer for OTA
  * @note   Placeholder for RF transceiver and protocol stack initialization
  * @retval None
  */
void OTA_TransportRf_Init(void)
{
    /* Placeholder: initialize RF transceiver / protocol stack */
}

/**
  * @brief  Poll RF transport layer for incoming packets
  * @note   Should be called periodically from main loop to handle retransmissions
  * @retval None
  */
void OTA_TransportRf_Poll(void)
{
    /* Placeholder: poll RF driver, handle retransmissions, etc. */
}

/**
  * @brief  Submit OTA packet received via RF transport
  * @param  packet: Pointer to transport packet
  * @retval true if packet was accepted, false otherwise
  */
bool OTA_TransportRf_Submit(const OtaTransportPacket_t *packet)
{
    if (packet == NULL)
    {
        return false;
    }

    OtaTransportPacket_t local_packet = *packet;
    local_packet.transport = OTA_TRANSPORT_RF;
    return OTA_DownloadService_HandlePacket(&local_packet);
}


