#include "rfid.h"

#include <ql_oe.h>
#include "gps.h"
#include "log.h"
#include "network.h"
#include "queue.h"
#include "setting.h"
#include "utils.h"
#include "ql_gpio.h"
#include "devconfig.h"

static int rfid_uart = -1;

#define MAX_BUF_RFID_LEN   2048
#define MAX_RFID_MSG       160

u8 flg_sv_lg = 0;
u8 sv_lg_buff[16];

typedef struct{
	u16 size;
	u8 data[MAX_RFID_MSG];
}rfid_mail_queue_typedef;

Queue_t rfid_tx_msg;
Queue_t rfid_rx_msg;

static void* UartRFIDRecv_Proc(void* arg);
static pthread_t thrd_rfid_uart_rcv;

void init_rfid_queue(void){
	initialize(&rfid_tx_msg, 20);
	initialize(&rfid_rx_msg, 20);
}

static void UART_RFID_init(void){
	int iRet;
	int baudRate = B_115200;
	rfid_uart = Ql_UART_Open(QL_UART_RFID_DEV, baudRate, FC_NONE);
	printf("< open(\"%s\", %d)=%d\n", QL_UART_RFID_DEV, baudRate, rfid_uart);

	/* Start: If need, to modify uart dcb config */
	ST_UARTDCB dcb = {
		.flowctrl = FC_NONE,	//none flow control
		.databit = DB_CS8,	//databit: 8
		.stopbit = SB_1,	//stopbit: 1
		.parity = PB_NONE,	//parity check: none
		.baudrate = B_115200	//baudrate: 115200
	};

	iRet = Ql_UART_SetDCB(rfid_uart, &dcb);
	printf("SET DCB ret: %d\n", iRet);

	if (pthread_create(&thrd_rfid_uart_rcv, NULL, UartRFIDRecv_Proc, NULL) != 0){
		log_system("Fail to create thread rfid rcv !\n");
	}
}

static void* UartRFIDRecv_Proc(void* arg){
	int iRet;
	fd_set fdset;
	struct timeval timeout = {30, 0};	// timeout 100ms

	u8 tmp[2048] = {0};
	while (rfid_uart >=0){
		FD_ZERO(&fdset);
		FD_SET(rfid_uart, &fdset);
		iRet = select(rfid_uart + 1, &fdset, NULL, NULL, &timeout);
		if (-1 == iRet){
			printf("< failed to select >\n");
			exit(-1);
		}else if (0 == iRet){// no data in Rx buffer
			timeout.tv_sec  = 0;
			timeout.tv_usec = 100000;
		}else{// data is in Rx data
			if (FD_ISSET(rfid_uart, &fdset)){
				do {
					memset(tmp, 0x0, sizeof(tmp));
					iRet = Ql_UART_Read(rfid_uart, tmp, 2048);
					log_fatal("> read(uart RFID)=%d:%s\n", iRet, tmp);

					if(tmp[0] == '>'){
						u8 cs = CRC_8BitsCompute(tmp, iRet - 1);
						if(cs == tmp[iRet - 1]){
							if(tmp[1] == '>'){// RESPONSE WHEN REQUEST
								if(tmp[2] == 'V'){
									MCU_Data.RFID_Verison = tmp[3];
								}
							}else{
								tmp[iRet - 1] = 0;
								get_mcu_data(tmp + 1 , iRet - 2);
							}
						}else{
							log_fatal("(UART RFID) CHECK SUM ERROR \r\n");
						}
					}else if((tmp[0] == '#') && (tmp[1] == '#')){
						rfid_send_msg("UART RFID READY\n", 11);
					}
				} while (2048 == iRet);
			}
		}
	}
}

u8 rfid_message[1024];
int rfid_send_and_rcv(u8 * src, u16 len, u8 * desc, u16* desc_size, u16 timeout){
	u16 cnt = 0;
	u8 cs = CRC_8BitsCompute(src, len);
	rfid_mail_queue_typedef rx;

	while(!isEmpty(&rfid_rx_msg)){// free queue
		dequeue(&rfid_rx_msg, &rx, sizeof(rfid_mail_queue_typedef));
		memset(&rx, 0x0, sizeof(rfid_mail_queue_typedef));
	}

	memcpy(rfid_message, src, len);
	rfid_message[len] = cs;

	Ql_UART_Write(rfid_uart, rfid_message, len + 1);
	while(cnt < timeout){
		if(!isEmpty(&rfid_rx_msg)){
			dequeue(&rfid_rx_msg, &rx, sizeof(rfid_mail_queue_typedef));
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

void rfid_send_msg(u8* data, u16 size){
	rfid_mail_queue_typedef tx;
	if(size > MAX_RFID_MSG - 1){
		return;
	}

	tx.size = size;
	memcpy(tx.data, data, size);

	if(!enqueue(&rfid_tx_msg, &tx, sizeof(rfid_mail_queue_typedef))){
		printf("Queue rfid TX full\n");
	}
}

void rfid_send_with_cs(u8* data, u16 size){
	u8 cs = 0;
	rfid_mail_queue_typedef tx;
	if(size > (MAX_RFID_MSG - 2)){
		return;
	}
	cs = CRC_8BitsCompute(data, size);

	memset(tx.data, 0x0, MAX_RFID_MSG);
	memcpy(tx.data, data, size);
	tx.data[size] = cs;
	tx.size = size + 1;

	if(!enqueue(&rfid_tx_msg, &tx, sizeof(rfid_mail_queue_typedef))){
		printf("Queue rfid TX full\n");
	}
}

void* thread_rfid_handler(void* arg){
	u32 request_time = 0;
	/* init uart */
	UART_RFID_init();

	for(;;){
		while(!isEmpty(&rfid_tx_msg)){
			rfid_mail_queue_typedef tx;
			dequeue(&rfid_tx_msg, &tx, sizeof(rfid_mail_queue_typedef));

			//printf(buf.data);
			Ql_UART_Write(rfid_uart, tx.data, tx.size);
			usleep(500000);
		}
		request_time++;
		if((request_time < 120) && ((request_time % 5) == 0)){
			if(MCU_Data.RFID_Verison == 0){
				/* check rfid version */
				rfid_send_with_cs(">VERSION?", 9);
			}else if(MCU_Data.ID_RFID == 0){
				/* check rfid id */
				read_rfid_id();
			}
		}

		if(flg_sv_lg == 1){
			flg_sv_lg = 0;
			driver_login(sv_lg_buff, 0);
		}else if(flg_sv_lg == 2){
			flg_sv_lg = 0;
			driver_logout();
		}

		sleep(1);
	}
}

