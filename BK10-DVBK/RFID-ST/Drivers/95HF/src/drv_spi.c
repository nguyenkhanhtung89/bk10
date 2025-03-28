/* Includes ------------------------------------------------------------------*/
#include "drv_spi.h"

void SPI_SendByte(SPI_TypeDef* SPIx, uint8_t data){
	/* Wait for SPIx Tx buffer empty */
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
	
	/* Send byte through the SPIx peripheral */
	SPI_SendData8(SPIx, data);
}

/**  
 *	@brief  Sends one word over SPI
 *  @param  SPIx : where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  data : data to send	(16 bit)
 *  @retval None
 */
void SPI_SendWord(SPI_TypeDef* SPIx, uint16_t data){
	/* Wait for SPIx Tx buffer empty */	
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
	
	/* Send byte through the SPIx peripheral */
	SPI_SendData8(SPIx, data);
}

/**  
 *	@brief  Sends one byte over SPI and recovers a response
 *  @param  SPIx : where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  data : data to send
 *  @retval data response from SPIx
 */
uint8_t SPI_SendReceiveByte(SPI_TypeDef* SPIx, uint8_t data){
	int32_t tick, delayPeriod;
	
	tick = osKernelSysTick(); 
	delayPeriod = osKernelSysTickMicroSec(50 * 1000); //200 milisecond timeout
	
	/* Wait for SPI1 Tx buffer empty */
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET && ((osKernelSysTick() - tick) < delayPeriod));
	
	/* Send byte through the SPI1 peripheral */
	SPI_SendData8(SPIx, data);	
	
	/* Wait for SPI1 data reception	*/
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET && ((osKernelSysTick() - tick) < delayPeriod));
	
	/* Read & return SPI1 received data	*/
	return SPI_ReceiveData8(SPIx);
}

/**
 *	@brief  reveive a byte array over SPI
 *  @param  SPIx	 	: where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  pCommand  	: pointer on the buffer to send
 *  @param  length	 	: length of the buffer to send
 *  @param  pResponse 	: pointer on the buffer response
 *  @retval None 
 */
void SPI_SendReceiveBuffer(SPI_TypeDef* SPIx, uc8 *pCommand, uint8_t length, uint8_t *pResponse){
	uint8_t i;
	
	/* the buffer size is limited to SPI_RESPONSEBUFFER_SIZE */
	length = MIN (SPI_RESPONSEBUFFER_SIZE,length);
	for(i=0; i<length; i++){
		pResponse[i] = SPI_SendReceiveByte(SPIx, pCommand[i]);
		osDelay(5);
	}
}

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
