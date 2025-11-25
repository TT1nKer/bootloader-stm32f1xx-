/**
  ******************************************************************************
  * @file    crypto.h
  * @brief   Lightweight cryptographic functions for OTA signature verification
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 TT1nker (GitHub: TT1nker)
  * All rights reserved.
  *
  * Contact: hostsjim22@gmail.com
  *
  ******************************************************************************
  */

#ifndef __CRYPTO_H
#define __CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* SHA-256 Configuration */
#define SHA256_BLOCK_SIZE            64U
#define SHA256_DIGEST_SIZE           32U

/* ECDSA Configuration */
#define ECDSA_PUBLIC_KEY_SIZE        64U  /* 32 bytes X + 32 bytes Y (uncompressed) */
#define ECDSA_SIGNATURE_SIZE         64U  /* 32 bytes R + 32 bytes S */
#define ECDSA_CURVE_SECP256R1        1U

/* SHA-256 Context */
typedef struct
{
    uint32_t total[2];
    uint32_t state[8];
    uint8_t buffer[SHA256_BLOCK_SIZE];
} sha256_context_t;

/* ECDSA Public Key (secp256r1 */
typedef struct
{
    uint8_t x[32];  /* X coordinate */
    uint8_t y[32];  /* Y coordinate */
} ecdsa_public_key_t;

/* ECDSA Signature */
typedef struct
{
    uint8_t r[32];  /* R component */
    uint8_t s[32];  /* S component */
} ecdsa_signature_t;

/* SHA-256 Functions */
void Crypto_SHA256_Init(sha256_context_t *ctx);
void Crypto_SHA256_Update(sha256_context_t *ctx, const uint8_t *data, uint32_t length);
void Crypto_SHA256_Final(sha256_context_t *ctx, uint8_t digest[SHA256_DIGEST_SIZE]);
void Crypto_SHA256_Calculate(const uint8_t *data, uint32_t length, uint8_t digest[SHA256_DIGEST_SIZE]);
HAL_StatusTypeDef Crypto_SHA256_CalculateFromFlash(uint32_t flash_address, uint32_t length, uint8_t digest[SHA256_DIGEST_SIZE]);

/* ECDSA Functions */
bool Crypto_ECDSA_Verify(const uint8_t *hash, uint32_t hash_length,
                         const ecdsa_signature_t *signature,
                         const ecdsa_public_key_t *public_key);
bool Crypto_ECDSA_VerifySignature(const uint8_t *hash, uint32_t hash_length,
                                   const uint8_t *signature_data, uint32_t signature_length,
                                   const uint8_t *public_key_data, uint32_t public_key_length);

/* Public Key Management */
HAL_StatusTypeDef Crypto_LoadPublicKey(uint32_t flash_address, ecdsa_public_key_t *public_key);
bool Crypto_IsPublicKeyValid(const ecdsa_public_key_t *public_key);

#ifdef __cplusplus
}
#endif

#endif /* __CRYPTO_H */
