#ifndef __LCD_UTILS_H
#define __LCD_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f30x.h"
#include "cmsis_os.h"

void display_battery(u16 milivol);
void display_charge(uint16_t main_pw);
void display_server_connect(u8 connected);
void display_drive_continue_time_load(char * name, u16 time);
void display_signal_strength(enum_STR_DISP_type type, uint8_t x, uint8_t y, enum_STR_LEVEL lv);
void display_inout(u8 acc, u8 door, u8 air);
void display_base_info(void);
#endif /* __LCD_UTILS_H */
