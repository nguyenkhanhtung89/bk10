/* Includes ------------------------------------------------------------------*/
#include "drv_spi.h"
#include "gd32f30x.h"

void SPI_SendByte(uint32_t SPIx, uint8_t data){
	/* Wait for SPIx Tx buffer empty */
	while(spi_i2s_flag_get(SPIx, SPI_FLAG_TBE) == RESET);
	
	/* Send byte through the SPIx peripheral */
	spi_i2s_data_transmit(SPIx, data);
}

/**  
 *	@brief  Sends one word over SPI
 *  @param  SPIx : where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  data : data to send	(16 bit)
 *  @retval None
 */
void SPI_SendWord(uint32_t SPIx, uint16_t data){
	/* Wait for SPIx Tx buffer empty */	
	while(spi_i2s_flag_get(SPIx, SPI_FLAG_TBE) == RESET);
	
	/* Send byte through the SPIx peripheral */
	spi_i2s_data_transmit(SPIx, (uint8_t)data);
}

/**  
 *	@brief  Sends one byte over SPI and recovers a response
 *  @param  SPIx : where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  data : data to send
 *  @retval data response from SPIx
 */
uint8_t SPI_SendReceiveByte(uint32_t SPIx, uint8_t data){
	int32_t tick, delayPeriod;
	
	tick = osKernelSysTick(); 
	delayPeriod = osKernelSysTickMicroSec(50 * 1000); //200 milisecond timeout
	
	/* Wait for SPI1 Tx buffer empty */
	while(spi_i2s_flag_get(SPIx, SPI_FLAG_TBE) == RESET && ((osKernelSysTick() - tick) < delayPeriod));
	
	/* Send byte through the SPI1 peripheral */
	spi_i2s_data_transmit(SPIx, data);	
	
	/* Wait for SPI1 data reception	*/
	while(spi_i2s_flag_get(SPIx, SPI_FLAG_RBNE) == RESET && ((osKernelSysTick() - tick) < delayPeriod));
	
	/* Read & return SPI1 received data	*/
	return spi_i2s_data_receive(SPIx);
}

/**
 *	@brief  reveive a byte array over SPI
 *  @param  SPIx	 	: where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  pCommand  	: pointer on the buffer to send
 *  @param  length	 	: length of the buffer to send
 *  @param  pResponse 	: pointer on the buffer response
 *  @retval None 
 */
void SPI_SendReceiveBuffer(uint32_t SPIx, uint8_t *pCommand, uint8_t length, uint8_t *pResponse){
	uint8_t i;
	
	/* the buffer size is limited to SPI_RESPONSEBUFFER_SIZE */
	length = MIN (SPI_RESPONSEBUFFER_SIZE,length);
	for(i=0; i<length; i++){
		pResponse[i] = SPI_SendReceiveByte(SPIx, pCommand[i]);
		osDelay(5);
	}
}

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
