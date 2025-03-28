/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdarg.h>
#include "queue.h"

#define TCP_MAX_LENGTH 512
#define SERVER_MAX_LENGTH 256

#define MAX_SMS_PASSWORD_LEN 32

extern Queue_t queue_tcp;

extern E_QL_WWAN_NET_REG_STATE_T current_nw_mode;

extern char current_operator[128];

extern u16 MobileCountryCode;

#define VIETNAM_MCC  452

#define CHINA_MCC_1  454
#define CHINA_MCC_2  455
#define CHINA_MCC_3  460
#define CHINA_MCC_4  461

typedef struct{
	u16 size;
	u8 data[SERVER_MAX_LENGTH];
}sv_mail_queue_typedef;

typedef struct{
	u16 size;
	u8 data[TCP_MAX_LENGTH];
	u8 Data_Type;
	u32 trace;
}tcp_mail_queue_typedef;

typedef struct{
	u8 CommandCode;
//	u8 flag_send;
//	u8 isRecoverMode;
	char rep[QL_SMS_MAX_MT_MSG_LENGTH];
//	char PhoneNumber[QL_SMS_MAX_ADDR_LENGTH];
//	u8 SMSPassword[MAX_SMS_PASSWORD_LEN];
}sms_def;

typedef enum{
	NW_MODE_GSM = 0,
	NW_MODE_WCDMA = 1,
	NW_MODE_TDSCDMA = 2,
	NW_MODE_LTE = 3,
	NW_MODE_CDMA = 4,
	NW_MODE_HDR = 5,
}enum_radio_tech;

typedef enum{
	DATA_DELIVERED = 0,
	DATA_ERROR = 1,
	DATA_TIMEOUT = 2,
	DATA_SEND_ERROR = 3,
	DATA_UNKNOWN = 4,
	CERT_ACK_OK = 5,
	CERT_ACK_ERROR = 6,
}enumTCP_result;

typedef struct{
	enum_radio_tech radio_tech;
	int8_t  signal_strength;
	u8 network_name[128];
}NetworkInfo_typedef;

typedef struct{
	NetworkInfo_typedef net_work_info;
	E_QL_SIM_STATES_T sim_status;
	bool internet_status;
	bool tcp_status;
	bool cam_status;
	bool is_cam_warning;
	bool move_logout_warning;
}nw_data_def;

bool Vehicle_in_China(void);

extern nw_data_def network_data;

bool check_internet(void);

void init_queue_tcp(void);

void* thread_network(void* arg);

void Push_TCP_Data(tcp_mail_queue_typedef obj);

void tcp_reconnect(u8 time);

void set_device_reset_timeout(u8 time);

u8 get_device_reset_timeout(void);

void set_device_reset_new_day(u16 time);

int update_driver_info_by_server(u8 * cmd);

int Send_SMS(char* phone_number, char* content);

int check_ipaddress(char* ip);

int lan_port_init(void);

#endif
