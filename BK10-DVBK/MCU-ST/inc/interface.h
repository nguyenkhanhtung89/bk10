
#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "cmsis_os.h"

#define LED_GPS_GPIO_PIN							GPIO_Pin_14
#define LED_GPS_GPIO_PORT 						GPIOB
#define LED_GPS_GPIO_CLK					  	RCC_AHBPeriph_GPIOB

#define LED_TCP_GPIO_PIN							GPIO_Pin_6
#define LED_TCP_GPIO_PORT 						GPIOC
#define LED_TCP_GPIO_CLK					  	RCC_AHBPeriph_GPIOC

#define LED_DRIVER_GPIO_PIN						GPIO_Pin_13
#define LED_DRIVER_GPIO_PORT 					GPIOB
#define LED_DRIVER_GPIO_CLK					  RCC_AHBPeriph_GPIOB

#define BUZZER_GPIO_PIN								GPIO_Pin_8
#define BUZZER_GPIO_PORT 							GPIOC
#define BUZZER_GPIO_CLK					  		RCC_AHBPeriph_GPIOC

#define ACC_GPIO_PIN									GPIO_Pin_6
#define ACC_GPIO_PORT 								GPIOB
#define ACC_GPIO_CLK					  			RCC_AHBPeriph_GPIOB
#define ACC_STATUS 										(!GPIO_ReadInputDataBit(ACC_GPIO_PORT, ACC_GPIO_PIN))

#define DOOR_GPIO_PIN									GPIO_Pin_14
#define DOOR_GPIO_PORT 								GPIOC
#define DOOR_GPIO_CLK					 				RCC_AHBPeriph_GPIOC
#define DOOR_STATUS 									(!GPIO_ReadInputDataBit(DOOR_GPIO_PORT, DOOR_GPIO_PIN))

#define AIR_GPIO_PIN									GPIO_Pin_13
#define AIR_GPIO_PORT 					  		GPIOC
#define AIR_GPIO_CLK					    		RCC_AHBPeriph_GPIOC
#define AIR_STATUS 										(!GPIO_ReadInputDataBit(AIR_GPIO_PORT, AIR_GPIO_PIN))

/* Buzzer Prescale */
#define BUZZER_LEVEL1 1300
#define BUZZER_LEVEL2 1200
#define BUZZER_LEVEL3 1100
#define BUZZER_LEVEL4 1050
#define BUZZER_LEVEL5 1000

typedef enum {
  LED_TCP,
  LED_GPS,
	LED_DRIVER,
	BUZZER
} Led_TypeDef;

typedef enum{
	TCPNotConnect = 0,
	TCPConnected = 1
}	GSM_StatusDef;

typedef enum{
	GPSStartting,
	GPSRecieved,	
	GPSNotRecieved,
	GPSNotWorking
}	GPS_StatusDef;

typedef enum{
	FlashMemoryStartting,
	FlashMemoryWorking,	
	FlashMemoryNotWorking
}	FlashMemory_StatusDef;

typedef enum{
	None,
	DeviceStarting,
	DeviceStarted,
	OverSpeed,
	DriveContinuousOverTime,
	bDriverLogin,
	bDriverLogout,
	overDrvInDay,
	overDrvInWeek,
	CamDisconnected,
	MoveWithoutLogin
}	Buzzer_StatusDef;

typedef struct{
	Buzzer_StatusDef BuzzerStatus;
}LedBuzzer_Def;

Buzzer_StatusDef get_buzer_state(void);

uint8_t is_buzz_disable(void);

void Interface_Init(void);

void Set_Led_Status(Led_TypeDef LedType, uint8_t state);

void reset_buzz_state(void);

void lcd_led_on(uint16_t timeout);

#endif /* __INTERFACE_H */

