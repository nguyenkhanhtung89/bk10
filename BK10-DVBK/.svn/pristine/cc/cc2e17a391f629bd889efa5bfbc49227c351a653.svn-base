#ifndef AUDIO_H
#define AUDIO_H

#include <stdio.h>
#include <stdarg.h>
#include <ql_oe.h>

#define   AUDIO_PW      		PINNAME_SDC1_DATA2
#define   AUDIO_CS     			PINNAME_GPIO2
#define   AUDIO_VOL_CRTL      	PINNAME_GPIO1

#define   MAX_PLAYLIST  32

typedef struct{
	char filepath[128];
	char filename[64];
	u8 volume;
}audio_file_inf;

void* thread_audio_handler(void* arg);

void add_play_list(char *filepath, char *filename, u8 volume);

int set_volume(u8 vol);

void volume_down(u8 value);

void volume_up(u8 value);

#endif
