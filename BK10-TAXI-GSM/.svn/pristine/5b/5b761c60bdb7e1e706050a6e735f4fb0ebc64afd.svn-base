
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

extern tibase_typedef tibase;

void clear_driver_result(void);

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
