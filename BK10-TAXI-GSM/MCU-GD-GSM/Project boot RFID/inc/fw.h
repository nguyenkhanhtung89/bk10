#ifndef __EEPROM_H
#define __EEPROM_H

#include "gd32f30x.h"

#include "cmsis_os.h"

#define FLASH_PAGE_SIZE    					0x800

#define FW_PKG_SZ   2048

#define ADD_PROGRAM  								APP_ADDRESS

void LoadFwInfo(void);
int FirmwareInfoParse(uint8_t *data);
void FirmwarePackageParse(uint8_t *data);
fmc_state_enum WriteFwPackage(uint32_t addr, uint8_t* buf, uint16_t num);
int16_t get_update_state(void);
void firmware_thrd_init(void);
uint32_t FLASH_GetFLG(uint32_t addr);
void FLASH_SetFLG(uint32_t addr, uint32_t state, uint8_t reset);

uint32_t flash_read_uint32_t(uint32_t address);
#endif /* __EEPROM_H */
