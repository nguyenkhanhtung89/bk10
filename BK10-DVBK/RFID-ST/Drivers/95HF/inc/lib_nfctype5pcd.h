#ifndef _LIB_NFCTYPE5PCD_H
#define _LIB_NFCTYPE5PCD_H

#include "lib_iso15693pcd.h"

/* Error codes */
#define PCDNFCT5_OK 											PCDNFC_OK
#define PCDNFCT5_ERROR 										PCDNFC_ERROR
#define PCDNFCT5_ERROR_MEMORY_TAG					PCDNFC_ERROR_MEMORY_TAG
#define PCDNFCT5_ERROR_MEMORY_INTERNAL		PCDNFC_ERROR_MEMORY_INTERNAL
#define PCDNFCT5_ERROR_LOCKED 						PCDNFC_ERROR_LOCKED
#define PCDNFCT5_ERROR_NOT_FORMATED				PCDNFC_ERROR_NOT_FORMATED

/* Functions */
uint8_t ReadTagData(uint8_t *pData);

uint8_t ISO15693_Write_BytesData(uint8_t *pData, uint8_t Len);

#endif
