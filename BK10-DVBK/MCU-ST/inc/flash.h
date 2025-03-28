#ifndef __FLASH_H
#define __FLASH_H

#include "cmsis_os.h"
#include "osObjects.h"

u32 FLASH_ReadData(u32 ADDR);

void FLASH_WriteData(u32 ADDR, u32 Data, uint8_t reset);

void FLASH_ReadBufData(uint32_t address, uint8_t *data, uint16_t len);

FLASH_Status FLASH_WriteBufData(u32 addr, u8* buf, u16 num);

#endif /* __FLASH_H */
