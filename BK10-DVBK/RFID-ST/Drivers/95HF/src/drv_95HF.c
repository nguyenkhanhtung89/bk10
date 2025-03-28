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
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	SPI_I2S_DeInit(RFTRANS_95HF_SPI);
	
	/* enables SPI and GPIO clocks */
  RCC_AHBPeriphClockCmd(POWER_95HF_GPIO_CLK |
													RFTRANS_95HF_SPI_SCK_GPIO_CLK  |
													RFTRANS_95HF_SPI_MISO_GPIO_CLK |
													RFTRANS_95HF_SPI_MOSI_GPIO_CLK |
													IRQOUT_RFTRANS_95HF_GPIO_CLOCK |
													RFTRANS_95HF_SPI_SELECT_GPIO_CLK, ENABLE);
  /* Enable SPI clock  */
  RCC_APB1PeriphClockCmd(RFTRANS_95HF_SPI_CLK, ENABLE);
	
	/* CFG AF mode */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
	/* Configure RFTRANS SCK*/
	GPIO_InitStructure.GPIO_Pin 	= RFTRANS_95HF_SPI_SCK_PIN;
	GPIO_Init(RFTRANS_95HF_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(RFTRANS_95HF_SPI_SCK_GPIO_PORT , RFTRANS_95HF_SPI_SCK_PinSource, RFTRANS_95HF_SPI_AF);
	
	/* Configure RFTRANS MISO */
	GPIO_InitStructure.GPIO_Pin 	= RFTRANS_95HF_SPI_MISO_PIN;
	GPIO_Init(RFTRANS_95HF_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(RFTRANS_95HF_SPI_MISO_GPIO_PORT, RFTRANS_95HF_SPI_MISO_PinSource, RFTRANS_95HF_SPI_AF);
	
	/* Configure RFTRANS MOSI */
	GPIO_InitStructure.GPIO_Pin 	= RFTRANS_95HF_SPI_MOSI_PIN;
	GPIO_Init(RFTRANS_95HF_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(RFTRANS_95HF_SPI_MOSI_GPIO_PORT, RFTRANS_95HF_SPI_MOSI_PinSource, RFTRANS_95HF_SPI_AF);
	
	/* CFG output mode */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	
	/* Configure Chip select */
	GPIO_InitStructure.GPIO_Pin   = RFTRANS_95HF_SPI_SELECT_PIN;
	GPIO_Init(RFTRANS_95HF_SPI_SELECT_GPIO_PORT, &GPIO_InitStructure);
	/* SPI_NSS  = High Level  */
	GPIO_SetBits(RFTRANS_95HF_SPI_SELECT_GPIO_PORT, RFTRANS_95HF_SPI_SELECT_PIN);
	
	/* Configure RFTRANS PWR  */
	GPIO_InitStructure.GPIO_Pin   = POWER_95HF_PIN;
  GPIO_Init(POWER_95HF_GPIO_PORT, &GPIO_InitStructure);
	/* RFTRANS POWER ON */
	GPIO_SetBits(POWER_95HF_GPIO_PORT, POWER_95HF_PIN);
	
	/* Configure IRQ pin as output */
	GPIO_InitStructure.GPIO_Pin  = IRQOUT_RFTRANS_95HF_PIN;
	GPIO_Init(IRQOUT_RFTRANS_95HF_GPIO_PORT, &GPIO_InitStructure);
	RFTRANS_95HF_IRQIN_HIGH();
	
	/* Initialize the SPI */
	SPI_StructInit(&SPI_InitStructure);
	
	/* SPI Config master with NSS manages by software using the SSI bit*/
	SPI_InitStructure.SPI_Mode 				= SPI_Mode_Master;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_NSS  				= SPI_NSS_Soft;
	SPI_InitStructure.SPI_CPOL 				= SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA 				= SPI_CPHA_2Edge;
	
	/* Init SPI BRIDGE */
	SPI_Init(RFTRANS_95HF_SPI, &SPI_InitStructure);
	/* Configure FIFO threshold */
	SPI_RxFIFOThresholdConfig(RFTRANS_95HF_SPI, SPI_RxFIFOThreshold_QF);
 	/* Enable SPI */
	SPI_Cmd(RFTRANS_95HF_SPI, ENABLE);
}

void drv95HF_SendSPICommand(uc8 *pData){
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

int8_t drv95HF_SendReceive(uc8 *pCommand, uint8_t *pResponse){
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

void drv95HF_SendCmd(uc8 *pCommand){
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

