/*----------------------------------------------------------------------------
 * osObjects.h: CMSIS-RTOS global object definitions for an application
 *----------------------------------------------------------------------------
 *
 * This header file defines global RTOS objects used throughout a project
 *
 * #define osObjectsPublic indicates that objects are defined; without that
 * definition the objects are defined as external symbols.
 *
 *--------------------------------------------------------------------------*/

#ifndef __osObjects
#define __osObjects

#if (!defined (osObjectsPublic))
#define osObjectsExternal           // define RTOS objects with extern attribute
#endif

#include "cmsis_os.h"               // CMSIS RTOS header file
#include "stm32f30x.h"
#include "thread.h"
#include "flash.h"

#define nTASK 3

#define	MCU_VERSION		25

#define APP_ADDRESS 								((u32)0x0800A000)

#define FLG_UPDATE_FW_ADDR  				((u32)0x0803F800)

#define FLG_UPDATE_DONE_ADDR  			((u32)0x0803F000)

#define BAURATE4_ADDR  							((u32)0x0803E800)

#define USE_RELEASE_BUTTON  				((u32)0x0803E000)

#define OBD_TYPE_ADDR  							((u32)0x0803D800)

#define MODE_TAXI_ADDR  						((u32)0x0803D000)

#define SLIENCE_MODE_ADDR  					((u32)0x0803C800)

#define BUZZER_LEVEL_ADDR  					((u32)0x0803C000)

#define TYPE_FULSE_ADC_ADDR  				((u32)0x0803B800)

#define DEV_CONFIG_ADDR  						((u32)0x0803B000)

#endif  // __osObjects
