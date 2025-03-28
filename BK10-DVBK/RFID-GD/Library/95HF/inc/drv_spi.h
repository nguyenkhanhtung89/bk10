/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __SPI_H
#define __SPI_H

/* Includes ----------------------------------------------------------------- */
#include "hw_config.h" 

#define SPI_RESPONSEBUFFER_SIZE		255

void SPI_SendByte(uint32_t SPIx, uint8_t data);
void SPI_SendWord(uint32_t SPIx, uint16_t data);

uint8_t SPI_SendReceiveByte		(uint32_t SPIx, uint8_t data); 
void 		SPI_SendReceiveBuffer	(uint32_t SPIx, uint8_t *pCommand, uint8_t length, uint8_t *pResponse);

#endif /* __SPI_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
