/**
  ******************************************************************************
  * @file    stm3210c_eval_sd.c
  * @author  MCD Application Team
  * @version V6.0.1
  * @date    18-December-2015
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

#include "stm3210c_eval_sd.h"

#define SD_DUMMY_BYTE   0xFF
#define SD_NO_RESPONSE_EXPECTED 0x80

static uint8_t SD_GetCIDRegister(SD_CID* Cid);
static uint8_t SD_GetCSDRegister(SD_CSD* Csd);
static SD_Info SD_GetDataResponse(void);
static uint8_t SD_GoIdleState(void);
static uint8_t SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Response);

void                      SD_IO_Init(void);
HAL_StatusTypeDef         SD_IO_WriteCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Response);
HAL_StatusTypeDef         SD_IO_WaitResponse(uint8_t Response);
void                      SD_IO_WriteDummy(void);
void                      SD_IO_WriteByte(uint8_t Data);
uint8_t                   SD_IO_ReadByte(void);

uint32_t SpixTimeout = EVAL_SPIx_TIMEOUT_MAX;   

#define SD_DETECT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOE_CLK_ENABLE()

void SD_IO_Init(void)
{
  GPIO_InitTypeDef  gpioinitstruct;
  uint8_t counter;

  /* SD_CS_GPIO and SD_DETECT_GPIO Periph clock enable */
  SD_CS_GPIO_CLK_ENABLE();
  SD_DETECT_GPIO_CLK_ENABLE();

  /* Configure SD_CS_PIN pin: SD Card CS pin */
  gpioinitstruct.Pin    = SD_CS_PIN;
  gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull   = GPIO_NOPULL;
  gpioinitstruct.Speed  = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SD_CS_GPIO_PORT, &gpioinitstruct);

 
  /*------------Put SD in SPI mode--------------*/
  /* SD SPI Config */
  SPI_2_init();
  
  /* SD chip select high */
  SD_CS_HIGH();
  
  /* Send dummy byte 0xFF, 10 times with CS high */
  /* Rise CS and MOSI for 80 clocks cycles */
  for (counter = 0; counter <= 9; counter++)
  {
    /* Send dummy byte 0xFF */
    SD_IO_WriteByte(SD_DUMMY_BYTE);
  }
}

/**
  * @brief  Write a byte on the SD.
  * @param  Data: byte to send.
  * @retval None
  */
void SD_IO_WriteByte(uint8_t Data)
{
  /* Send the byte */
  SPI_2_write(&Data);
}

/**
  * @brief  Read a byte from the SD.
  * @retval The received byte.
  */
uint8_t SD_IO_ReadByte(void)
{
  uint8_t data = 0;
  
  /* Get the received data */
  SPI_2_read(&data, 1);

  /* Return the shifted data */
  return data;
}

/**
  * @brief  Send 5 bytes command to the SD card and get response
  * @param  Cmd: The user expected command to send to SD card.
  * @param  Arg: The command argument.
  * @param  Crc: The CRC.
  * @param  Response: Expected response from the SD card
  * @retval  HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef SD_IO_WriteCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Response)
{
  uint32_t counter = 0x00;
  uint8_t frame[6];

  /* Prepare Frame to send */
  frame[0] = (Cmd | 0x40); /* Construct byte 1 */
  frame[1] = (uint8_t)(Arg >> 24); /* Construct byte 2 */
  frame[2] = (uint8_t)(Arg >> 16); /* Construct byte 3 */
  frame[3] = (uint8_t)(Arg >> 8); /* Construct byte 4 */
  frame[4] = (uint8_t)(Arg); /* Construct byte 5 */
  frame[5] = (Crc); /* Construct CRC: byte 6 */
  
  /* SD chip select low */
  SD_CS_LOW();
    
  /* Send Frame */
  for (counter = 0; counter < 6; counter++)
  {
    SD_IO_WriteByte(frame[counter]); /* Send the Cmd bytes */
  }
  SD_CS_HIGH();
  if(Response != SD_NO_RESPONSE_EXPECTED)
  {
    return SD_IO_WaitResponse(Response);
  }
  
  return HAL_OK;
}

/**
  * @brief  Wait response from the SD card
  * @param  Response: Expected response from the SD card
  * @retval  HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef SD_IO_WaitResponse(uint8_t Response)
{
  uint32_t timeout = 0xFFFF;
  uint8_t resp = 0;
  /* Check if response is got or a timeout is happen */
  resp = SD_IO_ReadByte();
  while ((resp != Response) && timeout)
  {
    timeout--;
    resp = SD_IO_ReadByte();
  }

  if (timeout == 0)
  {
    /* After time out */
    return HAL_TIMEOUT;
  }
  else
  {
    /* Right response got */
    return HAL_OK;
  }
}

/**
  * @brief  Send dummy byte with CS High
  * @retval None
  */
void SD_IO_WriteDummy(void)
{
    /* SD chip select high */
    SD_CS_HIGH();
    
    /* Send Dummy byte 0xFF */
    SD_IO_WriteByte(SD_DUMMY_BYTE);
}


__IO uint8_t SdStatus = SD_PRESENT;

 /**
  * @brief  Initializes the SD/SD communication.
  * @retval The SD Response: 
  *         - MSD_ERROR : Sequence failed
  *         - MSD_OK    : Sequence succeed
  */
uint8_t BSP_SD_Init(void)
{ 
  /* Configure IO functionalities for SD pin */
  SD_IO_Init();

  /* TODO - wHAT IF CARD IS NOT PRESENT? */
    SdStatus = SD_PRESENT;
  
  /* SD initialized and set to SPI mode properly */
  return (SD_GoIdleState());
}


/**
  * @brief  Returns information about specific card.
  * @param  pCardInfo: pointer to a SD_CardInfo structure that contains all SD 
  *         card information.
  * @retval The SD Response:
  *         - MSD_ERROR : Sequence failed
  *         - MSD_OK    : Sequence succeed
  */
uint8_t BSP_SD_GetCardInfo(SD_CardInfo *pCardInfo)
{
  uint8_t status = MSD_ERROR;

  SD_GetCSDRegister(&(pCardInfo->Csd));
  status = SD_GetCIDRegister(&(pCardInfo->Cid));
  pCardInfo->CardCapacity = (pCardInfo->Csd.DeviceSize + 1) ;
  pCardInfo->CardCapacity *= (1 << (pCardInfo->Csd.DeviceSizeMul + 2));
  pCardInfo->CardBlockSize = 1 << (pCardInfo->Csd.RdBlockLen);
  pCardInfo->CardCapacity *= pCardInfo->CardBlockSize;

  /* Returns the reponse */
  return status;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode. 
  * @param  p32Data: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read  
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumberOfBlocks: Number of SD blocks to read 
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks(uint32_t* p32Data, uint64_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t counter = 0, offset = 0;
  uint8_t rvalue = MSD_ERROR;
  uint8_t *pData = (uint8_t *)p32Data;
  
  /* Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and 
     Check if the SD acknowledged the set block length command: R1 response (0x00: no errors) */
  if (SD_IO_WriteCmd(SD_CMD_SET_BLOCKLEN, BlockSize, 0xFF, SD_RESPONSE_NO_ERROR) != HAL_OK)
  {
    return MSD_ERROR;
  }

  /* Data transfer */
  while (NumberOfBlocks--)
  {
    /* Send dummy byte: 8 Clock pulses of delay */
    SD_IO_WriteDummy();

    /* Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
    /* Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
    if (SD_IO_WriteCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr + offset, 0xFF, SD_RESPONSE_NO_ERROR) != HAL_OK)
    {
      return MSD_ERROR;
    }

    /* Now look for the data token to signify the start of the data */
    if (SD_IO_WaitResponse(SD_START_DATA_SINGLE_BLOCK_READ) == HAL_OK)
    {
      /* Read the SD block data : read NumByteToRead data */
      for (counter = 0; counter < BlockSize; counter++)
      {
        /* Read the pointed data */
        *pData = SD_IO_ReadByte();
        /* Point to the next location where the byte read will be saved */
        pData++;
      }
      /* Set next read address*/
      offset += BlockSize;
      /* get CRC bytes (not really needed by us, but required by SD) */
      SD_IO_ReadByte();
      SD_IO_ReadByte();
      /* Set response value to success */
      rvalue = MSD_OK;
    }
    else
    {
      /* Set response value to failure */
      rvalue = MSD_ERROR;
    }
  }
  
  /* Send dummy byte: 8 Clock pulses of delay */
  SD_IO_WriteDummy();
  /* Returns the reponse */
  return rvalue;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode. 
  * @param  p32Data: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written  
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumberOfBlocks: Number of SD blocks to write
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks(uint32_t* p32Data, uint64_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t counter = 0, offset = 0;
  uint8_t rvalue = MSD_ERROR;
  uint8_t *pData = (uint8_t *)p32Data;
  
  /* Data transfer */
  while (NumberOfBlocks--)
  {
    /* Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks  and
       Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
    if (SD_IO_WriteCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr + offset, 0xFF, SD_RESPONSE_NO_ERROR) != HAL_OK)
    {
      return MSD_ERROR;
    }

    /* Send dummy byte */
    SD_IO_WriteByte(SD_DUMMY_BYTE);
    SD_CS_LOW();
    /* Send the data token to signify the start of the data */
    SD_IO_WriteByte(SD_START_DATA_SINGLE_BLOCK_WRITE);

    /* Write the block data to SD : write count data by block */
    for (counter = 0; counter < BlockSize; counter++)
    {
      /* Send the pointed byte */
      SD_IO_WriteByte(*pData);
      
      /* Point to the next location where the byte read will be saved */
      pData++;
    }

    /* Set next write address */
    offset += BlockSize;
    SD_CS_HIGH();
    /* XXX: THIS MAY FAIL! */
    /* Put CRC bytes (not really needed by us, but required by SD) */
    SD_IO_ReadByte();
    SD_IO_ReadByte();

    /* Read data response */
    if (SD_GetDataResponse() == SD_DATA_OK)
    {
      /* Set response value to success */
      rvalue = MSD_OK;
    }
    else
    {
      /* Set response value to failure */
      rvalue = MSD_ERROR;
    }
  }

  /* Send dummy byte: 8 Clock pulses of delay */
  SD_IO_WriteDummy();

  /* Returns the reponse */
  return rvalue;
}

/**
  * @brief  Read the CSD card register.
  *         Reading the contents of the CSD register in SPI mode is a simple 
  *         read-block transaction.
  * @param  Csd: pointer on an SCD register structure
  * @retval SD status
  */
uint8_t SD_GetCSDRegister(SD_CSD* Csd)
{
  uint32_t counter = 0;
  uint8_t rvalue = MSD_ERROR;
  uint8_t CSD_Tab[16];

  /* Send CMD9 (CSD register) or CMD10(CSD register) and Wait for response in the R1 format (0x00 is no errors) */
  if (SD_IO_WriteCmd(SD_CMD_SEND_CSD, 0, 0xFF, SD_RESPONSE_NO_ERROR) == HAL_OK)
  {
    if (SD_IO_WaitResponse(SD_START_DATA_SINGLE_BLOCK_READ) == HAL_OK)
    {
      for (counter = 0; counter < 16; counter++)
      {
        /* Store CSD register value on CSD_Tab */
        CSD_Tab[counter] = SD_IO_ReadByte();
      }

      /* Get CRC bytes (not really needed by us, but required by SD) */
      SD_IO_WriteByte(SD_DUMMY_BYTE);
      SD_IO_WriteByte(SD_DUMMY_BYTE);

      /* Set response value to success */
      rvalue = MSD_OK;
    }
  }
  /* Send dummy byte: 8 Clock pulses of delay */
  SD_IO_WriteDummy();

  if(rvalue == SD_RESPONSE_NO_ERROR)
  {
    /* Byte 0 */
    Csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
    Csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
    Csd->Reserved1 = CSD_Tab[0] & 0x03;

    /* Byte 1 */
    Csd->TAAC = CSD_Tab[1];

    /* Byte 2 */
    Csd->NSAC = CSD_Tab[2];

    /* Byte 3 */
    Csd->MaxBusClkFrec = CSD_Tab[3];

    /* Byte 4 */
    Csd->CardComdClasses = CSD_Tab[4] << 4;

    /* Byte 5 */
    Csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
    Csd->RdBlockLen = CSD_Tab[5] & 0x0F;

    /* Byte 6 */
    Csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
    Csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
    Csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
    Csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
    Csd->Reserved2 = 0; /*!< Reserved */

    Csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;

    /* Byte 7 */
    Csd->DeviceSize |= (CSD_Tab[7]) << 2;

    /* Byte 8 */
    Csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;

    Csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
    Csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);

    /* Byte 9 */
    Csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
    Csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
    Csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
    /* Byte 10 */
    Csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
      
    Csd->EraseGrSize = (CSD_Tab[10] & 0x40) >> 6;
    Csd->EraseGrMul = (CSD_Tab[10] & 0x3F) << 1;

    /* Byte 11 */
    Csd->EraseGrMul |= (CSD_Tab[11] & 0x80) >> 7;
    Csd->WrProtectGrSize = (CSD_Tab[11] & 0x7F);

    /* Byte 12 */
    Csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
    Csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
    Csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
    Csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;

    /* Byte 13 */
    Csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xC0) >> 6;
    Csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
    Csd->Reserved3 = 0;
    Csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);

    /* Byte 14 */
    Csd->FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
    Csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
    Csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
    Csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
    Csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
    Csd->ECC = (CSD_Tab[14] & 0x03);

    /* Byte 15 */
    Csd->CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
    Csd->Reserved4 = 1;
  }
  /* Return the reponse */
  return rvalue;
}

/**
  * @brief  Read the CID card register.
  *         Reading the contents of the CID register in SPI mode is a simple 
  *         read-block transaction.
  * @param  Cid: pointer on an CID register structure
  * @retval SD status
  */
static uint8_t SD_GetCIDRegister(SD_CID* Cid)
{
  uint32_t counter = 0;
  uint8_t rvalue = MSD_ERROR;
  uint8_t CID_Tab[16];
  
  /* Send CMD10 (CID register) and Wait for response in the R1 format (0x00 is no errors) */
  if (SD_IO_WriteCmd(SD_CMD_SEND_CID, 0, 0xFF, SD_RESPONSE_NO_ERROR) == HAL_OK)
  {
    if (SD_IO_WaitResponse(SD_START_DATA_SINGLE_BLOCK_READ) == HAL_OK)
    {
      /* Store CID register value on CID_Tab */
      for (counter = 0; counter < 16; counter++)
      {
        CID_Tab[counter] = SD_IO_ReadByte();
      }

      /* Get CRC bytes (not really needed by us, but required by SD) */
      SD_IO_WriteByte(SD_DUMMY_BYTE);
      SD_IO_WriteByte(SD_DUMMY_BYTE);

      /* Set response value to success */
      rvalue = MSD_OK;
    }
  }

  /* Send dummy byte: 8 Clock pulses of delay */
  SD_IO_WriteDummy();

  if(rvalue == MSD_OK)
  {
    /* Byte 0 */
    Cid->ManufacturerID = CID_Tab[0];

    /* Byte 1 */
    Cid->OEM_AppliID = CID_Tab[1] << 8;

    /* Byte 2 */
    Cid->OEM_AppliID |= CID_Tab[2];

    /* Byte 3 */
    Cid->ProdName1 = CID_Tab[3] << 24;

    /* Byte 4 */
    Cid->ProdName1 |= CID_Tab[4] << 16;

    /* Byte 5 */
    Cid->ProdName1 |= CID_Tab[5] << 8;

    /* Byte 6 */
    Cid->ProdName1 |= CID_Tab[6];

    /* Byte 7 */
    Cid->ProdName2 = CID_Tab[7];

    /* Byte 8 */
    Cid->ProdRev = CID_Tab[8];

    /* Byte 9 */
    Cid->ProdSN = CID_Tab[9] << 24;

    /* Byte 10 */
    Cid->ProdSN |= CID_Tab[10] << 16;

    /* Byte 11 */
    Cid->ProdSN |= CID_Tab[11] << 8;

    /* Byte 12 */
    Cid->ProdSN |= CID_Tab[12];

    /* Byte 13 */
    Cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
    Cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;

    /* Byte 14 */
    Cid->ManufactDate |= CID_Tab[14];

    /* Byte 15 */
    Cid->CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
    Cid->Reserved2 = 1;
  }
  /* Return the reponse */
  return rvalue;
}

/**
  * @brief  Send 5 bytes command to the SD card and get response
  * @param  Cmd: The user expected command to send to SD card.
  * @param  Arg: The command argument.
  * @param  Crc: The CRC.
  * @param  Response: Expected response from the SD card
  * @retval SD status
  */
static uint8_t SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Response)
{
  uint8_t status = MSD_ERROR;
  
  if(SD_IO_WriteCmd(Cmd, Arg, Crc, Response) == HAL_OK)
  {
    status = MSD_OK;
  }
  
  /* Send Dummy Byte */
  SD_IO_WriteDummy();

  return status;
}

/**
  * @brief  Get SD card data response.
  * @retval The SD status: Read data response xxx0<status>1
  *         - status 010: Data accecpted
  *         - status 101: Data rejected due to a crc error
  *         - status 110: Data rejected due to a Write error.
  *         - status 111: Data rejected due to other error.
  */
static SD_Info SD_GetDataResponse(void)
{
  uint32_t counter = 0;
  SD_Info response, rvalue;

  while (counter <= 64)
  {
    /* Read response */
    response = (SD_Info)SD_IO_ReadByte();
    /* Mask unused bits */
    response &= 0x1F;
    switch (response)
    {
      case SD_DATA_OK:
      {
        rvalue = SD_DATA_OK;
        break;
      }
      case SD_DATA_CRC_ERROR:
        return SD_DATA_CRC_ERROR;
      case SD_DATA_WRITE_ERROR:
        return SD_DATA_WRITE_ERROR;
      default:
      {
        rvalue = SD_DATA_OTHER_ERROR;
        break;
      }
    }
    /* Exit loop in case of data ok */
    if (rvalue == SD_DATA_OK)
      break;
    /* Increment loop counter */
    counter++;
  }

  /* Wait null data */
  while (SD_IO_ReadByte() == 0);

  /* Return response */
  return response;
}

/**
  * @brief  Returns the SD status.
  * @retval The SD status.
  */
uint8_t BSP_SD_GetStatus(void)
{
  return MSD_OK;
}

/**
  * @brief  Put SD in Idle state.
  * @retval SD status
  */
static uint8_t SD_GoIdleState(void)
{
  /* Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode and 
     Wait for In Idle State Response (R1 Format) equal to 0x01 */
  if (SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95, SD_IN_IDLE_STATE) != MSD_OK)
  {
    /* No Idle State Response: return response failure */
    return MSD_ERROR;
  }

  /*----------Activates the card initialization process-----------*/
  /* Send CMD1 (Activates the card process) until response equal to 0x0 and
     Wait for no error Response (R1 Format) equal to 0x00 */
  while (SD_SendCmd(SD_CMD_SEND_OP_COND, 0, 0xFF, SD_RESPONSE_NO_ERROR) != MSD_OK);
  
  return MSD_OK;
}
/**
  * @brief  Erases the specified memory area of the given SD card. 
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval SD status
  */
uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
{
  uint8_t rvalue = MSD_ERROR;

  /* Send CMD32 (Erase group start) and check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
  if (SD_SendCmd(SD_CMD_SD_ERASE_GRP_START, StartAddr, 0xFF, SD_RESPONSE_NO_ERROR) == MSD_OK)
  {
    /* Send CMD33 (Erase group end) and Check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
    if (SD_SendCmd(SD_CMD_SD_ERASE_GRP_END, EndAddr, 0xFF, SD_RESPONSE_NO_ERROR) == MSD_OK)
    {
      /* Send CMD38 (Erase) and Check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
      if (SD_SendCmd(SD_CMD_ERASE, 0, 0xFF, SD_RESPONSE_NO_ERROR) == MSD_OK)
      {
          rvalue = MSD_OK;
      }
    }
  }
  
  /* Return the reponse */
  return rvalue;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
