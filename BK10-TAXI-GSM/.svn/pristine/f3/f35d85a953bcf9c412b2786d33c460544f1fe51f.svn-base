
#ifndef __SERIAL_H
#define __SERIAL_H

#include "cmsis_os.h"
#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>

#define USART_COM		        			    USART0
#define USART_BAUDRATE								230400
#define USART_COM_GPIO_TX             GPIOA
#define USART_COM_GPIO_RX             GPIOA
#define USART_COM_CLK                 RCU_USART0
#define USART_COM_GPIO_CLK_TX         RCU_GPIOA
#define USART_COM_GPIO_CLK_RX         RCU_GPIOA
#define USART_COM_TxPin               GPIO_PIN_9
//#define USART_COM_TxPinSource         GPIO_PinSource9
//#define USART_COM_AF                  GPIO_AF_7

#define USART_COM_RxPin               GPIO_PIN_10
//#define USART_COM_RxPinSource         GPIO_PinSource10
#define USART_COM_IRQn                USART0_IRQn
#define USART_COM_IRQHandler          USART0_IRQHandler
#define SerialRxBuffer_SIZE						(3072 + 128)
#define SerialPkg_SIZE								SerialRxBuffer_SIZE
typedef struct{
	uint16_t size;
	uint8_t data[SerialPkg_SIZE];
}Serial_Mail_Typedef;

typedef enum{
	END_CR,
	END_LF,
	END_CRLF,
	END_SUB,
	END_NONE
} ENDLINE;

typedef enum{
	GPS_TASK,
	GSM_TASK,
	SPI_TASK
}Task_type;

void Serial_Init(void);

void UART_SendByte(uint32_t USARTx, uint8_t data);

void SerialPushByte(uint8_t data);

void SerialPushStr(char* data, ENDLINE Type);

void SerialPushBufferWithCS(uint8_t* data, uint16_t size);

void SerialPushBuffer(uint8_t* data, uint16_t size);

void SerialPushInt(uint64_t val, ENDLINE type);

void PrintError(char* err);

void Send_TaskMsg(Task_type TaskN);

void SerialUARTInit(void);

int get_server_SOS_Status(void);

void reset_server_SOS_Status(void);

#endif /* __SERIAL_H */
