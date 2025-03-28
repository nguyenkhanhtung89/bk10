#ifndef __FLASH_H
#define __FLASH_H

#include "cmsis_os.h"

#include "osObjects.h"

uint32_t FLASH_ReadData(uint32_t ADDR);

void FLASH_WriteData(uint32_t ADDR, uint32_t Data, uint8_t reset);

void FLASH_ReadBufData(uint32_t address, uint8_t *data, uint16_t len);

void FLASH_WriteBufData(uint32_t ADDR, uint8_t *data, uint16_t len);

#endif /* __FLASH_H */
