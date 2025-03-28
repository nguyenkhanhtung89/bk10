
#ifndef __RFID_H
#define __RFID_H

#include "cmsis_os.h"
#include "lib_ConfigManager.h"
#include "utils.h"
#include "Serial.h"

#define	MAX_SIZE_DRIVER_LICENSE	       16
#define	MAX_SIZE_TAG_DATE     	       10
#define	MAX_SIZE_DRIVER_NAME		       44
#define	MAX_SIZE_TAG_INFO			         80
#define TAG_UID_SIZE									 8

extern uint8_t TagUID[16];
extern u64 UID__;

extern u8 drv_reload_time;

#define	LOGIN_CMD 1
#define	SYNC_CMD 0

typedef struct{
	uint8_t ST95_Status;
	uint8_t ST95_Step;
}ST95_def;

ST95_def get_ST95_data(void);

uint8_t get_ST95_status(void);

void RFID_Init(void);

void vRFIDTask(const void* args);

int WriteDriverInfoToTag(DriverInfoDef msg);

void send_tag_uid(u8* uid, u8 cmd);

#endif /* __RFID_H */

