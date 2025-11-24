/**
  ******************************************************************************
  * @file    ota_security.c
  * @brief   Basic integrity + signature scaffolding for OTA images
  ******************************************************************************
  */

#include "ota_security.h"

/**
  * @brief  Initialize OTA security subsystem
  * @note   Placeholder for loading public keys from secure storage
  * @retval None
  */
void OTA_Security_Init(void)
{
    /* Placeholder: load public keys from secure storage if needed */
}

/**
  * @brief  Validate CRC32 of downloaded firmware
  * @param  expected_crc: Expected CRC value from control packet
  * @param  actual_crc: Calculated CRC value from downloaded data
  * @retval true if CRC matches or validation disabled, false otherwise
  */
bool OTA_Security_ValidateCrc(uint32_t expected_crc, uint32_t actual_crc)
{
    if (expected_crc == 0U)
    {
        /* No CRC provided -> treat as success (useful for early bring-up) */
        return true;
    }

    return (expected_crc == actual_crc);
}

/**
  * @brief  Validate cryptographic signature of firmware image
  * @param  signature: Pointer to signature buffer
  * @param  signature_length: Length of signature in bytes
  * @param  image_address: Flash address of firmware image
  * @param  image_size: Size of firmware image in bytes
  * @retval true if signature is valid (currently always returns true - placeholder)
  * @note   TODO: Implement SHA-256 hash and RSA/ECDSA signature verification
  */
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


