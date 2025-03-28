
#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "cmsis_os.h"

#define LED_POWER_GPIO_PIN						GPIO_PIN_0
#define LED_POWER_GPIO_PORT 					GPIOC
#define LED_POWER_GPIO_CLK					  RCU_GPIOC

#define LED_GPS_GPIO_PIN							GPIO_PIN_1
#define LED_GPS_GPIO_PORT 						GPIOC
#define LED_GPS_GPIO_CLK					  	RCU_GPIOC

#define LED_TCP_GPIO_PIN							GPIO_PIN_2
#define LED_TCP_GPIO_PORT 						GPIOC
#define LED_TCP_GPIO_CLK					  	RCU_GPIOC

#define LED_DRIVER_GPIO_PIN						GPIO_PIN_3
#define LED_DRIVER_GPIO_PORT 					GPIOC
#define LED_DRIVER_GPIO_CLK					  RCU_GPIOC

#define BUZZER_PIN										GPIO_PIN_11
#define BUZZER_GPIO_PORT 							GPIOA
#define BUZZER_GPIO_CLK					  		RCU_GPIOA

#define NC_MCU_PIN										GPIO_PIN_5
#define NC_MCU_GPIO_PORT 							GPIOC
#define NC_MCU_GPIO_CLK					  		RCU_GPIOC

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
	PutOutTag = 0,
	PutinTag,
	DriverLogin
}	Driver_StatusDef;

typedef enum{
	None,
	DeviceStarting,
	DeviceStarted,
	OverSpeed,
	DriveContinuousOverTime,
	bDriverLogin,
	bDriverLogout
}	Buzzer_StatusDef;

typedef struct{
	Driver_StatusDef DriverLedStatus;
	Buzzer_StatusDef BuzzerStatus;
}LedBuzzer_Def;

Buzzer_StatusDef get_buzer_state(void);

uint8_t is_buzz_disable(void);

void Interface_Init(void);

void Set_Led_Status(Led_TypeDef LedType, uint8_t state);

void reset_buzz_state(void);

void lcd_led_on(uint16_t timeout);

#endif /* __INTERFACE_H */

