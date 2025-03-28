#include "RFID.h"
#include "interface.h"

static ST95_def ST95Data;
uint8_t TagUID[16];
u64 UID__ = 0;

osThreadId RFID_thread;
osThreadDef(vRFIDTask, osPriorityNormal, 1, 2048);

osSemaphoreId sRFIDFree;
osSemaphoreDef(sRFIDFree);

ST95_def get_ST95_data(void){
	return ST95Data;
}

uint8_t get_ST95_status(void){
	return ST95Data.ST95_Status;
}

void RFID_Init(void){
	/* Create RFID semaphore */
	sRFIDFree = osSemaphoreCreate(osSemaphore(sRFIDFree), 1);
	/* create RFID thread */
	RFID_thread = osThreadCreate(osThread(vRFIDTask), NULL);
  if(!RFID_thread){
		return;
	}
}

static void RFID_Reset(void){
	spi_i2s_deinit(RFTRANS_95HF_SPI);
	spi_disable(RFTRANS_95HF_SPI);
	RFTRANS_95HF_IRQIN_LOW();
	ST95Data.ST95_Status = 0;
	gpio_bit_reset(POWER_95HF_GPIO_PORT, POWER_95HF_PIN);
	osDelay(2000);
	
	gpio_bit_set(POWER_95HF_GPIO_PORT, POWER_95HF_PIN);
	ST95Data.ST95_Status = ST95HF_Init();
}

void send_tag_uid(u8* uid, u8 cmd){
	u8 cs = 0;
	u8 send_msg[16];
	memset(send_msg, 0x0, 16);
	send_msg[0] = '>';
	send_msg[1] = cmd;
	memcpy(send_msg + 2, uid, TAG_UID_SIZE);
	cs = CRC_8BitsCompute(send_msg, TAG_UID_SIZE + 2);
	send_msg[TAG_UID_SIZE + 2] = cs;
	SerialTxBufferPush(send_msg, TAG_UID_SIZE + 3);
}

uint8_t Login_msg[256];
static uint8_t _Name[MAX_SIZE_DRIVER_NAME], _License[MAX_SIZE_DRIVER_LICENSE], _IssDate[MAX_SIZE_TAG_DATE + 1], _ExpDate[MAX_SIZE_TAG_DATE + 1];
uint8_t Driver_Login(uint8_t *msg, uint8_t* Tag_UID){
	uint8_t cs = 0;
	if(cal_sum_buffer(msg, MAX_SIZE_DRIVER_LICENSE - 1) == msg[MAX_SIZE_DRIVER_LICENSE - 1]){
		if(cal_sum_buffer(msg + MAX_SIZE_DRIVER_LICENSE, MAX_SIZE_DRIVER_NAME - 1) == msg[MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_DRIVER_NAME - 1]){
			/* get License */
			memset(_License, 0, MAX_SIZE_DRIVER_LICENSE);
			memcpy(_License, msg, MAX_SIZE_DRIVER_LICENSE - 1);
			
			/* get Name */
			memset(_Name, 0, MAX_SIZE_DRIVER_NAME);
			memcpy(_Name, msg + MAX_SIZE_DRIVER_LICENSE, MAX_SIZE_DRIVER_NAME - 1);
			
			/* get issuance Date */
			memset(_IssDate, 0, MAX_SIZE_TAG_DATE + 1);
			memcpy(_IssDate, msg + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_DRIVER_NAME , MAX_SIZE_TAG_DATE);
			
			/* get expired Date */
			memset(_ExpDate, 0, MAX_SIZE_TAG_DATE + 1);
			memcpy(_ExpDate, msg + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_DRIVER_NAME + MAX_SIZE_TAG_DATE, MAX_SIZE_TAG_DATE);
			
			/* measure login message */
			memset(Login_msg, 0x0, 256);
			Login_msg[0] = '>';
			Login_msg[1] = 0x1;
			
			/* add tag UID */
			memcpy(Login_msg + 2, Tag_UID, TAG_UID_SIZE);
			/* add Name */
			memcpy(Login_msg + 2 + TAG_UID_SIZE, _Name, MAX_SIZE_DRIVER_NAME);
			/* add License */
			memcpy(Login_msg + 2 + TAG_UID_SIZE + MAX_SIZE_DRIVER_NAME, _License, MAX_SIZE_DRIVER_LICENSE);
			/* add issdate */
			memcpy(Login_msg + 2 + TAG_UID_SIZE + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE, _IssDate, MAX_SIZE_TAG_DATE);
			/* add expdate */
			memcpy(Login_msg + 2 + TAG_UID_SIZE + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_TAG_DATE, _ExpDate, MAX_SIZE_TAG_DATE);
			
			cs = CRC_8BitsCompute(Login_msg, (2 + TAG_UID_SIZE + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_TAG_DATE + MAX_SIZE_TAG_DATE));
			Login_msg[2 + TAG_UID_SIZE + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_TAG_DATE + MAX_SIZE_TAG_DATE] = cs;
			
			SerialTxBufferPush(Login_msg, (2 + TAG_UID_SIZE + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_TAG_DATE + MAX_SIZE_TAG_DATE + 1));
		}else{
			return 0;
		}
	}else{
		return 0;
	}

	return 1;
}

static void request_driver_info(void){
	u8 cs = 0;
	u8 send_msg[16];
	memset(send_msg, 0x0, 16);
	send_msg[0] = '>';
	send_msg[1] = 0x02;

	cs = CRC_8BitsCompute(send_msg, 2);
	send_msg[2] = cs;
	SerialTxBufferPush(send_msg, 3);
}

static uint8_t Reset_Cycles;
u8 drv_reload_time = 0;
static uint8_t read_rdy = 0;
void vRFIDTask(const void* args){
	ST95Data.ST95_Status = ST95HF_Init();

	for(;;){
		if(!ST95Data.ST95_Status){
			if(Reset_Cycles <= 12){
				Reset_Cycles++;
				if(Reset_Cycles % 2 == 0){//init ST95HF 10s cycles in 60s
					ST95Data.ST95_Status = ST95HF_Init();
				}
			}else{
				Reset_Cycles = 15;
			}
			
			osDelay(1000);
			/* Watchdog reset */
			fwdgt_counter_reload();
			osThreadYield(); 
			continue;
		}
		
		/* take semaphore */
		osSemaphoreWait(sRFIDFree, osWaitForever);
		if(TagScan(TagUID)){
			UID__ = (u64)(((u64)TagUID[4] << 32) | ((u64)TagUID[3] << 24) | ((u64)TagUID[2] << 16) | ((u64)TagUID[1] << 8) | ((u64)TagUID[0] & 0xFF));
			if(read_rdy == 0){
				osDelay(500);
				/* release semaphore*/
				osSemaphoreRelease(sRFIDFree);
				/* Watchdog reset */
				fwdgt_counter_reload();
				continue;
			}
			read_rdy = 0;
			Set_Led_Status(LED_DRIVER, PutinTag);
			ec21_data.DriverInfo.LoginStatus = STATUS_UNKNOWN;
			drv_reload_time = 0;
			send_tag_uid(TagUID, LOGIN_CMD);
		}else{
			read_rdy = 1;
		}
		
		if(ec21_data.DriverInfo.LoginStatus == STATUS_UNKNOWN){
			//Request driver info	
			drv_reload_time++;
			if(drv_reload_time >= 6){
				drv_reload_time = 0;
				request_driver_info();
			}
		}else{
			drv_reload_time = 0;
		}
		osDelay(500);
		
		/* release semaphore*/
		osSemaphoreRelease(sRFIDFree);
		/* Watchdog reset */
		fwdgt_counter_reload();
		osThreadYield(); 
	}
}

/* Variables for tag writing */
static uint8_t Driver_Tag[MAX_SIZE_TAG_INFO];
int WriteDriverInfoToTag(DriverInfoDef msg){
	char LastUIDFound[20] = {' '};
	uint8_t cs = 0;
	if(!ST95Data.ST95_Status) return -2; 
	
	osDelay(200);/* wait for deactive read tag info */
	
	/* Build Tag data */
	memset(Driver_Tag, 0, MAX_SIZE_TAG_INFO);
	memcpy(Driver_Tag, msg.License, MAX_SIZE_DRIVER_LICENSE - 1);
	
	cs = cal_sum_buffer((uint8_t*)msg.License, MAX_SIZE_DRIVER_LICENSE - 1);
	Driver_Tag[MAX_SIZE_DRIVER_LICENSE - 1] = cs;
	
	memcpy(Driver_Tag + MAX_SIZE_DRIVER_LICENSE, msg.Name, MAX_SIZE_DRIVER_NAME - 1);
	cs = cal_sum_buffer((uint8_t*)msg.Name, MAX_SIZE_DRIVER_NAME - 1);
	Driver_Tag[MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_DRIVER_NAME - 1] = cs;
	
	memcpy(Driver_Tag + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_DRIVER_NAME, msg.IssDate, MAX_SIZE_TAG_DATE);
	memcpy(Driver_Tag + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_DRIVER_NAME + MAX_SIZE_TAG_DATE, msg.ExpDate, MAX_SIZE_TAG_DATE);
	
	/* take semaphore */
	osSemaphoreWait(sRFIDFree, 5000);
	if(TagScan(TagUID)){
		/* Write a message */
		if(memcmp(LastUIDFound, TagUID, 8)){
			memcpy(LastUIDFound, TagUID, 8);
			if(ISO15693_Write_BytesData(Driver_Tag, MAX_SIZE_TAG_INFO) != PCDNFCT5_OK){
				RFID_Reset();
				osSemaphoreRelease(sRFIDFree);
				return -1;
			}
		}else{
			PCD_FieldOff();
			RFID_Reset();
			osSemaphoreRelease(sRFIDFree);
			return -1;
		}
	}
	osSemaphoreRelease(sRFIDFree);
	return 0;
}
