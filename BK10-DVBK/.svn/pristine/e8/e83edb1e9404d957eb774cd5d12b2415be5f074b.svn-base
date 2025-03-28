/* Define to prevent recursive inclusion ------------------------------------------------ */
#ifndef __DRIVER_RFTRANS_95HF_H
#define __DRIVER_RFTRANS_95HF_H

/* Includes ----------------------------------------------------------------------------- */
#include "drv_spi.h"
#include "cmsis_os.h" 
#include "Serial.h" 

/* RFtransceiver HEADER command definition ---------------------------------------------- */
#define RFTRANS_95HF_COMMAND_SEND															0x00
#define RFTRANS_95HF_COMMAND_RESET														0x01
#define RFTRANS_95HF_COMMAND_RECEIVE													0x02
#define RFTRANS_95HF_COMMAND_POLLING													0x03
#define RFTRANS_95HF_COMMAND_IDLE															0x07

/* RFtransceiver mask and data to check the data (SPI polling)--------------------------- */
#define RFTRANS_95HF_FLAG_DATA_READY													0x08
#define RFTRANS_95HF_FLAG_DATA_READY_MASK											0x08

/* RF transceiver status	--------------------------------------------------------------- */
#define RFTRANS_95HF_SUCCESS_CODE															RESULTOK
#define RFTRANS_95HF_NOREPLY_CODE															0x01
#define	RFTRANS_95HF_ERRORCODE_DEFAULT												0xFE
#define	RFTRANS_95HF_ERRORCODE_TIMEOUT												0xFD
#define RFTRANS_95HF_ERRORCODE_POR														0x44

/* RF transceiver polling status	------------------------------------------------------- */
#define RFTRANS_95HF_POLLING_RFTRANS_95HF											0x00
#define RFTRANS_95HF_POLLING_TIMEOUT													0x01

/* RF transceiver number of byte of the buffers------------------------------------------ */
#define RFTRANS_95HF_RESPONSEBUFFER_SIZE											0xFF
#define RFTRANS_95HF_MAX_BUFFER_SIZE													0xFF
#define RFTRANS_95HF_MAX_ECHO																	768

/* RF transceiver Offset of the command and the response -------------------------------- */
#define RFTRANS_95HF_COMMAND_OFFSET														0x00
#define RFTRANS_95HF_LENGTH_OFFSET														0x01
#define RFTRANS_95HF_DATA_OFFSET															0x02

/* ECHO response ------------------------------------------------------------------------ */
#define ECHORESPONSE																					0x55

/* Sleep parameters --------------------------------------------------------------------- */
#define IDLE_SLEEP_MODE																				0x00
#define IDLE_HIBERNATE_MODE																		0x01

#define IDLE_CMD_LENTH																				0x0E

#define WU_TIMEOUT																						0x01
#define WU_TAG																								0x02
#define WU_FIELD																							0x04
#define WU_IRQ																								0x08
#define WU_SPI																								0x10

#define HIBERNATE_ENTER_CTRL																	0x0400
#define SLEEP_ENTER_CTRL																			0x0100
#define SLEEP_FIELD_ENTER_CTRL																0x0142

#define HIBERNATE_WU_CTRL																			0x0400
#define SLEEP_WU_CTRL																					0x3800

#define LEAVE_CTRL																						0x1800

/* Calibration parameters---------------------------------------------------------------- */
#define WU_SOURCE_OFFSET																			0x02
#define WU_PERIOD_OFFSET																			0x09
#define DACDATAL_OFFSET																				0x0C
#define DACDATAH_OFFSET																				0x0D
#define NBTRIALS_OFFSET																				0x0F

/* baud rate command -------------------------------------------------------------------- */
#define	BAUDRATE_LENGTH						  													0x01
#define	BAUDRATE_DATARATE_DEFAULT			  											57600

#define MAX_BUFFER_SIZE  																			256 

#define ECHO																									0x55
#define LISTEN																								0x05
#define DUMMY_BYTE																						0xFF

#define READERREPLY_MAX_BUFFER_SIZE			0x40

/* Functions ---------------------------------------------------------------- */

void 		drv95HF_ResetSPI(void);
void 		drv95HF_InitializeSPI(void);
void 		drv95HF_ReceiveSPIResponse(uint8_t *pData);
void 		drv95HF_SendSPICommand(uint8_t *pData);

int8_t  drv95HF_SendReceive(uint8_t *pCommand, uint8_t *pResponse);
void  	drv95HF_SendCmd ( uint8_t *pCommand );
void drv95HF_SendIRQINPulse(void);

#endif /* __RFTRANS_95HF_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/



