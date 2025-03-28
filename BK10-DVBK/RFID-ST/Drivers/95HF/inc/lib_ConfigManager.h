/* Define to prevent recursive inclusion --------------------------------------------*/
#ifndef _LIB_MANAGER_H
#define _LIB_MANAGER_H

/* Includes -------------------------------------------------------------------------*/
#include "lib_iso15693pcd.h"
/* Manager status and erroc code ----------------------------------------------------*/
#define MANAGER_SUCCESSCODE															RESULTOK
#define MANAGER_ERRORCODE_DEFAULT												0xF1
#define MANAGER_ERRORCODE_PORERROR											0xF2
#define MANAGER_ERRORCODE_COMMUNICATION_LOST						0xF3

/* Flags for PICC/PCD tracking  ----------------------------------------------------------*/
#define	TRACK_NOTHING			0x00
#define	TRACK_NFCTYPE1 		0x01 /* 0000 0001 */
#define	TRACK_NFCTYPE2 		0x02 /* 0000 0010 */
#define	TRACK_NFCTYPE3 		0x04 /* 0000 0100 */
#define	TRACK_NFCTYPE4A 	0x08 /* 0000 1000 */
#define	TRACK_NFCTYPE4B 	0x10 /* 0001 0000 */
#define	TRACK_NFCTYPE5 		0x20 /* 0010 0000 */
#define TRACK_ALL 				0xFF /* 1111 1111 */

/* Flags for Initiator/Target tracking  ------------------------------------------------------*/
#define	P2P_NOTHING				0x00
#define	INITIATOR_NFCA 		0x01 /* 0000 0001 */
#define	INITIATOR_NFCF 		0x02 /* 0000 0010 */
#define	TARGET_NFCA 			0x04 /* 0000 0100 */
#define	TARGET_NFCF 			0x08 /* 0000 1000 */
#define	TARGET_LLCPA 			0x10 /* 0000 0100 */
#define	TARGET_LLCPF 			0x20 /* 0000 1000 */
#define	INITIATOR_LLCPA 	0x40 /* 0000 0100 */
#define	INITIATOR_LLCPF 	0x80 /* 0000 1000 */
#define P2P_ALL 					0xFF /* 1111 1111 */

/* Flags for Proprietary P2P tracking  ------------------------------------------------------*/
#define	PP2P_NOTHING					0x00
#define	PP2P_INITIATOR_NFCA 	0x01 /* 0000 0001 */
#define	PP2P_TARGET_NFCA 		0x10 /* 0000 0100 */
#define PP2P_ALL 							0xFF /* 1111 1111 */

/* Flags for SelectMode  ----------------------------------------------------------*/
#define	SELECT_NOTHING		0x00
#define	SELECT_PCD			 	0x01 /* 0000 0001 */
#define	SELECT_PICC			 	0x02 /* 0000 0010 */
#define	SELECT_P2P			 	0x04 /* 0000 0100 */
#define SELECT_ALL 				0xFF /* 1111 1111 */

/* structure of the manager state --------------------------------------------------*/
typedef struct {
	uint8_t SelectedMode;
	uint8_t PcdMode;
	uint8_t PiccMode;
	uint8_t P2pMode;
	uint8_t Result;
	uint16_t timeoutEmul;
	uint16_t timeoutReadEmul;
}MANAGER_CONFIG;


bool TagScan(uint8_t* TagUID);
/* public function	 ----------------------------------------------------------------*/

uint8_t ST95HF_Init (void);

#endif


/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/

