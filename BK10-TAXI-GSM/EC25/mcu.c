#include <ql_oe.h>

#include "mcu.h"
#include "gps.h"
#include "network.h"
#include "queue.h"
#include "setting.h"
#include "log.h"
#include "database.h"
#include "driver.h"
#include "fw.h"
#include "devconfig.h"

static int mcu_uart = -1;
#define MAX_BUF_MCU_LEN   2048
#define MAX_MCU_MSG       1024

typedef struct{
	u16 size;
	u8 data[MAX_MCU_MSG];
}mcu_mail_queue_typedef;

Queue_t mcu_tx_msg;
Queue_t mcu_rx_msg;

static void* UartMCURecv_Proc(void* arg);
static pthread_t thrd_mcu_uart_rcv;
bool mcu_connected = FALSE;

u8 mcu_msg_timeout = 0;

void init_mcu_queue(void){
	initialize(&mcu_tx_msg, 50);
	initialize(&mcu_rx_msg, 50);
}

static void UART_MCU_init(void){
	int baudRate = 230400;
	mcu_uart = Ql_UART_Open(QL_UART_MCU_DEV, baudRate, FC_NONE);
	/* Start: If need, to modify uart dcb config */
	ST_UARTDCB dcb = {
		.flowctrl = FC_NONE,	//  none flow control
		.databit = DB_CS8,	    //  databit: 8
		.stopbit = SB_1,	    //  stopbit: 1
		.parity = PB_NONE,	    //  parity check: none
		.baudrate = B_230400	//  baudrate: 230400
	};

	Ql_UART_SetDCB(mcu_uart, &dcb);

	if (pthread_create(&thrd_mcu_uart_rcv, NULL, UartMCURecv_Proc, NULL) != 0){
		log_system("Fail to create thread mcu rcv !\n");
	}
}

system_setting_typedef tmp_sys_setting;

u8 fw_msg[FW_PKG_SZ + 64];
u8 fw_tmp[FW_PKG_SZ + 64];
void get_mcu_data(u8* data, u16 len){
	u8  cmd_buff[128] = {0};
	u8  resp_buff[256] = {0};
	s32 ret = 0;
	enum_MCU_CMDCode cmd = (enum_MCU_CMDCode)data[0];

	switch(cmd){
		case DRIVER_SYNC:
			update_sync_status(data + 1);
			break;
		case DRIVER_LOGIN:
			driver_login(data + 1, 1);
			break;
		case DRIVER_INFO_REQUEST: //request Driver info from mcu
			log_fatal("DRIVER INFO UPDATE 4\n");
			update_rfid_driver_data();
			break;
		case DEVICE_SETTING_REQUEST: //request Device info from mcu
			break;
		case MCU_DATA_REPORT:
			mcu_report_data_parser(data + 1);
			log_system("MCU REPORT DATA\n");
			break;
		case FAULT_CODE_REPORT:
			break;
		case MCU_FW_REUPDATE: //reupdate
			break;
		case MCU_FW_PACKAGE_DATA:{ //get package data
			u16 pkg_idx = 0;
			u16 pkg_len = 0;
			u32 crc32 = 0;
			u8 crc8 = 0;
			pkg_idx = (data[1] << 8) | data[2];
			if(load_firmware_pkg(pkg_idx, fw_tmp, &pkg_len) == 0){
				fw_msg[0] = '>';
				fw_msg[1] = 0xFD;

				fw_msg[2] = pkg_idx >> 8;
				fw_msg[3] = pkg_idx & 0xFF;

				fw_msg[4] = pkg_len >> 8;
				fw_msg[5] = pkg_len & 0xFF;

				crc32 = CRC32Software(fw_tmp, pkg_len);
				fw_msg[6] = (crc32 >> 24) & 0xFF;
				fw_msg[7] = (crc32 >> 16) & 0xFF;
				fw_msg[8] = (crc32 >> 8) & 0xFF;
				fw_msg[9] =	(crc32 & 0xFF);

				memcpy(fw_msg + 10, fw_tmp, pkg_len);

				crc8 = CRC_8BitsCompute(fw_msg, pkg_len + 10);

				fw_msg[10 + pkg_len] = crc8;

				Ql_UART_Write(mcu_uart, fw_msg, (10 + pkg_len + 1));

				log_fatal("LOAD FIRM WARE PKG OK -> PKG ID = %d\n", pkg_idx);
			}else{
				log_fatal("LOAD FIRM WARE PKG ERROR\n");
			}

			break;
		}
		case RFID_FW_REUPDATE: //reupdate
			break;
		case RFID_FW_PACKAGE_DATA: //get package data
			break;
		case RFID_REPORT_ID:
			MCU_Data.ID_RFID = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4] ;
			log_system("ID CUA DAU DOC THE LA %u\n", MCU_Data.ID_RFID);
			break;
		case DEV_SETTING_REPORT:{//11
			u32 crc32_cal = 0;
			u32 crc32_data = 0;
			u16 size_setting = sizeof(sys_setting);

			if(size_setting == (len - 5)){
				crc32_cal = CRC32Software(data+1, sizeof(sys_setting));
				crc32_data = (data[1 + size_setting] << 24) | (data[1 + size_setting + 1] << 16) | (data[1 + size_setting + 2] << 8) | data[1 + size_setting + 3];

				if(crc32_cal == crc32_data){
					memcpy(tmp_sys_setting.bytes, data+1, sizeof(sys_setting));
					if(tmp_sys_setting.data.DeviceID != 0){
						SystemSetting = tmp_sys_setting;
						log_system("RESTORE ALL DEVICE SETTING\n");
						sync_dev_setting();
					}else{
						log_system("NO MORE UPDATE DEVICE SETTING\n");
					}
				}else{
					log_system("CRC 32 FAILED NO UPDATE DEVICE SETTING %u-%u\n", crc32_cal, crc32_data);
				}
			}else{
				log_system("SIZE SETTING != LEN %d, %d\n", size_setting, (len - 4));
			}
			break;
		}
		case VIN_NUMBER_REPORT://12
			memcpy(MCU_Data.VIN_NUMBER, data + 1, 17);
			break;
	}
}
char dbg_mcu_ir[256];
void* UartMCURecv_Proc(void* arg){
	int iRet;
	fd_set fdset;
	struct timeval timeout = {3, 0};	// timeout 3s
	u8 buffer[2048] = {0};
	mcu_mail_queue_typedef rx;
	while(mcu_uart >= 0){
		FD_ZERO(&fdset);
		FD_SET(mcu_uart, &fdset);
		iRet = select(mcu_uart + 1, &fdset, NULL, NULL, &timeout);
		if(-1 == iRet){
			//printf("< failed to select >\n");
			exit(-1);
		}else if(0 == iRet){// no data in Rx buffer
			timeout.tv_sec  = 0;
			timeout.tv_usec = 100000;
		}else{// data is in Rx data
			if(FD_ISSET(mcu_uart, &fdset)){
				do{
					memset(buffer, 0x0, sizeof(buffer));
					iRet = Ql_UART_Read(mcu_uart, buffer, MAX_BUF_MCU_LEN);
					log_fatal("> read(uart MCU)=%d:%s\n", iRet, buffer);
					if(buffer[0] == '>'){
						u8 cs = CRC_8BitsCompute(buffer, iRet - 1);
						if(cs == buffer[iRet - 1]){
							//log_system("CHECK SUM OK data -> %s, tm %d\r\n", buffer, mcu_msg_timeout);
							if(buffer[1] == '>'){// RESPONSE WHEN REQUEST
								if(iRet <= MAX_MCU_MSG){
									memcpy(rx.data, buffer + 2, iRet - 3);
									rx.size = iRet - 3;

									//log_system("DATA -> %s\n", rx.data);

									if(!enqueue(&mcu_rx_msg, &rx, sizeof(mcu_mail_queue_typedef))){
										log_system("Queue MCU RX full\n");
									}
								}
							}else{
								buffer[iRet - 1] = 0;
								get_mcu_data(buffer + 1 , iRet - 2);
							}
							mcu_connected = true;
							mcu_msg_timeout = 0;
						}else{
							log_system("(UART MCU)CHECK SUM ERROR \r\n");
						}
					}else if((buffer[0] == 'c') && (buffer[1] == 'o') && (buffer[2] == 'u') && (buffer[3] == 'n') && (buffer[4] == 't')){
						memset(dbg_mcu_ir, 0x0, 256);
						memcpy(dbg_mcu_ir, buffer, iRet);
					}
				} while (MAX_BUF_MCU_LEN == iRet);
			}
		}
	}
}

u8 mcu_message[1024];
int mcu_send_and_rcv(u8 * src, u16 len, u8 * desc, u16* desc_size, u16 timeout){
	u16 cnt = 0;
	u8 cs = CRC_8BitsCompute(src, len);
	mcu_mail_queue_typedef rx;

	while(!isEmpty(&mcu_rx_msg)){// free queue
		dequeue(&mcu_rx_msg, &rx, sizeof(mcu_mail_queue_typedef));
		memset(&rx, 0x0, sizeof(mcu_mail_queue_typedef));
	}

	memcpy(mcu_message, src, len);
	mcu_message[len] = cs;

	Ql_UART_Write(mcu_uart, mcu_message, len + 1);
	while(cnt < timeout){
		if(!isEmpty(&mcu_rx_msg)){
			dequeue(&mcu_rx_msg, &rx, sizeof(mcu_mail_queue_typedef));
			memcpy(desc, rx.data, rx.size);
			desc[rx.size] = 0x0;
			*desc_size =  rx.size;
			return 0;
		}
		cnt ++;
		usleep(1000);
	}
	return -1;
}

void mcu_send_msg(u8* data, u16 size){
	mcu_mail_queue_typedef tx;
	if(size > (MAX_MCU_MSG - 1)){
		return;
	}

	tx.size = size;
	memcpy(tx.data, data, size);

	if(!enqueue(&mcu_tx_msg, &tx, sizeof(mcu_mail_queue_typedef))){
		log_fatal("Queue MCU TX full\n");
	}
}

void mcu_send_with_cs(u8* data, u16 size){
	u8 cs = 0;
	mcu_mail_queue_typedef tx;
	if(size > (MAX_MCU_MSG - 2)){
		return;
	}
	cs = CRC_8BitsCompute(data, size);

	memset(tx.data, 0x0, MAX_MCU_MSG);
	memcpy(tx.data, data, size);
	tx.data[size] = cs;
	tx.size = size + 1;

	if(!enqueue(&mcu_tx_msg, &tx, sizeof(mcu_mail_queue_typedef))){
		log_fatal("Queue MCU TX full\n");
	}
}

void* thread_mcu_handler(void* arg){
	UART_MCU_init();
	for(;;){
		while(!isEmpty(&mcu_tx_msg)){
			mcu_mail_queue_typedef tx;
			dequeue(&mcu_tx_msg, &tx, sizeof(mcu_mail_queue_typedef));

			//log_fatal(buf.data);
			Ql_UART_Write(mcu_uart, tx.data, tx.size);
			usleep(500000);
		}

		mcu_msg_timeout++;
		if(mcu_msg_timeout >= 30){
			log_system("MCU MESSAGE TIME OUT..........\n");
			mcu_connected = FALSE;
			mcu_msg_timeout = 0;
			memset(&MCU_Data, 0x0, sizeof(mcu_type_def));
		}
		sleep(1);
	}
}
