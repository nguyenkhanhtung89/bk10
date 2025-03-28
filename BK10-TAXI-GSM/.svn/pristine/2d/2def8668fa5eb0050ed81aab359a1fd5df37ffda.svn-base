/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <stdio.h>
#include <stdarg.h>
#include <ql_oe.h>
#include "devconfig.h"

#if (USE_ENCRYPT == 1)

	void encrypt_GPSdata(u8 *IMEI, u32 time, u8* IN, u8* OUT, int len);

	int encrypt_picture(char *InputName, char *OutPutName);

	void hex_log_debug(u8 *hex, u16 len);

#endif /* USE_ENCRYPT */

#endif /* ENCRYPT_H */
