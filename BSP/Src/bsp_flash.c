/**
  ******************************************************************************
  * @file    bsp_flash.c
  * @brief   Board Support Package - Flash operations implementation
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
#include "bsp_flash.h"
#include "bsp_ota_meta.h"

/* Private function prototypes -----------------------------------------------*/
static bool IsAddressRangeValid(uint32_t address, uint32_t length);
static bool IsPageAligned(uint32_t address);
static bool ResolveBankRange(uint32_t bank_index,
                             uint32_t offset,
                             uint32_t length,
                             uint32_t *resolved_address);
static uint32_t Crc32Update(uint32_t current_crc, uint8_t data_byte);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Erase Flash pages
  * @param  address: Start address (must be page aligned)
  * @param  size: Size to erase (in bytes)
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_Flash_Erase(uint32_t address, uint32_t size)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    
    // Calculate number of pages to erase
    uint32_t num_pages = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

    if ((size == 0U) || !IsPageAligned(address) ||
        (size % FLASH_PAGE_SIZE) != 0U ||
        !IsAddressRangeValid(address, size))
    {
        return HAL_ERROR;
    }
    
    // Unlock Flash
    HAL_FLASH_Unlock();
    
    // Configure erase structure
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = address;
    EraseInitStruct.NbPages = num_pages;
    
    // Erase pages
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    
    // Lock Flash
    HAL_FLASH_Lock();
    
    return status;
}

/**
  * @brief  Write data to Flash
  * @param  address: Destination address (must be half-word aligned)
  * @param  data: Source data buffer
  * @param  length: Number of bytes to write
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_Flash_Write(uint32_t address, uint8_t *data, uint32_t length)
{
    HAL_StatusTypeDef status = HAL_OK;

    if ((length == 0U) || (data == NULL) ||
        (address & 0x1U) != 0U ||
        !IsAddressRangeValid(address, length))
    {
        return HAL_ERROR;
    }
    
    // Unlock Flash
    HAL_FLASH_Unlock();
    
    // Write data (STM32F1 requires half-word programming)
    for (uint32_t i = 0; i < length; i += 2)
    {
        uint16_t halfword;
        
        if (i + 1 < length)
        {
            // Two bytes available
            halfword = data[i] | (data[i + 1] << 8);
        }
        else
        {
            // Only one byte available, pad with 0xFF
            halfword = data[i] | 0xFF00;
        }
        
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 
                                   address + i, 
                                   halfword);
        
        if (status != HAL_OK)
        {
            break;
        }
    }
    
    // Lock Flash
    HAL_FLASH_Lock();
    
    return status;
}

/**
  * @brief  Read data from Flash
  * @param  address: Source address
  * @param  data: Destination buffer
  * @param  length: Number of bytes to read
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef BSP_Flash_Read(uint32_t address, uint8_t *data, uint32_t length)
{
    if ((length == 0U) || (data == NULL) ||
        !IsAddressRangeValid(address, length))
    {
        return HAL_ERROR;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        data[i] = *((volatile uint8_t*)(address + i));
    }
    
    return HAL_OK;
}

/**
  * @brief  Verify Flash content
  * @param  address: Flash address to verify
  * @param  expected_data: Expected data
  * @param  length: Number of bytes to verify
  * @retval true if verification passes, false otherwise
  */
bool BSP_Flash_Verify(uint32_t address, uint8_t *expected_data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        uint8_t read_data = *((volatile uint8_t*)(address + i));
        if (read_data != expected_data[i])
        {
            return false;
        }
    }
    
    return true;
}

/**
  * @brief  Read a word (32-bit) from Flash
  * @param  address: Flash address (must be word aligned)
  * @retval 32-bit word value
  */
uint32_t BSP_Flash_ReadWord(uint32_t address)
{
    return *((volatile uint32_t*)address);
}

/* Private functions --------------------------------------------------------*/

static bool IsAddressRangeValid(uint32_t address, uint32_t length)
{
    if (length == 0U)
    {
        return false;
    }

    uint32_t end_address = address + length - 1U;

    bool in_bank0 = (address >= OTA_BANK0_START_ADDRESS) && (end_address <= OTA_BANK0_END_ADDRESS);
    bool in_bank1 = (address >= OTA_BANK1_START_ADDRESS) && (end_address <= OTA_BANK1_END_ADDRESS);
    bool in_config = (address >= CONFIG_AREA_ADDRESS) && (end_address <= CONFIG_AREA_END_ADDRESS);
    bool in_metadata = (address >= OTA_METADATA_ADDRESS) && (end_address <= OTA_METADATA_END_ADDRESS);

    return in_bank0 || in_bank1 || in_config || in_metadata;
}

static bool IsPageAligned(uint32_t address)
{
    return ((address % FLASH_PAGE_SIZE) == 0U);
}

HAL_StatusTypeDef BSP_Flash_EraseBank(uint32_t bank_index)
{
    uint32_t bank_start = BSP_OtaMeta_GetBankStart(bank_index);
    if (bank_start == 0U)
    {
        return HAL_ERROR;
    }

    FLASH_EraseInitTypeDef erase = {0};
    uint32_t page_error = 0U;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = bank_start;
    erase.NbPages = OTA_BANK_SIZE / FLASH_PAGE_SIZE;

    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &page_error);
    HAL_FLASH_Lock();
    return status;
}

bool BSP_Flash_IsBankRangeValid(uint32_t bank_index, uint32_t offset, uint32_t length)
{
    return ResolveBankRange(bank_index, offset, length, NULL);
}

HAL_StatusTypeDef BSP_Flash_WriteChunk(uint32_t bank_index,
                                       uint32_t offset,
                                       const uint8_t *data,
                                       uint32_t length,
                                       uint32_t *running_crc)
{
    if ((data == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    uint32_t base_address = 0U;
    if (!ResolveBankRange(bank_index, offset, length, &base_address))
    {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_OK;
    HAL_FLASH_Unlock();

    for (uint32_t i = 0; i < length; i += 2U)
    {
        uint16_t halfword;

        if (i + 1U < length)
        {
            halfword = data[i] | (data[i + 1U] << 8);
        }
        else
        {
            halfword = data[i] | 0xFF00U;
        }

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                                   base_address + i,
                                   halfword);
        if (status != HAL_OK)
        {
            break;
        }
    }

    HAL_FLASH_Lock();

    if ((status == HAL_OK) && (running_crc != NULL))
    {
        *running_crc = BSP_Flash_CalcCRC32(*running_crc, data, length);
    }

    return status;
}

uint32_t BSP_Flash_CalcCRC32(uint32_t current_crc, const uint8_t *data, uint32_t length)
{
    if (data == NULL || length == 0U)
    {
        return current_crc;
    }

    uint32_t crc = ~current_crc;
    for (uint32_t i = 0; i < length; ++i)
    {
        crc = Crc32Update(crc, data[i]);
    }

    return ~crc;
}

static bool ResolveBankRange(uint32_t bank_index,
                             uint32_t offset,
                             uint32_t length,
                             uint32_t *resolved_address)
{
    if (length == 0U)
    {
        return false;
    }

    if (!BSP_OtaMeta_IsBankIndexValid(bank_index))
    {
        return false;
    }

    if ((offset + length) > OTA_BANK_SIZE)
    {
        return false;
    }

    uint32_t base = BSP_OtaMeta_GetBankStart(bank_index);
    if (base == 0U)
    {
        return false;
    }

    if (resolved_address != NULL)
    {
        *resolved_address = base + offset;
    }

    return true;
}

static uint32_t Crc32Update(uint32_t current_crc, uint8_t data_byte)
{
    uint32_t crc = current_crc ^ data_byte;

    for (uint32_t i = 0U; i < 8U; ++i)
    {
        if (crc & 1U)
        {
            crc = (crc >> 1U) ^ 0xEDB88320U;
        }
        else
        {
            crc >>= 1U;
        }
    }

    return crc;
}