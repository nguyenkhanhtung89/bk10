#include "cmsis_os.h"
#include "lib_nfctype5pcd.h"

uint8_t ReadTagData( uint8_t *pData ){
	// Try to determine the density by reading the first sector (128 bytes)
	if (ISO15693_ReadBytesTagData(ISO15693_HIGH_DENSITY, ISO15693_LRiS64K, pData, TAG_LENGHT, 0) != ISO15693_SUCCESSCODE){
		if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRiS64K, pData, TAG_LENGHT, 0) != ISO15693_SUCCESSCODE){
			if (ISO15693_ReadBytesTagData(ISO15693_HIGH_DENSITY, ISO15693_LRi2K, pData, TAG_LENGHT, 0) != ISO15693_SUCCESSCODE){
				if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRi2K, pData, TAG_LENGHT, 0) != ISO15693_SUCCESSCODE){
					return 0;
				}
			}
		}
	}
	return 1;
}

static uint8_t firstSector[140];
uint8_t ISO15693_Write_BytesData(uint8_t *pData, uint8_t Len){
	uint8_t status;
	uint8_t tagDensity = ISO15693_HIGH_DENSITY;
	// Try to determine the density by ready the first sector (128 bytes)
	if (ISO15693_ReadBytesTagData(ISO15693_HIGH_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE){
		if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE){
			if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRi2K, firstSector, 127, 0) != ISO15693_SUCCESSCODE){
				if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRi2K, firstSector, 127, 0) != ISO15693_SUCCESSCODE){
					return PCDNFCT5_ERROR;
				}
			}
		}
		tagDensity = ISO15693_LOW_DENSITY;
	}
	
	// Write the tag
	errchk(ISO15693_WriteBytes_TagData(tagDensity, pData, Len, 0));
	
	return PCDNFCT5_OK;	
Error:
	return PCDNFCT5_ERROR;
}

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
