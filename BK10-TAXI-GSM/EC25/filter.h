#ifndef FILTER_H
#define FILTER_H

#include <stdio.h>
#include <stdarg.h>
#include <ql_oe.h>

void* thread_filter_handler(void* arg);

void add_fuel_data(int ad1, int ad2);

extern bool flg_full_load;

extern u8 fuel_index;

extern bool kalman_startup;

#endif
