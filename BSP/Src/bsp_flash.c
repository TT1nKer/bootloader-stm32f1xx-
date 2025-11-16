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

/* Private function prototypes -----------------------------------------------*/

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



