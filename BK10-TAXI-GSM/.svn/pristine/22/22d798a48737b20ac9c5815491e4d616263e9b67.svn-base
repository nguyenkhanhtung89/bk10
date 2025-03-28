/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <stdarg.h>

#define QL_UART_SERIAL_DEV  "/dev/ttyHSL1"   //chuan   ->   /dev/ttyHSL1

void init_serial_queue(void);

void* thread_serial_handler(void* arg);

void DEBUG_APP__(u8* data, u16 size);

void REPORT_DATA(char* msg);

#endif
