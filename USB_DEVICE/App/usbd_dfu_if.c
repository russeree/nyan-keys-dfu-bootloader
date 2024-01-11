/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_dfu_if.c
  * @brief          : Usb device for Download Firmware Update.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_dfu_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_DFU
  * @brief Usb DFU device module.
  * @{
  */

/** @defgroup USBD_DFU_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Defines
  * @brief Private defines.
  * @{
  */

#define FLASH_DESC_STR      "@Internal Flash   /0x08000000/03*016Ka,01*016Kg,01*064Kg,07*128Kg,04*016Kg,01*064Kg,07*128Kg"

/* USER CODE BEGIN PRIVATE_DEFINES */
#define FLASH_DESC_STR      "@Internal Flash   /0x08000000/01*016Ka,01*016Ka,01*016Kg,01*016Kg,01*064Kg,01*128Kg,01*128Kg,01*128Kg"
#define FLASH_ERASE_TIME    (uint16_t)2500000
#define FLASH_PROGRAM_TIME  (uint16_t)50
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceHS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static uint16_t MEM_If_Init_HS(void);
static uint16_t MEM_If_Erase_HS(uint32_t Add);
static uint16_t MEM_If_Write_HS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint8_t *MEM_If_Read_HS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint16_t MEM_If_DeInit_HS(void);
static uint16_t MEM_If_GetStatus_HS(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif

__ALIGN_BEGIN USBD_DFU_MediaTypeDef USBD_DFU_fops_HS __ALIGN_END =
{
    (uint8_t*)FLASH_DESC_STR,
    MEM_If_Init_HS,
    MEM_If_DeInit_HS,
    MEM_If_Erase_HS,
    MEM_If_Write_HS,
    MEM_If_Read_HS,
    MEM_If_GetStatus_HS
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Memory initialization routine.
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Init_HS(void)
{
  /* USER CODE BEGIN 6 */
  HAL_StatusTypeDef flash_ok = HAL_ERROR;

  //Делаем память открытой
  while(flash_ok != HAL_OK){
      flash_ok = HAL_FLASH_Unlock();
  }
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  De-Initializes Memory.
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_DeInit_HS(void)
{
  /* USER CODE BEGIN 7 */
  HAL_StatusTypeDef flash_ok = HAL_ERROR;

  flash_ok = HAL_ERROR;
  while(flash_ok != HAL_OK){
      flash_ok = HAL_FLASH_Lock();
  }
  return (USBD_OK);
  /* USER CODE END 7 */
}

/**
  * @brief  Erase sector.
  * @param  Add: Address of sector to be erased.
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Erase_HS(uint32_t Add)
{
  /* USER CODE BEGIN 8 */
  FLASH_EraseInitTypeDef erase;
  uint32_t error = 0;
  
  // Take the address and convert it to a sector
#if (FLASH_SECTOR_TOTAL == 8)
  if(!address_to_flash_sector(&Add, &erase.Sector)) {
    return (USBD_FAIL);
  }
#else
  return (USBD_BUSY);
#endif

  erase.TypeErase = FLASH_TYPEERASE_SECTORS;
  erase.NbSectors = 1;
  erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &error);

  if(HAL_OK != status) {
      return (USBD_BUSY);
  }

  return (USBD_OK);
  /* USER CODE END 8 */
}

/**
  * @brief  Memory write routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be written (in bytes).
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Write_HS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  /* USER CODE BEGIN 9 */
  uint32_t i = 0;

  for (i = 0; i < Len; i += 4) {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by byte */
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(dest + i), *(uint32_t *)(src + i)) == HAL_OK) {
      /* Check the written value */
      if (*(uint32_t *)(src + i) != *(uint32_t *)(dest + i)) {
        /* Flash content doesn't match SRAM content */
        return (USBD_FAIL);
      } 
    } else {
      /* Error occurred while writing data in Flash memory */
      return (USBD_FAIL);
    }
  }

  return (USBD_OK);
  /* USER CODE END 9 */
}

/**
  * @brief  Memory read routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint8_t *MEM_If_Read_HS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  /* Return a valid address to avoid HardFault */
  /* USER CODE BEGIN 10 */
  uint8_t *src_ptr = src;
  for(uint32_t i=0; i<Len; i++)
  {
      dest[i] = *src_ptr;
      src_ptr++;
  }

  return (uint8_t *)(dest);
  /* USER CODE END 10 */
}

/**
  * @brief  Get status routine.
  * @param  Add: Address to be read from.
  * @param  Cmd: Number of data to be read (in bytes).
  * @param  buffer: used for returning the time necessary for a program or an erase operation
  * @retval 0 if operation is successful
  */
uint16_t MEM_If_GetStatus_HS(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
{
  /* USER CODE BEGIN 11 */
  switch(Cmd) {
    case DFU_MEDIA_PROGRAM:
        buffer[1] = (uint8_t)FLASH_PROGRAM_TIME;
        buffer[2] = (uint8_t)(FLASH_PROGRAM_TIME << 8);
        buffer[3] = 0;
        break;
 
    case DFU_MEDIA_ERASE:
        buffer[1] = (uint8_t)FLASH_ERASE_TIME;
        buffer[2] = (uint8_t)(FLASH_ERASE_TIME << 8);
        buffer[3] = 0;
        break;
 
    default:
        break;
    }
    return  (USBD_OK);
  /* USER CODE END 11 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
bool address_to_flash_sector(uint32_t *address, uint32_t *sector) {
    // We return false on the  
    if (*address < SECTOR_0_BASE_AXIM) {
        return false;
    } else if (*address < SECTOR_1_BASE_AXIM) {
        return false;
    } else if (*address < SECTOR_2_BASE_AXIM) {
        return false;
    } else if (*address < SECTOR_3_BASE_AXIM) {
        *sector = FLASH_SECTOR_2;
    } else if (*address < SECTOR_4_BASE_AXIM) {
        *sector = FLASH_SECTOR_3;
    } else if (*address < SECTOR_5_BASE_AXIM) {
        *sector = FLASH_SECTOR_4;
    } else if (*address < SECTOR_6_BASE_AXIM) {
        *sector = FLASH_SECTOR_5;
    } else if (*address < SECTOR_7_BASE_AXIM) {
        *sector = FLASH_SECTOR_6;
    } else if (*address < SECTOR_7_BASE_AXIM + SECTOR_SIZE_128k) {
        *sector = FLASH_SECTOR_7;
    } else {
        return false; // Assuming address is within the range of the last sector
    }
    return true;
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

