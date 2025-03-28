#include "utils.h"

const uint8_t CRC_Table[] = { 0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126,
    32, 163, 253, 31, 65, 157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189,
    62, 96, 130, 220, 35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128,
    222, 60, 98, 190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67,
    161, 255, 70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187,
    89, 7, 219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196,
    154, 101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122,
    36, 248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139, 87,
    9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22, 233,
    183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168, 116,
    42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53 };

uint8_t CRC_8BitsCompute(uint8_t* data, uint32_t size){
  /* Reset CRC data register to avoid overlap when computing new data stream */
  uint8_t CRC_Data = 0;
	
  while (size--) {
    CRC_Data = CRC_Table[*(data++) ^ CRC_Data];
  }
	
  /* Return the CRC value */
  return CRC_Data;
}

int16_t char_index_of(char *src, char ch, uint8_t count){
	uint16_t idx	= 0;
	uint16_t clen	= strlen(src);
  uint8_t cnt   = 0;  
	
	for(; idx < clen; idx++){
		if(src[idx] == ch){
			cnt ++;
			if(cnt == count){
			  return idx;
			}
		}
	}
	return -1;
}

int16_t find_char_of(uint8_t * src, char ch, uint8_t count, uint8_t max_len_buffer ){
	uint16_t idx	= 0;
  uint8_t cnt   = 0;  
	
	for(; idx < max_len_buffer; idx++){
		if(src[idx] == ch){
			cnt ++;
			if(cnt == count){
			  return idx;
			}
		}
	}
	return -1;
}

int16_t str_find(char *src, char *dest){
	char * pch;
	int16_t idx = -1;
	
	pch = strstr(src, dest);
	if(pch != NULL){
		idx = pch - src;
	}
	
	return idx;
}

uint8_t cal_sum_str(char *src){
	uint32_t sum = 0;
	uint16_t idx = 0;
	uint16_t len = strlen(src);
	
	for(; idx < len; idx++){
		sum += src[idx];
	}
	
	return (sum & 0xFF);
}

uint8_t cal_sum_buffer(uint8_t *src, uint16_t len){
	uint32_t sum = 0;
	uint16_t idx = 0;
	
	for(; idx < len; idx++){
		sum += src[idx];
	}
	
	return (sum & 0xFF);
}

uint16_t cal_sum16_buffer(uint8_t *src, uint16_t len){
	uint32_t sum = 0;
	uint16_t idx = 0;
	
	for(; idx < len; idx++){
		sum += src[idx];
	}
	
	return (sum & 0xFFFF);
}

uint8_t byte_arr_cmp(uint8_t *src, uint8_t *dest, uint8_t size){
	__IO uint8_t idx;
	
	for(idx = 0; idx < size; idx ++){
		if(src[idx] != dest[idx]) return 0;
	}
	
	return 1;
}

u32 calcCrc32(u8 *pBuf, u32 nSize){
	u32 index = 0;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
	
	/* Reset DR */
	CRC_ResetDR();     
	
	/* Calculate CRC */
	for(index = 0; index < nSize; index++){
		CRC->DR = (u32)pBuf[index];
	}
	
	return (CRC->DR);
}

void ArraySort(uint16_t vals[], uint8_t lenght){
	uint8_t l = 0, exchange = 1;
	uint16_t tmp = 0;
	
	/* Sort tab */
	while(exchange == 1){
		exchange = 0;
		for(l = 0; l < lenght - 1; l++){
			if(vals[l] > vals[l+1]){
				 tmp = vals[l];
				 vals[l] = vals[l+1];
				 vals[l+1] = tmp;
				 exchange=1;
			}
		}
	}
}

int compare( const void* a, const void* b){
	int int_a = * ( (int*) a );
	int int_b = * ( (int*) b );
	
	if ( int_a == int_b ) return 0;
	else if ( int_a < int_b ) return -1;
	else return 1;
}

int char_replace(char *src, char rep, char with){
	int i=0;
	int count=0;
  while (src[i]){
    if(src[i] == rep){
			src[i] = with;
			count++;
		}
    i++;
  }
	return count;
}
