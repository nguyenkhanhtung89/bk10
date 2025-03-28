#include <ql_oe.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "gps.h"
#include "network.h"
#include "mcu.h"
#include "serial.h"
#include "vibrate.h"
#include "database.h"
#include "log.h"
#include "utils.h"
#include "devconfig.h"
#include "audio.h"
#include "rfid.h"
#include "camera.h"
#include "filter.h"

pthread_t gps_thrd_id;
pthread_t nw_thrd_id;
pthread_t backup_thrd_id;
pthread_t serial_prc_thrd_id;
pthread_t mcu_prc_thrd_id;
pthread_t vibrate_thrd_id;
pthread_t area_thrd_id;
pthread_t audio_thrd_id;
pthread_t rfid_prc_thrd_id;
pthread_t camera_thrd_id;
pthread_t filter_thrd_id;
pthread_t init_thread_id;
#define   DEVICE_POWER_UP      			PINNAME_GPIO5

void* init_thread(void* arg){
	pthread_attr_t attr_normal;

	pthread_attr_init(&attr_normal);

	pthread_attr_setstacksize(&attr_normal, 1536*1024);

	/************************ Create thread gps*******************************************/
	if(pthread_create(&gps_thrd_id, &attr_normal, thread_gps, NULL) != 0){
		log_system("Create thread gps error!\n");
		exit(1);
	}

	while(!GPS_READY){
		log_system("THREAD GPS NOT READY -> WAIT...\n");
		sleep(2);
	}
	log_system("THREAD GPS  READY -> RUNNING.....\n");

	/************************ Create thread network*******************************************/
	if(pthread_create(&nw_thrd_id, &attr_normal, thread_network, NULL) != 0){
		log_system("Create thread network error!\n");
		exit(1);
	}

	/************************ Create thread backup data*******************************************/
	if(pthread_create(&backup_thrd_id, &attr_normal, thread_backup, NULL) != 0){
		log_system("Create thread backup error!\n");
		exit(1);
	}

	/************************ Create thread serial handler ***********************************/
	if(pthread_create(&serial_prc_thrd_id, &attr_normal, thread_serial_handler, NULL) != 0){
		log_system("Create thread serial handler error!\n");
		exit(1);
	}

	/************************ Create thread mcu handler ***********************************/
	if(pthread_create(&mcu_prc_thrd_id, &attr_normal, thread_mcu_handler, NULL) != 0){
		log_system("Create thread mcu handler error!\n");
		exit(1);
	}

	/************************ Create thread vibrate ***********************************/
//	if(pthread_create(&vibrate_thrd_id, &attr_normal, thread_vibrate, NULL) != 0){
//		log_trace("Create thread vibrate error!\n");
//		exit(1);
//	}

//	if(pthread_create(&audio_thrd_id, &attr_normal, thread_audio_handler, NULL) != 0){
//		log_system("Create thread audio handler error!\n");
//		exit(1);
//	}

	/************************ Create thread rfid ***********************************/
	if(pthread_create(&rfid_prc_thrd_id, &attr_normal, thread_rfid_handler, NULL) != 0){
		log_system("Create thread rfid handler error!\n");
		exit(1);
	}

	/************************ Create thread camera ***********************************/
//	if(pthread_create(&camera_thrd_id, &attr_normal, thread_camera_handler, NULL) != 0){
//		log_system("Create thread camera handler error!\n");
//		exit(1);
//	}

	/************************ Create thread fuel filter ***********************************/
//	if(pthread_create(&filter_thrd_id, &attr_normal, thread_filter_handler, NULL) != 0){
//		log_system("Create thread filter handler error!\n");
//		exit(1);
//	}

    Ql_GPIO_Init(DEVICE_POWER_UP, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_DISABLE);
    Ql_GPIO_SetLevel(DEVICE_POWER_UP, PINLEVEL_HIGH);

	for(;;){
		//	log_system("SIZE SETTING %d", sizeof(sys_setting));
		if(MCU_Data.obd_data.connected == 1){
			Ql_GPIO_SetLevel(DEVICE_POWER_UP, PINLEVEL_LOW);
			usleep(400000);
			Ql_GPIO_SetLevel(DEVICE_POWER_UP, PINLEVEL_HIGH);
			usleep(400000);
			Ql_GPIO_SetLevel(DEVICE_POWER_UP, PINLEVEL_LOW);
			usleep(400000);
			Ql_GPIO_SetLevel(DEVICE_POWER_UP, PINLEVEL_HIGH);
			usleep(400000);
			Ql_GPIO_SetLevel(DEVICE_POWER_UP, PINLEVEL_LOW);
			usleep(400000);
			Ql_GPIO_SetLevel(DEVICE_POWER_UP, PINLEVEL_HIGH);
			usleep(400000);
		}else{
			Ql_GPIO_SetLevel(DEVICE_POWER_UP, PINLEVEL_LOW);
			usleep(100000);
			Ql_GPIO_SetLevel(DEVICE_POWER_UP, PINLEVEL_HIGH);
			sleep(5);
		}
	}
}

int main(void){
	//show_debug_app();
	/* queue init */
	init_mcu_queue();
	init_serial_queue();
	init_rfid_queue();
	init_queue_tcp();

	/* load all device setting */
	load_device_setting();

	/* load module imei */
	memset(DeviceSetting.IMEI, 0x0, MAX_SIZE_IMEI);
	QL_DEV_GetImei(DeviceSetting.IMEI, MAX_SIZE_IMEI);
	log_system("%s\n", DeviceSetting.IMEI);

	/* database init */
	init_database();

	log_system("DINH VI BACH KHOA Linux Embedded \n");
	log_system("Main process: PID: %d, TID: %lu\n\n", getpid(), pthread_self());

	if(pthread_create(&init_thread_id, NULL, init_thread, NULL) != 0){
		log_system("Create thread base init error!\n");
		exit(1);
	}
	pthread_join(init_thread_id, NULL);

	return 0;
}

