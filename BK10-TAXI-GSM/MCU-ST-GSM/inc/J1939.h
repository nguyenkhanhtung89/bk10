#ifndef __J1939_H
#define __J1939_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmsis_os.h"
#include "stm32f30x.h"

#define CANx                       CAN1
#define CAN_CLK                    RCC_APB1Periph_CAN1
#define CAN_RX_PIN                 GPIO_Pin_11
#define CAN_TX_PIN                 GPIO_Pin_12
#define CAN_GPIO_PORT              GPIOA
#define CAN_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define CAN_AF_PORT                GPIO_AF_9
#define CAN_RX_SOURCE              GPIO_PinSource11
#define CAN_TX_SOURCE              GPIO_PinSource12
#define CAN1_IRQHandler			 			 USB_LP_CAN1_RX0_IRQHandler

typedef struct{
	u8 front_passenger_seat;
	u8 right_rear_passenger_seat;
	u8 mid_rear_passenger_seat;
	u8 left_rear_passenger_seat;
	
	u16 rpm;
	u8 sp;						/* Speed */
	int temp;					/* Temperature */
	u16 p_fuel;				/* Percent fuel */
	u32 s_km;					/* Km */
	u32 s_hour;				/* Hour */
	u32 s_fuel;				/* Lit */
	double econ_fuel;			/* Km/Lit */
	double avg_econ_fuel;	/* Km/Lit */
	u8 p_load;						/* Percent power */
	u32 bat;							/* mV */
	u8 connected;
	char VIN_ID[64];
	u32 odometer;
	u8 timeout;
	
	u8 isOBD_VF8;
} ODBDef;

typedef struct{
	u8 iLampStatus;
	u8 sLampStatus;
	u8 wLampStatus;
	u8 pLampStatus;
	u32 PGN;
	u32	SPN;  							// 18bit
	u8 FMI;
	u8 Count;	
} FaultCodeJ1939;

typedef struct{
	u8 first;
	u8 second;
}FaulCodeODBII;

void J1939Thread_Init(void);

void vJ1939Task(const void* args);

ODBDef getDataOBD(void);

uint8_t findFaultCode(FaultCodeJ1939 fc);

uint8_t FindFaultCodeODB(u8 fst, u8 sec);

uint8_t StoreFaultCode(FaultCodeJ1939 fc);

uint8_t StoreFaultCodeODBII(u8 fst, u8 sec);

void SetECUClearData(void);

u8 OBDII_REQUEST_DTCS(u8 idx);

#endif /* __J1939_H */

