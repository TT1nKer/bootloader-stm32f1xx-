/**
  ******************************************************************************
  * @file    ota_transport_rf.c
  * @brief   Proprietary RF transport shim for OTA packets
  ******************************************************************************
  */

#include "ota_transport.h"

void OTA_TransportRf_Init(void)
{
    /* Placeholder: initialize RF transceiver / protocol stack */
}

void OTA_TransportRf_Poll(void)
{
    /* Placeholder: poll RF driver, handle retransmissions, etc. */
}

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


