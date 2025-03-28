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

const uint32_t Crc32Table[] = {
	0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
	0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD,
	0x4C11DB70,0x48D0C6C7,0x4593E01E,0x4152FDA9,0x5F15ADAC,0x5BD4B01B,0x569796C2,0x52568B75,
	0x6A1936C8,0x6ED82B7F,0x639B0DA6,0x675A1011,0x791D4014,0x7DDC5DA3,0x709F7B7A,0x745E66CD,
	0x9823B6E0,0x9CE2AB57,0x91A18D8E,0x95609039,0x8B27C03C,0x8FE6DD8B,0x82A5FB52,0x8664E6E5,
	0xBE2B5B58,0xBAEA46EF,0xB7A96036,0xB3687D81,0xAD2F2D84,0xA9EE3033,0xA4AD16EA,0xA06C0B5D,
	0xD4326D90,0xD0F37027,0xDDB056FE,0xD9714B49,0xC7361B4C,0xC3F706FB,0xCEB42022,0xCA753D95,
	0xF23A8028,0xF6FB9D9F,0xFBB8BB46,0xFF79A6F1,0xE13EF6F4,0xE5FFEB43,0xE8BCCD9A,0xEC7DD02D,
	0x34867077,0x30476DC0,0x3D044B19,0x39C556AE,0x278206AB,0x23431B1C,0x2E003DC5,0x2AC12072,
	0x128E9DCF,0x164F8078,0x1B0CA6A1,0x1FCDBB16,0x018AEB13,0x054BF6A4,0x0808D07D,0x0CC9CDCA,
	0x7897AB07,0x7C56B6B0,0x71159069,0x75D48DDE,0x6B93DDDB,0x6F52C06C,0x6211E6B5,0x66D0FB02,
	0x5E9F46BF,0x5A5E5B08,0x571D7DD1,0x53DC6066,0x4D9B3063,0x495A2DD4,0x44190B0D,0x40D816BA,
	0xACA5C697,0xA864DB20,0xA527FDF9,0xA1E6E04E,0xBFA1B04B,0xBB60ADFC,0xB6238B25,0xB2E29692,
	0x8AAD2B2F,0x8E6C3698,0x832F1041,0x87EE0DF6,0x99A95DF3,0x9D684044,0x902B669D,0x94EA7B2A,
	0xE0B41DE7,0xE4750050,0xE9362689,0xEDF73B3E,0xF3B06B3B,0xF771768C,0xFA325055,0xFEF34DE2,
	0xC6BCF05F,0xC27DEDE8,0xCF3ECB31,0xCBFFD686,0xD5B88683,0xD1799B34,0xDC3ABDED,0xD8FBA05A,
	0x690CE0EE,0x6DCDFD59,0x608EDB80,0x644FC637,0x7A089632,0x7EC98B85,0x738AAD5C,0x774BB0EB,
	0x4F040D56,0x4BC510E1,0x46863638,0x42472B8F,0x5C007B8A,0x58C1663D,0x558240E4,0x51435D53,
	0x251D3B9E,0x21DC2629,0x2C9F00F0,0x285E1D47,0x36194D42,0x32D850F5,0x3F9B762C,0x3B5A6B9B,
	0x0315D626,0x07D4CB91,0x0A97ED48,0x0E56F0FF,0x1011A0FA,0x14D0BD4D,0x19939B94,0x1D528623,
	0xF12F560E,0xF5EE4BB9,0xF8AD6D60,0xFC6C70D7,0xE22B20D2,0xE6EA3D65,0xEBA91BBC,0xEF68060B,
	0xD727BBB6,0xD3E6A601,0xDEA580D8,0xDA649D6F,0xC423CD6A,0xC0E2D0DD,0xCDA1F604,0xC960EBB3,
	0xBD3E8D7E,0xB9FF90C9,0xB4BCB610,0xB07DABA7,0xAE3AFBA2,0xAAFBE615,0xA7B8C0CC,0xA379DD7B,
	0x9B3660C6,0x9FF77D71,0x92B45BA8,0x9675461F,0x8832161A,0x8CF30BAD,0x81B02D74,0x857130C3,
	0x5D8A9099,0x594B8D2E,0x5408ABF7,0x50C9B640,0x4E8EE645,0x4A4FFBF2,0x470CDD2B,0x43CDC09C,
	0x7B827D21,0x7F436096,0x7200464F,0x76C15BF8,0x68860BFD,0x6C47164A,0x61043093,0x65C52D24,
	0x119B4BE9,0x155A565E,0x18197087,0x1CD86D30,0x029F3D35,0x065E2082,0x0B1D065B,0x0FDC1BEC,
	0x3793A651,0x3352BBE6,0x3E119D3F,0x3AD08088,0x2497D08D,0x2056CD3A,0x2D15EBE3,0x29D4F654,
	0xC5A92679,0xC1683BCE,0xCC2B1D17,0xC8EA00A0,0xD6AD50A5,0xD26C4D12,0xDF2F6BCB,0xDBEE767C,
	0xE3A1CBC1,0xE760D676,0xEA23F0AF,0xEEE2ED18,0xF0A5BD1D,0xF464A0AA,0xF9278673,0xFDE69BC4,
	0x89B8FD09,0x8D79E0BE,0x803AC667,0x84FBDBD0,0x9ABC8BD5,0x9E7D9662,0x933EB0BB,0x97FFAD0C,
	0xAFB010B1,0xAB710D06,0xA6322BDF,0xA2F33668,0xBCB4666D,0xB8757BDA,0xB5365D03,0xB1F740B4
};

uint32_t CRC32Software(uint8_t* pData, long Length){
	uint32_t nReg;
	uint32_t nTemp = 0;
	long i, n;
	nReg = 0xFFFFFFFF;
	for (n = 0; n < Length; n++){
		nReg ^= (uint32_t)pData[n];
		for (i = 0; i < 4; i++){
			nTemp = Crc32Table[(nReg >> 24) & 0xff];
			nReg <<= 8;
			nReg ^= nTemp;
		}
	}
	return nReg;
}

uint8_t CLS2_check_mes(char* s, int16_t len){//*CFV010000FEB5
	int16_t idx;
	uint32_t check_sum_val = 0, check_sum_cal = 0;
	char str_check_sum[4]  = {0x00, 0x00, 0x00, 0x00};
	
	if(len < 6) return 0;
	
	/* Copy checksum str(Hex str format) */
	memcpy(str_check_sum, &s[len - 2], 2);
	/* Read checksum */
	sscanf(str_check_sum, "%x", &check_sum_val);
	
	/* Calculate NMEA Checksum */
	for(idx = 0; idx < len - 2; idx++){
		check_sum_cal += s[idx];
	}
	check_sum_cal = check_sum_cal & 0xFF;
	/* Check invaild check sum */
	if(check_sum_val != check_sum_cal){
		return 0;
	}
	return 1;
}

void u32_to_bytes(uint32_t num, uint8_t *msg){
	msg[3] = num >> 24;
	msg[2] = (num >> 16) & 0xFFFFFF;
	msg[1] = (num >> 8) & 0xFFFF;
	msg[0] = num & 0xFF;
}

uint32_t bytes_to_u32(uint8_t *msg){
	uint32_t num = 0;
	num |= (msg[3] << 24);
	num |= ((msg[2] << 16) & 0x00FF0000);
	num |= ((msg[1] << 8)  & 0x0000FF00);
	num |= msg[0] & 0x000000FF;

	return num;
}

uint8_t get_number(char *msg){
	uint8_t num = 0;
	num = (10 * (msg[0] & 0x0F)) + (msg[1] & 0x0F);
	
	return num;
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

//static double degToRad(double deg) {
//	return deg * (double)0.017453292519943295;
//}

uint8_t byte_arr_cmp(uint8_t *src, uint8_t *dest, uint8_t size){
	__IO uint8_t idx;
	
	for(idx = 0; idx < size; idx ++){
		if(src[idx] != dest[idx]) return 0;
	}
	
	return 1;
}

void str_trim(uint8_t *src){
	uint32_t len = 0;
	uint16_t idx = 0;
	
	len = strlen((char*)src);
	if(len == 0){
		return;
	}
	
	/* Remove 0x0D and 0x0A end string */
	for(idx = len - 1; idx > 0; idx--){
		if((src[idx] == 0x0D) || (src[idx] == 0x0A) || (src[idx] == 0x20)){
			src[idx] = 0;
		}else{
			break;
		}
	}
}

void sTrim(uint8_t *src){
	uint32_t len = 0;
	uint16_t idx = 0;
	uint8_t flag_= 0;
	
	len = strlen((char*)src);
	if(len == 0){
		return;
	}
	/* Remove 0x0D and 0x0A end string */
	for(idx = 0; idx < len; idx++){
		if((src[idx] == 0x0D) || (src[idx] == 0x0A)){
			src[idx] = 0;
			flag_ = 1;
		}else if(flag_ == 1){
			src[idx] = 0;
		}
	}
}

uint32_t calcCrc32(uint8_t *pBuf, uint32_t nSize){
	uint32_t index = 0;
	rcu_periph_clock_enable(RCU_CRC);
	
	/* Reset DR */
	crc_data_register_reset();     
	
	/* Calculate CRC */
	for(index = 0; index < nSize; index++){
		crc_free_data_register_write(pBuf[index]);
	}
	
	return crc_data_register_read();
}

void ArraySort(uint16_t vals[], uint8_t lenght){
	uint8_t l = 0, exchange = 1;
	uint16_t tmp = 0;

	/* Sort tab */
	while(exchange == 1){
		exchange = 0;
		for(l = 0; l < (lenght - 1); l++){
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

void str_upper(char *str){
	int i=0;
  while (str[i]){
    str[i] = toupper(str[i]);
    i++;
  }
}

void str_lower(char *str){
	int i=0;
  while (str[i]){
    str[i] = tolower(str[i]);
    i++;
  }
}

//void str_str_upper(char *src, char *dest){
//	int i=0;
//  while (src[i]){
//    dest[i] = toupper(src[i]);
//    i++;
//  }
//}

//void str_str_lower(char *src, char *dest){
//	int i=0;
//  while (src[i]){
//    dest[i] = tolower(src[i]);
//    i++;
//  }
//}

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

uint8_t number_check_valid(char* msg){
	uint8_t index;
	uint8_t len = strlen(msg);
	for(index = 0; index < len ; index++){
		if((msg[index] < 48) || (msg[index] > 57)){
			return 0;
		}
	}
	return 1;
}

int16_t buf_cmp(u8 *src, u8 *des, u16 des_len){
	u16 j;
	for(j = 0; j < des_len; j++){
		if(src[j] != des[j]) return -1;
	}
	return 0;
}


