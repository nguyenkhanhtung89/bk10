
#ifndef __DIGITAL_H
#define __DIGITAL_H

#include "cmsis_os.h"
#include "stm32f30x.h"
#include <stdio.h>
#include <string.h>

#define UART4RxBuffer_SIZE 128
#define USART2RxBuffer_SIZE 512

void digital_init(void);

#define J1587_RDE_PIN              	GPIO_Pin_4
#define J1587_RDE_GPIO_PORT 				GPIOF
#define J1587_RDE_GPIO_CLK					RCC_AHBPeriph_GPIOF

typedef struct {
	uint16_t size;
	u8 data[512];
} J1587_Mail_Typedef;

typedef struct{
	u16 fuel1_val;
	u8 fuel1_cnn;
	u8 fuel1_tm;
	u16 fuel2_val;
	u8 fuel2_cnn;
	u8 fuel2_tm;
}fuel_232_def;

typedef struct{
	u8 size;
	u8 msg[32];
} J1587SegMsg;

typedef struct{
	u8 mid;
	u8 spid;
	u8 fmi;
	u8 iden;
	u8 type_code;
	u32 status_fault;							// 0 active, 1 inactive
	u32	occ_count;  							// 18bit
	u8 Count;	
} FaultCodeJ1587;

extern fuel_232_def fuel_232_value;

void USART2PushBuffer(uint8_t* data, uint16_t size);
	
void UART4PushBuffer(uint8_t* data, uint16_t size);

#endif /* __DIGITAL */
