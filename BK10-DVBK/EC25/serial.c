#include <ql_oe.h>

#include "serial.h"
#include "gps.h"
#include "network.h"
#include "queue.h"
#include "setting.h"
#include "log.h"
#include "database.h"
#include "devconfig.h"

static int serial_uart = -1;
#define MAX_BUF_SERIAL_LEN   2048
#define MAX_REPORT_MSG       128
#define MAX_DEBUG_MSG        1024

typedef struct{
	u16 size;
	u8 data[MAX_REPORT_MSG];
}report_mail_queue_typedef;

typedef struct{
	u16 size;
	u8 data[MAX_DEBUG_MSG];
}debug_mail_queue_typedef;

Queue_t report_queue;
Queue_t debug_queue;

static void* UartSerialRecv_Proc(void* arg);
static pthread_t thrd_serial_uart_rcv;

void init_serial_queue(void){
	initialize(&report_queue, 1000);
	initialize(&debug_queue, 256);
}

static void UART_serial_init(void){
	int iRet;
	int baudRate = 115200;
	serial_uart = Ql_UART_Open(QL_UART_SERIAL_DEV, baudRate, FC_NONE);
	printf("< open(\"%s\", %d)=%d\n", QL_UART_SERIAL_DEV, baudRate, serial_uart);

	/* Start: If need, to modify uart dcb config */
	ST_UARTDCB dcb = {
		.flowctrl = FC_NONE,	//none flow control
		.databit = DB_CS8,	//databit: 8
		.stopbit = SB_1,	//stopbit: 1
		.parity = PB_NONE,	//parity check: none
		.baudrate = B_115200	//baudrate: 115200
	};

	iRet = Ql_UART_SetDCB(serial_uart, &dcb);
	printf("SET DCB ret: %d\n", iRet);

	if (pthread_create(&thrd_serial_uart_rcv, NULL, UartSerialRecv_Proc, NULL) != 0){
		log_system("Fail to create thread serial rcv !\n");
	}
}

void Report_handler(u8 *msg){
	u8 code;
	u8 day = ((msg[1] & 0x0F) * 10) + (msg[2] & 0x0F);

	if(strlen((char*)msg) < 7) return;

	/* Get code */
	code = (msg[0] & 0x0F);

	switch(code){
		case 0:
			/* Report basic data */
			report_basic_data();
			/* Report continuous driving time */
			report_driver_data(day, 1);
			/* Report stop/park */
			report_driver_data(day, 2);
			/* Report journey in day */
			report_trip_info(day);
			/* Report speed in day */
			report_speed(day);
			break;
		case 1:
			/* Report basic data */
			report_basic_data();
			break;
		case 2:
			/* Report continuous driving time */
			report_driver_data(day, 1);
			break;
		case 3:
			/* Report stop/park */
			report_driver_data(day, 2);
			break;
		case 4:
			/* Report journey in day */
			report_trip_info(day);
			break;
		case 5:
			/* Report speed in day */
			report_speed(day);
			break;
	}
}

void get_serial_data(u8* data, u16 len){
	u8  cmd_buff[128] = {0};
	u8  resp_buff[512] = {0};
	s32 ret = 0;
	eNumCMDCode cmd = 0;

    /* DEVICE SETTING */
    memset(cmd_buff, 0x0, sizeof(cmd_buff));
    sprintf(cmd_buff, "SET_");
    ret = strncmp(data, cmd_buff, strlen(cmd_buff));
    if(!ret){
        u8 *p1=NULL;
        p1 = data + strlen(cmd_buff);
        cmd = device_setting(p1, PC_SET);

		if (cmd != NOT_RESPONSE_CMD){
			sprintf(resp_buff, "%s\n", get_device_setting(cmd, PC_SET));
			Ql_UART_Write(serial_uart, resp_buff, strlen(resp_buff));
		}
        return;
    }

    /* GET DEVICE SETTING */
    memset(cmd_buff, 0x0, sizeof(cmd_buff));
    sprintf(cmd_buff, "READ_");
    ret = strncmp(data, cmd_buff, strlen(cmd_buff));
    if(!ret){
        u8 *p1=NULL;
        p1 = data + strlen(cmd_buff);

        cmd = (eNumCMDCode)(atoi(p1));
        sprintf(resp_buff, "%s\n", get_device_setting(cmd, PC_SET));
        Ql_UART_Write(serial_uart, resp_buff, strlen(resp_buff));
        return;
    }
    /* REPORT QC31VN */
    memset(cmd_buff, 0x0, sizeof(cmd_buff));
    sprintf(cmd_buff, "READ0");
    ret = strncmp(data, cmd_buff, strlen(cmd_buff));
    if(!ret){
        u8 *p1=NULL;
        p1 = data + strlen(cmd_buff);
        /* disable debug appliction */
        disable_debug_app();
        /* report to tacho meter */
        Report_handler(p1);
        return;
    }

	if(!strncmp(data, "SHOW_GSM", 8)){
		show_debug_app();
	}

	if(!strncmp(data, "EN_DEBUG", 8)){
		show_debug_app();
	}

	if(!strncmp(data, "LOGIN_", 6)){
		if(str_find(data, SystemSetting.data.PassWord) >= 0){
			DeviceSetting.com_enset = 1;
			log_system("DANG NHAP THANH CONG!!!\n");
		}
	}
	if(!strncmp(data, "LOGOUT", 6)){
		DeviceSetting.com_enset = 0;
		log_system("DANG XUAT THANH CONG!!!\n");
	}
}

void* UartSerialRecv_Proc(void* arg){
	int iRet;
	fd_set fdset;
	struct timeval timeout = {3, 0};	// timeout 3s
	u8 buffer[2048] = {0};
	while(serial_uart >= 0){
		FD_ZERO(&fdset);
		FD_SET(serial_uart, &fdset);
		iRet = select(serial_uart + 1, &fdset, NULL, NULL, &timeout);
		if(-1 == iRet){
			printf("< failed to select >\n");
			exit(-1);
		}else if(0 == iRet){// no data in Rx buffer
			timeout.tv_sec  = 3;
			timeout.tv_usec = 0;
		}else{// data is in Rx data
			if(FD_ISSET(serial_uart, &fdset)){
				do{
					memset(buffer, 0x0, sizeof(buffer));
					iRet = Ql_UART_Read(serial_uart, buffer, MAX_BUF_SERIAL_LEN);
					log_debug("> read(uart SERIAL)=%d:%s\n", iRet, buffer);
					get_serial_data(buffer , iRet);
				} while (MAX_BUF_SERIAL_LEN == iRet);
			}
		}
	}
}

void* thread_serial_handler(void* arg){
	UART_serial_init();
	for(;;){
		/* REPORT DRIVER DATA */
		while(!isEmpty(&report_queue)){
			report_mail_queue_typedef rpt;
			dequeue(&report_queue, &rpt, sizeof(report_mail_queue_typedef));
			rpt.data[rpt.size] = 0x0;
			Ql_UART_Write(serial_uart, rpt.data, rpt.size);
			usleep(4000);
		}
		/* DEBUG APPLICATION */
		while(!isEmpty(&debug_queue)){
			debug_mail_queue_typedef dbg;
			dequeue(&debug_queue, &dbg, sizeof(debug_mail_queue_typedef));
			dbg.data[dbg.size] = 0x0;
			Ql_UART_Write(serial_uart, dbg.data, dbg.size);
		}

		sleep(1);
	}
}

void DEBUG_APP__(u8* data, u16 size){
	debug_mail_queue_typedef dbg;
	if(size > (MAX_DEBUG_MSG - 1)){
		return;
	}

	dbg.size = size;
	memcpy(dbg.data, data, size);
	dbg.data[size] = 0x0;
	printf(dbg.data);

	if(!enqueue(&debug_queue, &dbg, sizeof(debug_mail_queue_typedef))){
		printf("Queue DEBUG full\n");
	}
}

void REPORT_DATA(char* msg){
	u16 size = strlen(msg);
	report_mail_queue_typedef rpt;
	if(size > (MAX_REPORT_MSG - 1)){
		return;
	}

	rpt.size = size;
	memcpy(rpt.data, msg, size);

	if(!enqueue(&report_queue, &rpt, sizeof(report_mail_queue_typedef))){
		printf("Queue Serial full\n");
	}
}
