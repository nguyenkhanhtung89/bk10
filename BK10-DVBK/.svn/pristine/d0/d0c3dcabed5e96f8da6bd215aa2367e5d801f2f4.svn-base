#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "gd32f30x.h"

#define MAX_SUB_LEN 16

uint8_t CRC_8BitsCompute(uint8_t* data, uint32_t size);

int16_t char_index_of(char* src, char ch, uint8_t count);

int16_t str_find(char *src, char* dest);

uint8_t cal_sum_str(char *src);

uint8_t cal_sum_buffer(uint8_t *src, uint16_t len);

uint16_t cal_sum16_buffer(uint8_t *src, uint16_t len);

uint8_t byte_arr_cmp(uint8_t *src, uint8_t *dest, uint8_t size);

uint32_t calcCrc32(uint8_t *pBuf, uint32_t nSize);

void ArraySort(uint16_t vals[], uint8_t lenght);

int compare (const void * a, const void * b);

int16_t find_char_of(uint8_t * src, char ch, uint8_t count, uint8_t max_len_buffer );

int char_replace(char *src, char rep, char with);

#endif /* __UTILS_H */
