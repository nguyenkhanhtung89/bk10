#include "fw.h"
#include "utils.h"
#include <string.h>
#include "Serial.h"
#include "utils.h"
#include "osObjects.h"

char text[256];
static __IO uint32_t write_address = 0;
static	__IO uint8_t update_fw_flag;
static	__IO uint32_t total_pack;
static	__IO uint32_t total_crc;
static	__IO uint16_t fw_package_id;
static	__IO uint8_t is_send_request;
static	__IO uint8_t is_receiver_package;
static	__IO uint8_t receiver_timeout;
static	__IO uint8_t repush_package;
void vFirmwareUpdateHandler(const void* args);
osThreadId fwupdate_thread;
osThreadDef(vFirmwareUpdateHandler, osPriorityNormal, 1, 2048);

static void PushRequestPackage(uint16_t pk_id);
static void PushRequestFirmware(void);

void disable_irqn(void){
	NVIC_DisableIRQ(USART0_IRQn);
}

void enable_irqn(void){
	NVIC_EnableIRQ(USART0_IRQn);
}

fmc_state_enum WriteFwPackage(uint32_t addr, uint8_t* buf, uint16_t num){
	uint32_t *pdata;
	uint32_t data = 0;
	uint32_t idx  = 0;
	fmc_state_enum FLASHStatus = FMC_READY;	
	
	disable_irqn();
	fmc_unlock();	
	/* clear all pending flags */
	fmc_flag_clear(FMC_FLAG_BANK0_END);
	fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
	fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	/* Erase Page */	
	FLASHStatus = fmc_page_erase(addr + 0xFF);
	if(FLASHStatus != FMC_READY){
		fmc_lock();
		enable_irqn();
		return FLASHStatus;
	}
	
	pdata = (uint32_t*)(&buf[0]);
	/* Write data */
	for(idx = 0; idx < num; idx += 4){
		/* write to flash */
		data = *pdata;
		FLASHStatus = fmc_word_program(addr, data);
		pdata++;
		addr += 4;
		
		if(FLASHStatus != FMC_READY){
			fmc_lock();
			enable_irqn();
			return FLASHStatus;
		}
	}
	
	/* Re-lock flash */
	fmc_lock();
	enable_irqn();
	return FLASHStatus;
}

/* VERSION_NUMPACK_CRC# */
int FirmwareInfoParse(uint8_t *data){
	/* Check backup flag */
	if(update_fw_flag){
		return -2;
	}

	total_pack = (data[0] << 8) | data[1];
	total_crc = (data[2] << 24) | (data[3] << 16) | (data[4] << 8) | (data[5] & 0xFF);

	if((total_pack == 0) || (total_crc == 0)){
		return -1;
	}
	
	/* Set value repare upload */
	update_fw_flag = 1;
	
	/* Get address to write data to flash */
	write_address = ADD_PROGRAM;
	
	
	/* package first */
	fw_package_id = 0;
	
	is_send_request = 0;
	
	//FLASH_SetLatency(FLASH_Latency_0);
	return 0;
}

char TEST_[128];
uint8_t num_1[4] = {0x0, 0x0, 0x0, 0x1};
uint8_t num_0[4] = {0x0, 0x0, 0x0, 0x0};

void FirmwarePackageParse(uint8_t *data){//> 0xFD 
	uint32_t pkg_id, pkg_len, pkg_crc, cal_crc;
	
	pkg_id = ((data[0] << 8) | data[1]);
	pkg_len = ((data[2] << 8) | data[3]);
	pkg_crc = ((data[4] << 24) | (data[5] << 16) | (data[6] << 8) | (data[7] & 0xFF));
	
	if(pkg_id < (total_pack - 1)){
		if(pkg_len != FLASH_PAGE_SIZE){
			SerialPushStr("ERROR!!! Package lenght", END_LF);
		}
	}
	
	/* Calculate CRC */
	cal_crc = CRC32Software((uint8_t *)data + 8, pkg_len);
	
	/* Check package and CRC */
	if((pkg_id == fw_package_id) && (cal_crc == pkg_crc)){
		/* Write firmware data to Flash */
		
		sprintf(TEST_, "WRITE ADDRESS -> %X, LEN = %d", write_address, pkg_len);
		SerialPushStr(TEST_, END_LF);
		
		if(WriteFwPackage(write_address, data + 8, pkg_len) != FMC_READY){
			return;
		}
		
		cal_crc = CRC32Software((uint8_t *)write_address, pkg_len);
		
		if(cal_crc != pkg_crc){
			return;
		}else{
			sprintf(TEST_, "WRITE PACKAGE %d SUCCESS!!!", fw_package_id);
			SerialPushStr(TEST_, END_LF);
		}

		is_receiver_package = 1;
		
		/* Incream address to write */
		write_address += pkg_len;
		
		/* Repush package zero */
		repush_package = 0;
		
		/* End package */
		if(pkg_id == (total_pack - 1)){
			uint32_t fw_start_addr, fw_size, fw_cal_crc;
			
			/* Get address begin write firmware */
			fw_start_addr = ADD_PROGRAM;
			
			/* Get length of firmware */
			fw_size = write_address - fw_start_addr;
			
			/* Calculate CRC32 of firmware */
			fw_cal_crc = CRC32Software((uint8_t *)fw_start_addr, fw_size);
			
			/* Check CRC */
			if(fw_cal_crc == total_crc){
				SerialPushStr("CHECK TOTAL CRC FW SUCCESS", END_LF);
				/* go to new firmware */
				if(WriteFwPackage(FLG_UPDATE_DONE_ADDR, num_1, 4) == FMC_READY){
					SerialPushStr("SET FLG_UPDATE_DONE_ADDR SUCCESS", END_LF);
				}else{
					SerialPushStr("SET FLG_UPDATE_DONE_ADDR ERROR", END_LF);
				}
				if(WriteFwPackage(FLG_UPDATE_FW_ADDR, num_0, 4) == FMC_READY){
					SerialPushStr("SET FLG_UPDATE_FW_ADDR SUCCESS", END_LF);
				}else{
					SerialPushStr("SET FLG_UPDATE_FW_ADDR ERROR", END_LF);
				}
				
				update_fw_flag = 0;
				total_pack = 0;
				total_crc = 0;
				write_address = 0;
				fw_package_id = 0;
				SerialPushStr("SET FLAG GOTO NEW FIRMWARE SUCCESS", END_LF);
				osDelay(3000);
				/* Soft Reset */
				NVIC_SystemReset();
			}else{
				SerialPushStr("CHECK TOTAL CRC FW ERROR!!!", END_LF);
				/* Recalculate address */
				write_address = ADD_PROGRAM;
				/* Reset package id*/
				fw_package_id = 0;
				
				/* Re-upload firmware */
				PushRequestFirmware();
				is_send_request = 0;
			}
			update_fw_flag = 0;
			total_pack = 0;
		}
		/* Imcream package ID */
		fw_package_id = fw_package_id + 1;
		/* Set flag request to false */
		is_send_request = 0;
	}else{
//		sprintf(text, "\nCRC FAILED %u - %u, PackageID %d, %d\n", cal_crc, pkg_crc, pkg_id, fw_package_id);
//		SerialPushStr(text, END_LF);
		
		/* Repush package */
		repush_package += 1;
		
		/* Reset package */
		if(repush_package > 10){
			//SerialPushStr("REPUSH PACKAGE TO MANY !!!", END_LF);
			
			/* Soft Reset */
			NVIC_SystemReset();
		}
		
		/* Repush */
		PushRequestPackage(fw_package_id);
	}
}

void vFirmwareUpdateHandler(const void* args){
	for(;;){
		osDelay(500);
		if((!update_fw_flag) || (total_pack == 0)){
			osDelay(1000);
			continue;
		}
		
		if(is_send_request && !is_receiver_package){
			receiver_timeout ++;
			if(receiver_timeout >= 10){
				is_send_request = 0;
			}
			continue;
		}else{
			receiver_timeout = 0;
		}

		if(update_fw_flag && !is_send_request){
			PushRequestPackage(fw_package_id);

			is_receiver_package = 0;
		}
		
		osThreadYield(); 
	}
}

uint8_t tmp_msg[120];
static void PushRequestPackage(uint16_t pk_id){
	tmp_msg[0] = '>';
	tmp_msg[1] = 7;
	tmp_msg[2] = pk_id >> 8;
	tmp_msg[3] = pk_id & 0xFF;
	
	SerialPushBufferWithCS(tmp_msg, 4);
	
	/* Set flag request sent */
	is_send_request = 1;
	receiver_timeout = 0;
}

static void PushRequestFirmware(void){
	//re update
	uint8_t crc = 0;
	tmp_msg[0] = '>';
	tmp_msg[1] = 6;
	crc = CRC_8BitsCompute(tmp_msg, 2);
	tmp_msg[2] = crc;
	
	SerialPushBuffer(tmp_msg, 3);
}

uint32_t flash_read_uint32_t(uint32_t address){
	return (*(volatile uint32_t*)address);
}

void FLASH_SetFLG(uint32_t addr, uint32_t state, uint8_t reset){
	/* Wait for last operation to be completed */
  fmc_state_enum status = FMC_READY;
	
	if(addr < COUNT_FAIL_JMP_ADDR){
		return;
	}
	
//  status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
//	if(status != FLASH_COMPLETE){
//		return;
//	}
	
	
	disable_irqn();
	
	fmc_unlock();
	/* clear all pending flags */
	fmc_flag_clear(FMC_FLAG_BANK0_END);
	fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
	fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
		
	
	/* Erase page */
	if(fmc_page_erase(addr) != FMC_READY){
		fmc_lock();
		enable_irqn();
		return;
	}
	
	/* Write data */
	fmc_word_program(addr, state);
	
	fmc_lock();
	enable_irqn();
	if(reset){
		osDelay(1000);
		NVIC_SystemReset();
	}
}

//void FLASH_SetFLG(uint32_t ADDR, uint32_t state, uint8_t reset){
//	/* Wait for last operation to be completed */
//  FLASH_Status status = FLASH_COMPLETE;
//	SerialPushStr("X-0", END_LF);
//  status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
//	if(status != FLASH_COMPLETE){
//		SerialPushStr("SET FLG ERROR!!! BUSY!!!", END_LF);
//		return;
//	}
//	SerialPushStr("X-1", END_LF);
//	disable_irqn();
//	FLASH_Unlock();	
//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
//	SerialPushStr("X-2", END_LF);
//	/* Erase page */
//	if(FLASH_ErasePage(ADDR) != FLASH_COMPLETE){
//		FLASH_Lock();
//		enable_irqn();
//		SerialPushStr("SET FLG ERROR!!! NOT COMPLETE!!!", END_LF);
//		return;
//	}
//	SerialPushStr("X-3", END_LF);
//	
//	/* Write data */
//	FLASH_ProgramWord(ADDR, state);

//	SerialPushStr("X-4", END_LF);
//	FLASH_Lock();
//	enable_irqn();
//	if(reset){
//		osDelay(1000);
//		NVIC_SystemReset();
//	}
//	SerialPushStr("X-5", END_LF);
//}

uint32_t FLASH_GetFLG(uint32_t addr){//set flg = 0 in app
	uint32_t Result = flash_read_uint32_t(addr);
	
	if(Result == 0xFF){
		Result = 0x00;
	}
	/* Return Application Program */
	return Result;
}

void firmware_thrd_init(void){
	fwupdate_thread = osThreadCreate(osThread(vFirmwareUpdateHandler), NULL);
  if(!fwupdate_thread) return;
}
