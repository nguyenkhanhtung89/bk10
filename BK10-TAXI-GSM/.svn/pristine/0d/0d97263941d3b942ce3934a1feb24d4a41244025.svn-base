
#ifndef __DIGITAL_H
#define __DIGITAL_H

#include "cmsis_os.h"
#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>

#define UART3RxBuffer_SIZE 128
#define USART1RxBuffer_SIZE 512

void digital_init(void);

#define J1587_RDE_PIN              	GPIO_PIN_5
#define J1587_RDE_GPIO_PORT 				GPIOA
#define J1587_RDE_GPIO_CLK					RCU_GPIOA

typedef struct {
	uint16_t size;
	uint8_t data[512];
} J1587_Mail_Typedef;

typedef struct{
	uint16_t fuel1_val;
	uint8_t fuel1_cnn;
	uint8_t fuel1_tm;
	uint16_t fuel2_val;
	uint8_t fuel2_cnn;
	uint8_t fuel2_tm;
}fuel_232_def;

typedef struct{
	uint8_t size;
	uint8_t msg[32];
} J1587SegMsg;

typedef struct{
	uint8_t mid;
	uint8_t spid;
	uint8_t fmi;
	uint8_t iden;
	uint8_t type_code;
	uint32_t status_fault;							// 0 active, 1 inactive
	uint32_t	occ_count;  							// 18bit
	uint8_t Count;	
} FaultCodeJ1587;

extern fuel_232_def fuel_232_value;


void USART1PushBuffer(uint8_t* data, uint16_t size);
	
void UART3PushBuffer(uint8_t* data, uint16_t size);

#endif /* __DIGITAL */
