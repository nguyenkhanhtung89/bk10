#include <stdio.h>
#include "drv_95HF.h"

/* drv95HF_Private_Functions */
static void drv95HF_SendSPIResetByte(void);
static int8_t drv95HF_SPIPollingCommand(void);
static uint8_t DummyBuffer[MAX_BUFFER_SIZE];

static void drv95HF_SendSPIResetByte(void){
	/* Send reset control byte */
	SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_RESET);
}

void drv95HF_ResetSPI(void){
	/* Deselect Rftransceiver over SPI */
	RFTRANS_95HF_SELECT_HIGH();
	osDelay(1);
	/* Select 95HF device over SPI */
	RFTRANS_95HF_SELECT_LOW();
	osDelay(2);
	/* Send reset control byte	*/
	drv95HF_SendSPIResetByte();
	/* Deselect 95HF device over SPI */
	RFTRANS_95HF_SELECT_HIGH();
	osDelay(3);
	
	/* send a pulse on IRQ_in to wake-up 95HF device */
	drv95HF_SendIRQINPulse();
	osDelay(10);
}

void drv95HF_InitializeSPI(void){
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_AF);
	
	/* PB12 duoi dang NSS */
	gpio_init (GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12); 

	/* SPI1 cau hinh GPIO: SCK / PB13, MISO / PB14, MOSI / PB15 */
	gpio_init (GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_15);
	gpio_init (GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
	
	RFTRANS_95HF_SELECT_HIGH();
	
	rcu_periph_clock_enable(POWER_95HF_GPIO_CLK);
	gpio_init(POWER_95HF_GPIO_PORT,GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, POWER_95HF_PIN);
	gpio_bit_set(POWER_95HF_GPIO_PORT,POWER_95HF_PIN);
	
	rcu_periph_clock_enable(IRQOUT_RFTRANS_95HF_GPIO_CLOCK);
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);
	gpio_init(IRQOUT_RFTRANS_95HF_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, IRQOUT_RFTRANS_95HF_PIN);
	RFTRANS_95HF_IRQIN_HIGH();
	
	spi_parameter_struct spi_init_struct;
	rcu_periph_clock_enable (RFTRANS_95HF_SPI_CLK);
	
	/* Cau hinh tham so SPI1 */
	
	spi_init_struct.trans_mode            = SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode           = SPI_MASTER;
	spi_init_struct.frame_size            = SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
	spi_init_struct.nss                   = SPI_NSS_SOFT;
	spi_init_struct.prescale              = SPI_PSC_32;
	spi_init_struct.endian                = SPI_ENDIAN_MSB;
	spi_init (RFTRANS_95HF_SPI, & spi_init_struct);
	
	spi_enable (RFTRANS_95HF_SPI);
}

void drv95HF_SendSPICommand(uint8_t *pData){
	memset(DummyBuffer, 0, MAX_BUFFER_SIZE);
	/*  Select xx95HF over SPI  */
	RFTRANS_95HF_SELECT_LOW();
	osDelay(2);
	
	/* Send a sending request to xx95HF  */
	SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_SEND);
	
	if(*pData == ECHO){
		/* Send a sending request to xx95HF */ 
		SPI_SendReceiveByte(RFTRANS_95HF_SPI, ECHO);
	}else{
		SPI_SendReceiveBuffer(RFTRANS_95HF_SPI, pData, pData[RFTRANS_95HF_LENGTH_OFFSET]+RFTRANS_95HF_DATA_OFFSET, DummyBuffer);
	}

	/* Deselect xx95HF over SPI  */
	RFTRANS_95HF_SELECT_HIGH();
}

static int8_t drv95HF_SPIPollingCommand(void){
	uint8_t Polling_Status = 0;
	int8_t TimeOut = 5;
	
	do{
		RFTRANS_95HF_SELECT_LOW();
		
		osDelay(2);
		
		/* poll the 95HF transceiver until he's ready ! */
		Polling_Status  = SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_POLLING);
		
		Polling_Status &= RFTRANS_95HF_FLAG_DATA_READY_MASK;
		
		osDelay(10);
	}while((Polling_Status != RFTRANS_95HF_FLAG_DATA_READY) && (TimeOut-- != 0x00));
	
	RFTRANS_95HF_SELECT_HIGH();
	
	if(TimeOut <= 0x00)
		return RFTRANS_95HF_POLLING_TIMEOUT;
	
	return RFTRANS_95HF_SUCCESS_CODE;
}

void drv95HF_ReceiveSPIResponse(uint8_t *pData){
	memset(DummyBuffer, 0, MAX_BUFFER_SIZE);
	/* Select 95HF transceiver over SPI */
	RFTRANS_95HF_SELECT_LOW();
	osDelay(2);
	
	/* Request a response from 95HF transceiver */
	SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_RECEIVE);
	
	/* Recover the "Command" byte */
	pData[RFTRANS_95HF_COMMAND_OFFSET] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
	
	if(pData[RFTRANS_95HF_COMMAND_OFFSET] == ECHO){
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = 0x00;
		/* In case we were in listen mode error code cancelled by user (0x85 0x00) must be retrieved */
		pData[RFTRANS_95HF_LENGTH_OFFSET+1] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
		pData[RFTRANS_95HF_LENGTH_OFFSET+2] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
	}else if(pData[RFTRANS_95HF_COMMAND_OFFSET] == 0xFF){
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = 0x00;
		pData[RFTRANS_95HF_LENGTH_OFFSET+1] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
		pData[RFTRANS_95HF_LENGTH_OFFSET+2] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
	}else{
		/* Recover the "Length" byte */
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
		/* Checks the data length */
		if(pData[RFTRANS_95HF_LENGTH_OFFSET] != 0x00)
			/* Recover data 	*/
		SPI_SendReceiveBuffer(RFTRANS_95HF_SPI, DummyBuffer, pData[RFTRANS_95HF_LENGTH_OFFSET], &pData[RFTRANS_95HF_DATA_OFFSET]);
	}
	
	/* Deselect xx95HF over SPI */
	RFTRANS_95HF_SELECT_HIGH();
}

int8_t drv95HF_SendReceive(uint8_t *pCommand, uint8_t *pResponse){
	/* First step  - Sending command 	*/
	drv95HF_SendSPICommand(pCommand);
	/* Second step - Polling	*/
	if(drv95HF_SPIPollingCommand() != RFTRANS_95HF_SUCCESS_CODE){
		*pResponse =RFTRANS_95HF_ERRORCODE_TIMEOUT;
		return RFTRANS_95HF_POLLING_TIMEOUT;
	}
	/* Third step  - Receiving bytes */
	drv95HF_ReceiveSPIResponse(pResponse);
	
	return RFTRANS_95HF_SUCCESS_CODE;
}

void drv95HF_SendCmd(uint8_t *pCommand){
	/* First step  - Sending command 	*/
	drv95HF_SendSPICommand(pCommand);
}

void drv95HF_SendIRQINPulse(void){
	/* Send a pulse on IRQ_IN */
	RFTRANS_95HF_IRQIN_HIGH();
	osDelay(1);
	RFTRANS_95HF_IRQIN_LOW();
	osDelay(1);
	RFTRANS_95HF_IRQIN_HIGH();
	/* Need to wait 10ms after the pulse before to send the first command */
	osDelay(10);
}

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/

