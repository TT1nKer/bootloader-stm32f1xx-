/**
  ******************************************************************************
  * @file    ota_download.c
  * @brief   OTA download manager - common logic shared across transports
  ******************************************************************************
  */

#include "ota_transport.h"
#include "bsp_flash.h"
#include "bsp_ota_meta.h"
#include "ota_security.h"
#include <string.h>

typedef struct
{
    bool active;
    OtaTransportType_t transport;
    uint32_t bank_index;
    uint32_t bank_address;
    uint32_t total_size;
    uint32_t received;
    uint32_t expected_sequence;
    uint32_t expected_crc;
    uint32_t running_crc;
    uint32_t version;
    uint32_t signature_length;
    uint8_t signature[OTA_SIGNATURE_MAX_BYTES];
} OtaDownloadContext_t;

static OtaDownloadContext_t s_ctx;

/* Private helpers -----------------------------------------------------------*/
static void ResetContext(void);
static bool BeginDownload(const OtaTransportPacket_t *packet);
static bool HandleDataPacket(const OtaTransportPacket_t *packet);
static bool FinalizeDownload(void);

/**
  * @brief  Initialize OTA download service
  * @retval None
  */
void OTA_DownloadService_Init(void)
{
    OTA_Security_Init();
    ResetContext();
}

/**
  * @brief  Handle incoming OTA transport packet
  * @param  packet: Pointer to transport packet
  * @retval true if packet was handled successfully, false otherwise
  */
bool OTA_DownloadService_HandlePacket(const OtaTransportPacket_t *packet)
{
    if (packet == NULL)
    {
        return false;
    }

    switch (packet->type)
    {
        case OTA_PACKET_CONTROL:
            return BeginDownload(packet);

        case OTA_PACKET_DATA:
            return HandleDataPacket(packet);

        case OTA_PACKET_ABORT:
            OTA_DownloadService_Abort(packet->transport);
            return true;

        default:
            return false;
    }
}

/**
  * @brief  Abort ongoing OTA download
  * @param  transport: Transport type that initiated abort
  * @retval None
  */
void OTA_DownloadService_Abort(OtaTransportType_t transport)
{
    (void)transport;
    ResetContext();
}

/**
  * @brief  Check if OTA download is currently active
  * @retval true if download is in progress, false otherwise
  */
bool OTA_DownloadService_IsActive(void)
{
    return s_ctx.active;
}

/**
  * @brief  Get expected packet sequence number
  * @retval Expected sequence number for next packet
  */
uint32_t OTA_DownloadService_GetExpectedSequence(void)
{
    return s_ctx.expected_sequence;
}

/**
  * @brief  Reset download context to initial state
  * @retval None
  */
static void ResetContext(void)
{
    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.expected_crc = 0U;
    s_ctx.running_crc = 0xFFFFFFFFU;
}

/**
  * @brief  Begin new OTA download session
  * @param  packet: Pointer to control packet with download parameters
  * @retval true if download session started successfully, false otherwise
  */
static bool BeginDownload(const OtaTransportPacket_t *packet)
{
    if (packet->total_size == 0U || packet->total_size > OTA_BANK_SIZE)
    {
        return false;
    }

    OtaMetadata_t meta;
    BSP_OtaMeta_Load(&meta);

    if (!BSP_OtaMeta_IsBankIndexValid(meta.staged_bank))
    {
        return false;
    }

    if (BSP_Flash_EraseBank(meta.staged_bank) != HAL_OK)
    {
        meta.last_error = OTA_ERROR_STORAGE_FAIL;
        BSP_OtaMeta_Save(&meta);
        return false;
    }

    meta.state = OTA_STATE_DOWNLOADING;
    meta.staged_size = 0U;
    meta.staged_crc = 0U;
    meta.staged_version = packet->version;
    meta.last_error = OTA_ERROR_NONE;
    BSP_OtaMeta_Save(&meta);

    s_ctx.active = true;
    s_ctx.transport = packet->transport;
    s_ctx.bank_index = meta.staged_bank;
    s_ctx.bank_address = BSP_OtaMeta_GetBankStart(meta.staged_bank);
    s_ctx.total_size = packet->total_size;
    s_ctx.received = 0U;
    s_ctx.expected_sequence = 0U;
    s_ctx.expected_crc = packet->crc32;
    s_ctx.running_crc = 0xFFFFFFFFU;
    s_ctx.version = packet->version;
    s_ctx.signature_length = (packet->signature_length > OTA_SIGNATURE_MAX_BYTES)
                               ? OTA_SIGNATURE_MAX_BYTES
                               : packet->signature_length;
    if (s_ctx.signature_length > 0U)
    {
        memcpy(s_ctx.signature, packet->signature, s_ctx.signature_length);
    }

    return true;
}

/**
  * @brief  Handle data packet during OTA download
  * @param  packet: Pointer to data packet
  * @retval true if data was written successfully, false otherwise
  */
static bool HandleDataPacket(const OtaTransportPacket_t *packet)
{
    if (!s_ctx.active)
    {
        return false;
    }

    if (packet->sequence != s_ctx.expected_sequence)
    {
        return false;
    }

    if (packet->length == 0U || packet->length > OTA_MAX_CHUNK_SIZE)
    {
        return false;
    }

    if (packet->offset != s_ctx.received)
    {
        return false;
    }

    if ((s_ctx.received + packet->length) > s_ctx.total_size)
    {
        return false;
    }

    HAL_StatusTypeDef status = BSP_Flash_WriteChunk(s_ctx.bank_index,
                                                    packet->offset,
                                                    packet->payload,
                                                    packet->length,
                                                    &s_ctx.running_crc);
    if (status != HAL_OK)
    {
        return false;
    }

    s_ctx.received += packet->length;
    s_ctx.expected_sequence++;

    if (s_ctx.received == s_ctx.total_size)
    {
        return FinalizeDownload();
    }

    return true;
}

/**
  * @brief  Finalize OTA download with CRC and signature validation
  * @retval true if download completed and validated successfully, false otherwise
  */
static bool FinalizeDownload(void)
{
    uint32_t final_crc = s_ctx.running_crc;

    OtaMetadata_t meta;
    BSP_OtaMeta_Load(&meta);

    if (!BSP_OtaMeta_IsBankIndexValid(meta.staged_bank))
    {
        ResetContext();
        return false;
    }

    if ((meta.staged_bank != s_ctx.bank_index) ||
        (s_ctx.total_size == 0U))
    {
        ResetContext();
        return false;
    }

    if (!OTA_Security_ValidateCrc(s_ctx.expected_crc, final_crc))
    {
        meta.last_error = OTA_ERROR_CRC_MISMATCH;
        meta.state = OTA_STATE_ROLLBACK;
        BSP_OtaMeta_Save(&meta);
        ResetContext();
        return false;
    }

    if (!OTA_Security_ValidateSignature(s_ctx.signature,
                                        s_ctx.signature_length,
                                        s_ctx.bank_address,
                                        s_ctx.total_size))
    {
        meta.last_error = OTA_ERROR_INVALID_IMAGE;
        meta.state = OTA_STATE_ROLLBACK;
        BSP_OtaMeta_Save(&meta);
        ResetContext();
        return false;
    }

    meta.staged_size = s_ctx.total_size;
    meta.staged_crc = final_crc;
    meta.staged_version = s_ctx.version;
    meta.state = OTA_STATE_READY;
    meta.last_error = OTA_ERROR_NONE;
    BSP_OtaMeta_Save(&meta);

    ResetContext();
    return true;
}


