/**
  ******************************************************************************
  * @file    crypto_ecdsa.c
  * @brief   Lightweight ECDSA signature verification (secp256r1)
  ******************************************************************************
  * @attention
  *
  * This is a simplified ECDSA verification implementation for bootloader use.
  * For production, consider using a proven crypto library like micro-ecc.
  *
  * Copyright (c) 2025 TT1nker (GitHub: TT1nker)
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "crypto.h"
#include "main.h"
#include "bootloader_config.h"
#include <string.h>

/* secp256r1 curve parameters */
#define ECDSA_P_BITS                 256U
#define ECDSA_P_BYTES                32U

/* Note: This is a placeholder implementation. For production use, integrate
 * a proven crypto library like micro-ecc or mbedTLS minimal.
 * Full ECDSA verification requires:
 * - Big integer arithmetic
 * - Elliptic curve point operations
 * - Modular arithmetic
 * - ~8KB code size
 */

/**
  * @brief  Verify ECDSA signature
  * @param  hash: Hash of the message
  * @param  hash_length: Hash length (must be 32 for SHA-256)
  * @param  signature: ECDSA signature
  * @param  public_key: ECDSA public key
  * @retval true if signature is valid, false otherwise
  * @note   Currently returns false (not implemented) - placeholder for future crypto library integration
  */
bool Crypto_ECDSA_Verify(const uint8_t *hash, uint32_t hash_length,
                         const ecdsa_signature_t *signature,
                         const ecdsa_public_key_t *public_key)
{
    (void)hash;
    (void)hash_length;
    (void)signature;
    (void)public_key;

    /* TODO: Implement ECDSA verification using crypto library
     * For now, return false to indicate signature verification is not available
     * This prevents accepting unsigned firmware when signature verification is expected
     */
    return false;
}

/**
  * @brief  Verify ECDSA signature from raw data
  * @param  hash: Hash of the message
  * @param  hash_length: Hash length
  * @param  signature_data: Raw signature data (64 bytes: R + S)
  * @param  signature_length: Signature length
  * @param  public_key_data: Raw public key data (64 bytes: X + Y)
  * @param  public_key_length: Public key length
  * @retval true if signature is valid, false otherwise
  */
bool Crypto_ECDSA_VerifySignature(const uint8_t *hash, uint32_t hash_length,
                                   const uint8_t *signature_data, uint32_t signature_length,
                                   const uint8_t *public_key_data, uint32_t public_key_length)
{
    ecdsa_signature_t signature;
    ecdsa_public_key_t public_key;

    if (hash == NULL || signature_data == NULL || public_key_data == NULL) {
        return false;
    }

    if (hash_length != SHA256_DIGEST_SIZE ||
        signature_length != ECDSA_SIGNATURE_SIZE ||
        public_key_length != ECDSA_PUBLIC_KEY_SIZE) {
        return false;
    }

    // Parse signature
    memcpy(signature.r, signature_data, 32);
    memcpy(signature.s, &signature_data[32], 32);

    // Parse public key
    memcpy(public_key.x, public_key_data, 32);
    memcpy(public_key.y, &public_key_data[32], 32);

    return Crypto_ECDSA_Verify(hash, hash_length, &signature, &public_key);
}

/**
  * @brief  Load public key from Flash
  * @param  flash_address: Flash address of public key
  * @param  public_key: Output public key structure
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef Crypto_LoadPublicKey(uint32_t flash_address, ecdsa_public_key_t *public_key)
{
    if (public_key == NULL) {
        return HAL_ERROR;
    }

    // Read public key from flash
    memcpy(public_key->x, (const void *)flash_address, 32);
    memcpy(public_key->y, (const void *)(flash_address + 32), 32);

    return HAL_OK;
}

/**
  * @brief  Check if public key is valid
  * @param  public_key: Public key to validate
  * @retval true if valid, false otherwise
  */
bool Crypto_IsPublicKeyValid(const ecdsa_public_key_t *public_key)
{
    if (public_key == NULL) {
        return false;
    }

    // Basic validation: check for all zeros (invalid key)
    bool x_zero = true;
    bool y_zero = true;

    for (uint32_t i = 0; i < 32; i++) {
        if (public_key->x[i] != 0) {
            x_zero = false;
        }
        if (public_key->y[i] != 0) {
            y_zero = false;
        }
    }

    return !(x_zero && y_zero);
}
