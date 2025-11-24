/**
  ******************************************************************************
  * @file    ota_security.c
  * @brief   Basic integrity + signature scaffolding for OTA images
  ******************************************************************************
  */

#include "ota_security.h"

void OTA_Security_Init(void)
{
    /* Placeholder: load public keys from secure storage if needed */
}

bool OTA_Security_ValidateCrc(uint32_t expected_crc, uint32_t actual_crc)
{
    if (expected_crc == 0U)
    {
        /* No CRC provided -> treat as success (useful for early bring-up) */
        return true;
    }

    return (expected_crc == actual_crc);
}

bool OTA_Security_ValidateSignature(const uint8_t *signature,
                                    uint32_t signature_length,
                                    uint32_t image_address,
                                    uint32_t image_size)
{
    (void)signature;
    (void)signature_length;
    (void)image_address;
    (void)image_size;

    /*
     * TODO:
     *  - Derive hash (SHA-256) over [image_address, image_size]
     *  - Verify signature using RSA/ECDSA public key
     *  - Return true only when signature matches
     */
    return true;
}


