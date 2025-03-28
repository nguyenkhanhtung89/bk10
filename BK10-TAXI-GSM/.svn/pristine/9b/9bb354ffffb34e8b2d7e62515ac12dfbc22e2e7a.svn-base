
#ifndef __POWER_H
#define __POWER_H

#include "cmsis_os.h"

#include "osObjects.h"

void Power_Init(void);

#define LOW_BATTERY_LEVEL       			3600

#define LOW_EXT_POWER_LEVEL     			7000

#define V12_R1 	  100000
#define V12_R2 	  13000

#define VBAT_R1 	2200
#define VBAT_R2 	3300

#define VADC_R1 	4700
#define VADC_R2 	3300

#define V5V6_R1 	4700
#define V5V6_R2 	3300

typedef struct{
	__IO uint16_t main_power_val;
	
	__IO uint16_t battery_val;
	
	__IO uint16_t adc_pulse_val; /* pulse or adc */
	
	__IO uint16_t v_5v6_val;
	
	__IO uint16_t PulseKm;
	
	__IO uint8_t PulseSpeed;
	
	__IO uint32_t PulseDistance;
	
	uint8_t fuel1_value;
	
	uint8_t fuel2_value;
}ADC_def;

typedef struct{
__IO uint16_t FuelCyclesTime;		
	
__IO uint16_t RisingTime;	

__IO uint16_t FallingTime;	
	
__IO int32_t FuelPulseValues;
	
__IO int32_t FuelCyclesValues;

/* Clear fuel timeout */
__IO uint32_t FuelPulseTimeOut;
	
__IO uint32_t TM3_Counter;
	
__IO uint32_t Freq_pulse;
}PulseDef;			   
extern ADC_def ADCData;

typedef struct{
	uint8_t statusButton;
	uint8_t statusHiring;
	uint8_t statusIR1;
	uint8_t statusIR2;
	uint16_t countFallingIR1;
	uint16_t countFallingIR2;
	uint8_t status_seat;
}SensorIRDef;

extern SensorIRDef sensorIR;

typedef enum{
	POWER_BALANCE,
	POWER_SAVE
}power_type;

typedef struct{
	power_type power_mode;
	uint8_t ext_power_l;
	uint8_t ext_power_h;
	uint8_t low_bat_cnt;
}power_def;

extern power_def PowerData;

extern uint16_t counter_ir_1;
extern uint16_t counter_ir_2;

#define PWR_EC21_GPIO_PIN							GPIO_PIN_12
#define PWR_EC21_GPIO_PORT 						GPIOC
#define PWR_EC21_GPIO_CLK					 	  RCU_GPIOC

#define PWR_BATTERY_GPIO_PIN					GPIO_PIN_11
#define PWR_BATTERY_GPIO_PORT 				GPIOB
#define PWR_BATTERY_GPIO_CLK					RCU_GPIOB

#define PWR_BOST_GPIO_PIN							GPIO_PIN_9//ok
#define PWR_BOST_GPIO_PORT 						GPIOC
#define PWR_BOST_GPIO_CLK							RCU_GPIOC

#define PWR_5VOUT_GPIO_PIN						GPIO_PIN_1//ok
#define PWR_5VOUT_GPIO_PORT 					GPIOA
#define PWR_5VOUT_GPIO_CLK						RCU_GPIOA

#define TRAN_SENSOR_GPIO_PIN					GPIO_PIN_0//ok
#define TRAN_SENSOR_GPIO_PORT 				GPIOA
#define TRAN_SENSOR_GPIO_CLK					RCU_GPIOA

#define PWR_MAIN_GPIO_PIN							GPIO_PIN_15//ok
#define PWR_MAIN_GPIO_GPIO_PORT 			GPIOC
#define PWR_MAIN_GPIO_GPIO_CLK				RCU_GPIOC

#define PWR_RFID_GPIO_PIN							GPIO_PIN_9//ok
#define PWR_RFID_GPIO_GPIO_PORT 			GPIOB
#define PWR_RFID_GPIO_GPIO_CLK				RCU_GPIOB

extern uint8_t time_check_sensor;

void EC21_PW_Reset(void);

void InitVS1838B(void);

#endif /* __POWER_H */
