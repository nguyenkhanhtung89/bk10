#ifndef __J1939_H
#define __J1939_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmsis_os.h"
#include "gd32f30x.h"

typedef struct{
	
	uint16_t rpm;
	
	uint8_t sp;						/* Speed */
	
	int temp;					/* Temperature */
	
	uint16_t p_fuel;				/* Percent fuel */
	
	uint32_t s_km;					/* Km */
	
	uint32_t s_hour;				/* Hour */
	
	uint32_t s_fuel;				/* Lit */
	
	double econ_fuel;			/* Km/Lit */
	
	double avg_econ_fuel;	/* Km/Lit */
	
	uint8_t p_load;						/* Percent power */
	
	uint32_t bat;							/* mV */
	
	uint8_t connected;
	
	char VIN_ID[64];
	
	uint32_t odometer;
	
} ODBDef;

extern ODBDef canbus;

typedef struct{
	
	uint8_t iLampStatus;
		
	uint8_t sLampStatus;
		
	uint8_t wLampStatus;
		
	uint8_t pLampStatus;
	
	uint32_t PGN;
	
	uint32_t	SPN;  							// 18bit
	
	uint8_t FMI;
	
	uint8_t Count;	
	
} FaultCodeJ1939;

typedef struct{
	
	uint8_t first;
	
	uint8_t second;
	
}FaulCodeODBII;

void J1939Thread_Init(void);

void vJ1939Task(const void* args);

ODBDef getDataOBD(void);

uint8_t findFaultCode(FaultCodeJ1939 fc);

uint8_t FindFaultCodeODB(uint8_t fst, uint8_t sec);

uint8_t StoreFaultCode(FaultCodeJ1939 fc);

uint8_t StoreFaultCodeODBII(uint8_t fst, uint8_t sec);

void SetECUClearData(void);

uint8_t OBDII_REQUEST_DTCS(uint8_t idx);

#endif /* __J1939_H */

