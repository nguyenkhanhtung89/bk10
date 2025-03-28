
#ifndef __SERIAL_H
#define __SERIAL_H

#include "cmsis_os.h"
#include "stm32f30x.h"
#include <stdio.h>
#include <string.h>

#define USART_EC21		        			   USART1

#define USART_EC21_CLK                 RCC_APB2Periph_USART1

#define USART_EC21_AF                  GPIO_AF_7

#define USART_EC21_TxPin               GPIO_Pin_9
#define USART_EC21_GPIO_TX             GPIOA
#define USART_EC21_GPIO_CLK_TX         RCC_AHBPeriph_GPIOA
#define USART_EC21_TxPinSource         GPIO_PinSource9

#define USART_EC21_RxPin               GPIO_Pin_10
#define USART_EC21_GPIO_RX             GPIOA
#define USART_EC21_GPIO_CLK_RX         RCC_AHBPeriph_GPIOA
#define USART_EC21_RxPinSource         GPIO_PinSource10

#define USART_EC21_IRQn                USART1_IRQn
#define USART_EC21_IRQHandler          USART1_IRQHandler
#define SerialBuffer_SIZE						 256

#define	MAX_SIZE_NUMBER_PLATE	  	16
#define	MAX_SIZE_DOMAIN	          32

typedef enum{
	STATUS_UNKNOWN = 0,
	STATUS_LOGOUT = 1,
	STATUS_LOGIN = 2
}enum_login_status;

typedef enum{
	END_CR,
	END_LF,
	END_CRLF,
	END_SUB,
	END_NONE
} ENDLINE;

typedef enum{
	E_QL_SIM_STAT_NOT_INSERTED,
	E_QL_SIM_STAT_READY,
	E_QL_SIM_STAT_PIN_REQ,
	E_QL_SIM_STAT_PUK_REQ,
	E_QL_SIM_STAT_BUSY,
	E_QL_SIM_STAT_BLOCKED,
	E_QL_SIM_STAT_UNKNOWN
}E_QL_SIM_STATES_T;

typedef struct{
	__IO enum_login_status LoginStatus;
	u8 sync;
	u64 UID;
	char Name[64];
	char License[64];
	char IssDate[16];
	char ExpDate[16];
}DriverInfoDef;

typedef struct{
	uint8_t  signal_strength;
	E_QL_SIM_STATES_T sim_status;
	uint8_t internet_status;
	uint8_t tcp_status;
}nw_data_def;

typedef struct {
  uint16_t size;
  uint8_t data[SerialBuffer_SIZE];
} Serial_Mail_Typedef;

typedef enum {
	GPS_DATA_MESSAGE = 1,
	DEVICE_SETTING_MESSAGE = 2,
	DRIVER_MESSAGE = 3
} enum_ec21_message;

typedef struct{
	uint8_t gps_status;
	uint8_t login_status;
	uint8_t ov_speed_status;
	uint8_t drv_continue_status;
	uint8_t sim_status;
	uint8_t tcp_status;
	uint8_t internet_status;
	uint8_t acc_status;
	DriverInfoDef DriverInfo;
	uint8_t buzz_level;
	uint8_t slience_mode;
}ec21_message_def;
extern ec21_message_def ec21_data;

typedef struct{
	uint8_t buzz_level;
	uint8_t slience_mode;
	uint32_t device_id; 
}mcu_config_def;

extern mcu_config_def mcu_config;

void Serial_Init(void);

void UART_SendByte(USART_TypeDef* USARTx, uint8_t data);

void SerialTxBufferPush(uint8_t * data, uint16_t size);

#endif /* __SERIAL_H */
