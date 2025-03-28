/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef ATC_H
#define ATC_H

#include <stdio.h>
#include <stdarg.h>
#include <ql_oe.h>

void config_sim_detect(void);

int config_tlv_codec(void);

u8 audio_ready(void);

#endif
