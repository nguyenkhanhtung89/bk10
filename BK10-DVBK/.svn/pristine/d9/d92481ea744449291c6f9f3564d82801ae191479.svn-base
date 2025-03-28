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

#define nTASK 3

#define	RFID_VERSION		110

#define APP_ADDRESS 								((u32)0x0800A000)

#define FLG_UPDATE_FW_ADDR  				((u32)0x0803F800)

#define FLG_UPDATE_DONE_ADDR  			((u32)0x0803F000)

#define SLIENCE_MODE_ADDR  					((u32)0x0803E800)

#define BUZZER_LEVEL_ADDR  					((u32)0x0803E000)

#define RF_DEVICE_ID  							((u32)0x0803D800)

u32 FLASH_ReadData(u32 ADDR);

void FLASH_WriteData(u32 ADDR, u32 Data, uint8_t reset);

#endif  // __osObjects
