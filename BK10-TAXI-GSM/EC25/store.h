/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef STORE_H
#define STORE_H

#include <stdio.h>
#include <stdarg.h>
#include <ql_oe.h>

int read_data(char* full_path, u8* desc, u16* rlen);

int write_data(char* full_path, u8* data, u16 len);

int write_string(char* full_path, char* data);

int read_string(char* full_path, char * desc);

void write_number(char* full_path, u64 num);

u64 read_number(char* full_path);

int read_float(char* full_path, float* desc);

void write_float(char* full_path, float val);

#endif
