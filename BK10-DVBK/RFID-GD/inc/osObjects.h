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
#include "gd32f30x.h"
#include "thread.h"
// global 'thread' functions ---------------------------------------------------

#define	RFID_VERSION		110

#define APP_ADDRESS 								((uint32_t)0x0800A000)

#define FLG_UPDATE_FW_ADDR  				((uint32_t)0x0803F800)

#define FLG_UPDATE_DONE_ADDR  			((uint32_t)0x0803F000)

#define RF_DEVICE_ID  							((uint32_t)0x0803D800)

#define SLIENCE_MODE_ADDR  					((uint32_t)0x0803E000)

#define BUZZER_LEVEL_ADDR  					((uint32_t)0x0803E800)

uint32_t FLASH_ReadData(uint32_t ADDR);

void FLASH_WriteData(uint32_t ADDR, uint32_t Data, uint8_t reset);

#endif  // __osObjects
