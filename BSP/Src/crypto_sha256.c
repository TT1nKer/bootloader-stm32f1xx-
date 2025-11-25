/**
  ******************************************************************************
  * @file    crypto_sha256.c
  * @brief   SHA-256 implementation for OTA signature verification
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

/* Includes ------------------------------------------------------------------*/
#include "crypto.h"
#include "main.h"
#include "bootloader_config.h"
#include <string.h>

/* Private helper macros (must be defined before use) */
#define PUT_UINT32_BE(n, b, i) \
{ \
    (b)[(i)] = (uint8_t)((n) >> 24); \
    (b)[(i) + 1] = (uint8_t)((n) >> 16); \
    (b)[(i) + 2] = (uint8_t)((n) >> 8); \
    (b)[(i) + 3] = (uint8_t)((n)); \
}

#define GET_UINT32_BE(n, b, i) \
{ \
    (n) = ((uint32_t)(b)[(i)] << 24) | \
          ((uint32_t)(b)[(i) + 1] << 16) | \
          ((uint32_t)(b)[(i) + 2] << 8) | \
          ((uint32_t)(b)[(i) + 3]); \
}

/* SHA-256 Constants */
#define SHA256_K_SIZE                 64U

static const uint32_t sha256_k[SHA256_K_SIZE] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* Private functions */
static void sha256_transform(sha256_context_t *ctx, const uint8_t data[SHA256_BLOCK_SIZE]);
static uint32_t rotr(uint32_t x, uint32_t n);
static uint32_t ch(uint32_t x, uint32_t y, uint32_t z);
static uint32_t maj(uint32_t x, uint32_t y, uint32_t z);
static uint32_t sigma0(uint32_t x);
static uint32_t sigma1(uint32_t x);
static uint32_t gamma0(uint32_t x);
static uint32_t gamma1(uint32_t x);

/**
  * @brief  Initialize SHA-256 context
  * @param  ctx: SHA-256 context
  * @retval None
  */
void Crypto_SHA256_Init(sha256_context_t *ctx)
{
    if (ctx == NULL) {
        return;
    }

    memset(ctx, 0, sizeof(sha256_context_t));

    // Initial hash values
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

/**
  * @brief  Update SHA-256 with data
  * @param  ctx: SHA-256 context
  * @param  data: Data to hash
  * @param  length: Data length
  * @retval None
  */
void Crypto_SHA256_Update(sha256_context_t *ctx, const uint8_t *data, uint32_t length)
{
    uint32_t left, fill;

    if (ctx == NULL || data == NULL || length == 0) {
        return;
    }

    left = ctx->total[0] & 0x3F;
    fill = SHA256_BLOCK_SIZE - left;

    ctx->total[0] += length;
    if (ctx->total[0] < length) {
        ctx->total[1]++;
    }

    if (left && length >= fill) {
        memcpy((void *)(ctx->buffer + left), data, fill);
        sha256_transform(ctx, ctx->buffer);
        length -= fill;
        data += fill;
        left = 0;
    }

    while (length >= SHA256_BLOCK_SIZE) {
        sha256_transform(ctx, data);
        length -= SHA256_BLOCK_SIZE;
        data += SHA256_BLOCK_SIZE;
    }

    if (length > 0) {
        memcpy((void *)(ctx->buffer + left), data, length);
    }
}

/**
  * @brief  Finalize SHA-256 and get digest
  * @param  ctx: SHA-256 context
  * @param  digest: Output digest (32 bytes)
  * @retval None
  */
void Crypto_SHA256_Final(sha256_context_t *ctx, uint8_t digest[SHA256_DIGEST_SIZE])
{
    uint32_t last, padn;
    uint8_t msglen[8];
    uint32_t high, low;

    if (ctx == NULL || digest == NULL) {
        return;
    }

    high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
    low = (ctx->total[0] << 3);

    PUT_UINT32_BE(high, msglen, 0);
    PUT_UINT32_BE(low, msglen, 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56) ? (56 - last) : (120 - last);

    Crypto_SHA256_Update(ctx, (uint8_t *)"\x80", 1);
    while (padn > 0) {
        Crypto_SHA256_Update(ctx, (uint8_t *)"\x00", 1);
        padn--;
    }
    Crypto_SHA256_Update(ctx, msglen, 8);

    PUT_UINT32_BE(ctx->state[0], digest, 0);
    PUT_UINT32_BE(ctx->state[1], digest, 4);
    PUT_UINT32_BE(ctx->state[2], digest, 8);
    PUT_UINT32_BE(ctx->state[3], digest, 12);
    PUT_UINT32_BE(ctx->state[4], digest, 16);
    PUT_UINT32_BE(ctx->state[5], digest, 20);
    PUT_UINT32_BE(ctx->state[6], digest, 24);
    PUT_UINT32_BE(ctx->state[7], digest, 28);
}

/**
  * @brief  Calculate SHA-256 hash of data
  * @param  data: Data to hash
  * @param  length: Data length
  * @param  digest: Output digest buffer (32 bytes)
  * @retval None
  */
void Crypto_SHA256_Calculate(const uint8_t *data, uint32_t length, uint8_t digest[SHA256_DIGEST_SIZE])
{
    sha256_context_t ctx;

    Crypto_SHA256_Init(&ctx);
    Crypto_SHA256_Update(&ctx, data, length);
    Crypto_SHA256_Final(&ctx, digest);
}

/**
  * @brief  Calculate SHA-256 hash from Flash memory
  * @param  flash_address: Flash address to hash
  * @param  length: Data length
  * @param  digest: Output digest buffer (32 bytes)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef Crypto_SHA256_CalculateFromFlash(uint32_t flash_address, uint32_t length, uint8_t digest[SHA256_DIGEST_SIZE])
{
    sha256_context_t ctx;
    uint8_t buffer[256];
    uint32_t remaining = length;
    uint32_t offset = 0;

    Crypto_SHA256_Init(&ctx);

    while (remaining > 0) {
        uint32_t chunk_size = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining;

        // Read from flash
        memcpy(buffer, (const void *)(flash_address + offset), chunk_size);

        Crypto_SHA256_Update(&ctx, buffer, chunk_size);

        remaining -= chunk_size;
        offset += chunk_size;
    }

    Crypto_SHA256_Final(&ctx, digest);

    return HAL_OK;
}

/* Private functions */
static uint32_t rotr(uint32_t x, uint32_t n)
{
    return ((x >> n) | (x << (32 - n)));
}

static uint32_t ch(uint32_t x, uint32_t y, uint32_t z)
{
    return ((x & y) ^ (~x & z));
}

static uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
{
    return ((x & y) ^ (x & z) ^ (y & z));
}

static uint32_t sigma0(uint32_t x)
{
    return (rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22));
}

static uint32_t sigma1(uint32_t x)
{
    return (rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25));
}

static uint32_t gamma0(uint32_t x)
{
    return (rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3));
}

static uint32_t gamma1(uint32_t x)
{
    return (rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10));
}

static void sha256_transform(sha256_context_t *ctx, const uint8_t data[SHA256_BLOCK_SIZE])
{
    uint32_t S[8], W[64], t0, t1;
    uint32_t i;

    // Copy state
    for (i = 0; i < 8; i++) {
        S[i] = ctx->state[i];
    }

    // Prepare message schedule
    for (i = 0; i < 16; i++) {
        GET_UINT32_BE(W[i], data, 4 * i);
    }
    for (i = 16; i < 64; i++) {
        W[i] = gamma1(W[i - 2]) + W[i - 7] + gamma0(W[i - 15]) + W[i - 16];
    }

    // Main loop
    for (i = 0; i < 64; i++) {
        t1 = S[7] + sigma1(S[4]) + ch(S[4], S[5], S[6]) + sha256_k[i] + W[i];
        t0 = sigma0(S[0]) + maj(S[0], S[1], S[2]);
        S[7] = S[6];
        S[6] = S[5];
        S[5] = S[4];
        S[4] = S[3] + t1;
        S[3] = S[2];
        S[2] = S[1];
        S[1] = S[0];
        S[0] = t0 + t1;
    }

    // Add to state
    for (i = 0; i < 8; i++) {
        ctx->state[i] += S[i];
    }
}
