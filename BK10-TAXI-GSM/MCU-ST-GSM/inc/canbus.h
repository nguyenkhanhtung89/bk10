#ifndef __CANBUS_H
#define __CANBUS_H

#include <stdint.h>

#include "j1939.h"

uint8_t CAN_service_init(void);

extern ODBDef canbus;

typedef union{
	unsigned int num;
	unsigned char byte[sizeof(unsigned int)];
}dnumber_uni;

#endif /* CANBUS_H */

