#ifndef __VIBRATE_H
#define __VIBRATE_H

#include <ql_oe.h>

typedef enum{
	LSM_STOPPING = 0,
	LSM_ENGINE_ON = 1,
	LSM_VEHICLE_RUN = 2
}enum_LSM_State;

typedef enum{
	LIS3DH = 0,
	LIS3DSH = 1
}enum_Vibrate_State;

extern uint8_t SensorVibrate1;

extern enum_LSM_State LSM_Status;

extern uint8_t countCheckInitSensor;

#define LSM_ACC ((LSM_Status != 0) ? 1 : 0)

void *thread_vibrate(void *arg);

#endif /* __VIBRATE */
