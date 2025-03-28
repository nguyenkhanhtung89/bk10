
#ifndef __DRIVER_H
#define __DRIVER_H

#include <time.h>
#include "gps.h"
#include "database.h"

typedef enum{
	STATUS_UNKNOW = 0,
	STATUS_RUNNING = 1,
	STATUS_STOPPING = 2
} CAR_STATUS;

//typedef struct{
//	u8 flg_update;
//	u64 uid__;
//	u32 driver_inday;
//	u32 driver_inweek;
//	u32 park_count;
//	u8 ov_spd_count;
//	u8 drv_ctn_count;
//	u8 ov_drv_ctn_count;
//}drv_summary_def;

extern tibase_typedef tibase;

void clear_driver_result(u8 clear_day, u8 clear_week);

void calculate_drive_data(void);

int driver_login(u8 * info, u8 chk);

void driver_logout(void);

void SendDriverInfo(void);

void clear_driver_info(void);

u8 update_sync_status(u8 * info);

u32 get_run_time(void);

u32 get_stop_time(void);

void sync_tibase(void);

#endif /* __DRIVER_H */
