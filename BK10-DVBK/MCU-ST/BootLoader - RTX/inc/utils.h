#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "stm32f30x.h"

uint8_t CRC_8BitsCompute(uint8_t* data, uint32_t size);

u32 calcCrc32(u8 *pBuf, u32 nSize);

#endif /* __UTILS_H */
