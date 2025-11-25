/**
  ******************************************************************************
  * @file    ota_security.h
  * @brief   OTA security helpers (CRC + signature scaffolding)
  ******************************************************************************
  */

#ifndef __OTA_SECURITY_H
#define __OTA_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "ota_transport.h"

void OTA_Security_Init(void);
bool OTA_Security_ValidateCrc(uint32_t expected_crc, uint32_t actual_crc);
bool OTA_Security_ValidateSignature(const uint8_t *signature,
                                    uint32_t signature_length,
                                    uint32_t image_address,
                                    uint32_t image_size);
bool OTA_Security_CalculateSHA256(uint32_t image_address, uint32_t image_size, uint8_t hash[32]);

#ifdef __cplusplus
}
#endif

#endif /* __OTA_SECURITY_H */


