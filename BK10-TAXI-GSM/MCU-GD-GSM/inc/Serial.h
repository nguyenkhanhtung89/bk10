
#ifndef __SERIAL_H
#define __SERIAL_H

#include "cmsis_os.h"
#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>

#define USART_EC21		        			   USART0
#define USART_EC21_CLK                 RCU_USART0

#define USART_EC21_TxPin               GPIO_PIN_9
#define USART_EC21_GPIO_TX             GPIOA
#define USART_EC21_GPIO_CLK_TX         RCU_GPIOA

#define USART_EC21_RxPin               GPIO_PIN_10
#define USART_EC21_GPIO_RX             GPIOA
#define USART_EC21_GPIO_CLK_RX         RCU_GPIOA

#define USART_EC21_IRQn                USART0_IRQn
#define USART_EC21_IRQHandler          USART0_IRQHandler
#define SerialRxBuffer_SIZE						 512

typedef enum{
	END_CR,
	END_LF,
	END_CRLF,
	END_SUB,
	END_NONE
} ENDLINE;

typedef struct {
  uint16_t size;
  uint8_t data[SerialRxBuffer_SIZE];
} Serial_Mail_Typedef;

typedef struct {
	uint8_t flg_ec21_crash;
	uint8_t flg_ec21_rdy;
	uint16_t ec21_cnn_timeout;
	uint8_t ec21_pw_rst_cnt;
} ec21_pw_def;
extern ec21_pw_def ec21_status;

#define EC21_DATA_MESSAGE   1
#define USART1_FORWARD_MSG  2
#define UART3_FORWARD_MSG   3
#define STORE_FLASH_DATA   	4
#define GET_FLASH_DATA   		5

typedef enum{
	OBD_NONE = 0,
	OBDII_TYPE = 1,
	J1939_TYPE = 2,
	J1587_TYPE = 3,
	VFE34_TYPE = 4,
	VF8_TYPE = 5
}enum_OBD_type;

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
	uint16_t PulseKm;
	uint32_t Baud2;
	uint32_t Baud4;
	u8 seat_level;
	enum_OBD_type TypeOBD;
	uint8_t buzz_level;
	uint8_t slience_mode;
	uint8_t gps_status;
	uint8_t login_status;
	uint8_t ov_speed_status;
	uint8_t drv_continue_status;
	E_QL_SIM_STATES_T sim_status;
	uint8_t tcp_status;
	uint8_t internet_status;
	uint8_t TaxiMode;
	uint8_t disable_release_button;
}ec21_message_def;

extern ec21_message_def ec21_data;

typedef struct{
	uint32_t baud_2_value;
	uint32_t baud_4_value;
	uint16_t PulseKm;
	u8 seat_level;
	enum_OBD_type TypeOBD;
	uint8_t buzz_level;
	uint8_t slience_mode;
	uint8_t TaxiMode;
	uint8_t LedMode;
	uint8_t LedStatus;
	uint8_t tmp_use_release_button;
	uint8_t use_release_button;
}mcu_config_def;
extern mcu_config_def mcu_config;

void Serial_Init(void);

void PrintError(char* err);

void UART_SendByte(uint32_t USARTx, uint8_t data);

void SerialPushByte(uint8_t data);

void SerialPushStr(char* data, ENDLINE Type);

void SerialPushBuffer(uint8_t* data, uint16_t size);

void ec21_power_rst(uint8_t time);

void device_battery_off(uint8_t time);

void load_mcu_config(void);						   
#endif /* __SERIAL_H */
