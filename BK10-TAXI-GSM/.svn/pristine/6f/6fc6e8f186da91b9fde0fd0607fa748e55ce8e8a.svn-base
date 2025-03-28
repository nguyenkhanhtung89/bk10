#ifndef __CANBUS_H
#define __CANBUS_H

#include <stdint.h>

uint8_t CAN_service_init(void);

extern u8 VIN_NUMBER[32];
extern u8 flg_rcv_vin_number;
extern u8 flg_check_update_vin;

typedef union{
	unsigned int num;
	unsigned char byte[sizeof(unsigned int)];
}dnumber_uni;

typedef struct{
	u8 front_passenger_seat;
	u8 right_rear_passenger_seat;
	u8 mid_rear_passenger_seat;
	u8 left_rear_passenger_seat;
	u8 seat_value;
	uint16_t rpm;
	
	uint8_t speedOBD;						/* Speed */
	
	int temp;					/* Temperature */
	
	uint16_t p_fuel;				/* Percent fuel */
	
	uint32_t s_km;					/* Km */
	
	uint32_t s_100m;
	
	uint32_t s_hour;				/* Hour */
	
	uint32_t s_fuel;				/* Lit */
	
	double econ_fuel;			/* Km/Lit */
	
	double avg_econ_fuel;	/* Km/Lit */
	
	uint8_t p_load;						/* Percent power */
	
	uint32_t bat;							/* mV */
	
	uint8_t connected;
	
	uint32_t odometer;
	
	u8 timeout;
	
	u8 isOBD_VF8;
	
} ODBDef;

extern ODBDef canbus;

void vOBDIITransmit(const void* args);

#endif /* CANBUS_H */

