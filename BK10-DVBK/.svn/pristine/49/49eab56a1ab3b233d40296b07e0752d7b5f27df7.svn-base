#ifndef __EEPROM_H
#define __EEPROM_H

#include "stm32f30x.h"

#include "cmsis_os.h"

#define FLASH_PAGE_SIZE    					0x800

#define FW_PKG_SZ   2048

#define ADD_PROGRAM  								APP_ADDRESS

void LoadFwInfo(void);
int FirmwareInfoParse(uint8_t *data);
void FirmwarePackageParse(uint8_t *data);
FLASH_Status WriteFwPackage(u32 addr, u8* buf, u16 num);
int16_t get_update_state(void);
void firmware_thrd_init(void);
u32 FLASH_GetFLG(u32 addr);
void FLASH_SetFLG(u32 addr, u32 state, u8 reset);

u32 flash_read_u32(u32 address);
#endif /* __EEPROM_H */
