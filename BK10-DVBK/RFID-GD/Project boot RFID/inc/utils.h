#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "gd32f30x.h"

uint8_t CRC_8BitsCompute(uint8_t* data, uint32_t size);

uint32_t CRC32Software(uint8_t* pData, long Length);
#endif /* __UTILS_H */
