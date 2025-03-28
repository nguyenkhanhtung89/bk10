#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "stm32f30x.h"

#define MAX_SUB_LEN 16

uint8_t CRC_8BitsCompute(uint8_t* data, uint32_t size);

uint8_t CLS2_check_mes(char* s, int16_t len);

void u32_to_bytes(u32 num, u8* msg);

u32 bytes_to_u32(u8* msg);

u8 get_number(char *msg);

int16_t char_index_of(char* src, char ch, uint8_t count);

int16_t str_find(char *src, char* dest);

uint8_t cal_sum_str(char *src);

uint8_t cal_sum_buffer(uint8_t *src, uint16_t len);

uint16_t cal_sum16_buffer(uint8_t *src, uint16_t len);

uint8_t byte_arr_cmp(uint8_t *src, uint8_t *dest, uint8_t size);

void sTrim(uint8_t *src);

void str_trim(uint8_t *src);

u32 calcCrc32(u8 *pBuf, u32 nSize);

void ArraySort(uint16_t vals[], uint8_t lenght);

int compare (const void * a, const void * b);

void str_upper(char *str);

int16_t find_char_of(uint8_t * src, char ch, uint8_t count, uint8_t max_len_buffer );

void str_lower(char *str);

uint32_t CRC32Software(uint8_t* pData, long Length);

//void str_str_upper(char *src, char *dest);

//void str_str_lower(char *src, char *dest);

int char_replace(char *src, char rep, char with);
uint8_t number_check_valid(char* msg);

#endif /* __UTILS_H */
