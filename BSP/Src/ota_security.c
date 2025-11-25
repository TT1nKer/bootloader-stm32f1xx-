/**
  ******************************************************************************
  * @file    ota_security.c
  * @brief   Basic integrity + signature scaffolding for OTA images
  ******************************************************************************
  */

#include "ota_security.h"
#include "crypto.h"
#include "bootloader_config.h"
#include <string.h>

/* Private variables */
static ecdsa_public_key_t oem_public_key;
static bool public_key_loaded = false;

/**
  * @brief  Initialize OTA security subsystem
  * @note   Loads public key from secure storage
  * @retval None
  */
void OTA_Security_Init(void)
{
#if ENABLE_SIGNATURE_VERIFY
    // Load public key from config area
    if (Crypto_LoadPublicKey(ECDSA_PUBLIC_KEY_ADDRESS, &oem_public_key) == HAL_OK) {
        if (Crypto_IsPublicKeyValid(&oem_public_key)) {
            public_key_loaded = true;
        }
    }
#else
    // Signature verification disabled
    public_key_loaded = false;
#endif
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
  * @retval true if signature is valid, false otherwise
  */
bool OTA_Security_ValidateSignature(const uint8_t *signature,
                                    uint32_t signature_length,
                                    uint32_t image_address,
                                    uint32_t image_size)
{
#if ENABLE_SIGNATURE_VERIFY
    uint8_t hash[SHA256_DIGEST_SIZE];
    HAL_StatusTypeDef status;

    // If no signature provided, allow (for backward compatibility)
    if (signature == NULL || signature_length == 0) {
        return true;
    }

    // If signature length doesn't match ECDSA signature size, reject
    if (signature_length != ECDSA_SIGNATURE_SIZE) {
        return false;
    }

    // If public key not loaded, cannot verify - reject for security
    if (!public_key_loaded) {
        return false;
    }

    // Calculate SHA-256 hash of firmware image
    status = Crypto_SHA256_CalculateFromFlash(image_address, image_size, hash);
    if (status != HAL_OK) {
        return false;
    }

    // Verify ECDSA signature
    return Crypto_ECDSA_VerifySignature(hash, SHA256_DIGEST_SIZE,
                                        signature, signature_length,
                                        (const uint8_t *)&oem_public_key,
                                        ECDSA_PUBLIC_KEY_SIZE);
#else
    // Signature verification disabled - allow unsigned firmware
    (void)signature;
    (void)signature_length;
    (void)image_address;
    (void)image_size;
    return true;
#endif
}

/**
  * @brief  Calculate SHA-256 hash of firmware image
  * @param  image_address: Flash address of firmware image
  * @param  image_size: Size of firmware image in bytes
  * @param  hash: Output hash buffer (32 bytes)
  * @retval true if hash calculated successfully, false otherwise
  */
bool OTA_Security_CalculateSHA256(uint32_t image_address, uint32_t image_size, uint8_t hash[SHA256_DIGEST_SIZE])
{
    if (hash == NULL) {
        return false;
    }

    return (Crypto_SHA256_CalculateFromFlash(image_address, image_size, hash) == HAL_OK);
}


