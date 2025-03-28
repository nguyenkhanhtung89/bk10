#ifndef CAMERA_H
#define CAMERA_H

#include <stdio.h>
#include <stdarg.h>
#include <ql_oe.h>

typedef struct{
	u8 chanel;
	u32 sttime;
}image_mail_queue_typedef;

void* thread_camera_handler(void* arg);

void setting_number_plate(char * plate);

void setting_provider(char *provider);

void cam_setting_driver_info(char * name, char * license);

void init_cam(void);

u8 get_list_camera_sn(char* out);

void reboot_camera(void);

int change_ip_camera(u8 src, u8 des);

u8 get_list_camera_index(char *out);

void Qdelete_img(char *fName);

extern u8 pw_camera_status;

extern u16 count_warning_cam_inday;

#endif
