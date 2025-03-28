/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef MCU_H
#define MCU_H

#include <stdio.h>
#include <stdarg.h>

#define QL_UART_MCU_DEV  "/dev/ttyHS0"

typedef struct{
	u16 main_power;
	u8 OBD_connect;
	u16 ecu_rpm;
	u8 ecu_speed;
	s16 ecu_water_temp;
	u32 ecu_totalkm;
	u32 ecu_toal_fuel;
	u8 ecu_percent_fuel;
	u8 ecu_voltage;
	u8 ecu_engine_load;
}ECU_data_def;

typedef enum{
	DRIVER_SYNC = 0,
	DRIVER_LOGIN = 1,
	DRIVER_INFO_REQUEST = 2,
	DEVICE_SETTING_REQUEST = 3,
	MCU_DATA_REPORT = 4,
	FAULT_CODE_REPORT = 5,
	MCU_FW_REUPDATE = 6,
	MCU_FW_PACKAGE_DATA = 7,
	RFID_FW_REUPDATE = 8,
	RFID_FW_PACKAGE_DATA = 9,
	RFID_REPORT_ID = 10,
	DEV_SETTING_REPORT = 11,
	VIN_NUMBER_REPORT = 12
}enum_MCU_CMDCode;

typedef enum{
	GPS_DATA_MESSAGE = 0,
	CONFIG_MESSAGE = 1,
	DRIVER_MESSAGE = 2,
	WRITE_RFID_MESSAGE = 3
}ec21_message_def;

extern char dbg_mcu_ir[256];

extern bool mcu_connected;

void init_mcu_queue(void);

void* thread_mcu_handler(void* arg);

void mcu_send_msg(u8* data, u16 size);

void mcu_send_with_cs(u8* data, u16 size);

int mcu_send_and_rcv(u8 * src, u16 len, u8 * desc, u16* desc_size, u16 timeout);

#endif
