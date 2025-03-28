#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f30x.h"
#include "cmsis_os.h"

void init_lcd_services(void);


typedef enum{
	DISP_HOME = 0,
	DISP_DEVICE_INFO = 1,
	DISP_DRIVER_INFO = 2,
	DISP_DRIVER_RESULT = 3,
	DISP_DRIVER_REPORT = 4,
	DISP_DEVICE_SETTING = 5,
	DISP_PRIVATE_SETTING = 6,
	DISP_FUEL_VALUE = 7,
}enum_DISP_INFO;

extern enum_DISP_INFO pDisplay;

void FnKey_RaiseEvent(void);

#endif /* __DISPLAY_H */
