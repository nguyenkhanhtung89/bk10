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

typedef  void (*pFunction)(void);

#define APP_ADDRESS 								((uint32_t)0x0800A000)

#define COUNT_FAIL_JMP_ADDR  				((uint32_t)0x0803B000)

#define FLG_UPDATE_DONE_ADDR  			((uint32_t)0x0803F000)

#define FLG_UPDATE_FW_ADDR  				((uint32_t)0x0803F800)

uint32_t FLASH_GetFLG(uint32_t ADDR);

void FLASH_SetFLG(uint32_t ADDR, uint32_t state, uint8_t reset);

void goto_application(uint32_t addr);

void IWDG_Start(void);

#endif  // __osObjects
