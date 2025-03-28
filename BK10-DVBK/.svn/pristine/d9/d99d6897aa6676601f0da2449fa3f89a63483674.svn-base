/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef CAM_H
#define CAM_H

#include <stdio.h>
#include <stdarg.h>
#include <ql_oe.h>

#define QL_UART_RFID_DEV  "/dev/ttyHSL2"

void init_rfid_queue(void);

void* thread_rfid_handler(void* arg);

void rfid_send_msg(u8* data, u16 size);

void rfid_send_with_cs(u8* data, u16 size);

int rfid_send_and_rcv(u8 * src, u16 len, u8 * desc, u16* desc_size, u16 timeout);

extern u8 flg_sv_lg;

extern u8 sv_lg_buff[16];

#endif
