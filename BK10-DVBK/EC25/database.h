#ifndef DATABASE_H
#define DATABASE_H
#include <ql_oe.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "gps.h"

#define DATA_BASE_PATH          				"/usrdata/database.db"

#define	UNITS_PROVIDED					    	"CONG TY CP THIET BI DIEN - DIEN TU BACH KHOA"

#define	DEVICE_NAME							    "BK10"

#define	MAX_SIZE_DRIVER_NAME		   			44

#define	MAX_SIZE_DRIVER_LICENSE	       			16

#define	MAX_SIZE_TAG_DATE     	       			10

#define MAX_DRIVER_DATA_LEN 					80

#define DEFAULT_DRIVER_INFO            			"NULL"

#define DEFAULT_DRIVER_DATE            			"NULL"

typedef struct{
	u8 status_1;
	u8 status_2;
	u32 tm_stamp;
	struct tm *d_time;
	loc_def location;
	u16 angle;
	u8 pulse_speed;
	u8 speed_limit;
	u32 gps_distance;
	int16_t DriveInWeek;
	u16 fuel1;//rs232-1
	u16 fuel2;//rs232-2
	u16 fuel3;//adc or pulse
	u64 tagUID;
	int8_t GSMStreng;
	u16 batery_power;
	u16 main_power;
	u16 frqsend;
	u8 speed1;
	u8 speed2;
	u8 speed3;
	u8 speed4;
	u8 speed5;
	u8 speed6;
	u8 speed7;
	u8 speed8;
	u8 speed9;
	u8 speed10;//current speed
	u8 FlgSend;
}trip_data_typedef;

typedef enum{
	P_STATUS1 = 0,
	P_STATUS2 = 1,
	P_UTIME = 2,
	P_LAT = 3,
	P_LON	= 4,
	P_ANGLE = 5,
	P_PULSESPEED = 6,
	P_TRIPSPEEDLIMIT = 7,
	P_GPSDISTANCE = 8,
	P_PULSEDISTANCE = 9,
	P_FUEL1 = 10,
	P_FUEL2 = 11,
	P_FUEL3 = 12,
	P_TAGUID = 13,
	P_GSMSTRENG = 14,
	P_MAIN_PW = 15,
	P_BAT_PW = 16,
	P_FRQSEND = 17,
	P_SPEED1 = 18,
	P_SPEED2 = 19,
	P_SPEED3 = 20,
	P_SPEED4 = 21,
	P_SPEED5 = 22,
	P_SPEED6 = 23,
	P_SPEED7 = 24,
	P_SPEED8 = 25,
	P_SPEED9 = 26,
	P_SPEED10 = 27,
	P_FLGSEND = 28,
	P_TRIPDAY = 29,
	P_TRIPHOUR = 30
}enum_tripinfo_position;

typedef enum{
	STATUS_UNKNOWN = 0,
	STATUS_LOGOUT = 1,
	STATUS_LOGIN = 2
}enum_login_status;

typedef struct{
	u64 UID;
	u8 Status;
	u32 BTime;
	u32 ETime;
	loc_def begin_location;
	loc_def end_location;
	u32 Distance;
	u8 DrvDay;
	u8 FlgSend;
}driver_data_typedef;

typedef struct{
	char DriverName[MAX_SIZE_DRIVER_NAME + 10];
	char DriverLicense[MAX_SIZE_DRIVER_LICENSE + 10];
	char IssDate[MAX_SIZE_TAG_DATE + 10];
	char ExpDate[MAX_SIZE_TAG_DATE + 10];
	u32 drvinday;
	u32 drvinweek;
	u8 sync;
}driver_info_def;

typedef struct{
	enum_login_status LoginStatus;
	bool over_speed_status;
	bool over_driver_continue_status;
	bool over_driver_inday_status;
	bool over_driver_inweek_status;
	u64 UID;
	driver_info_def driver_info;
	loc_def b_loc_run;
	loc_def b_loc_stop;
	u32 bRunTime;
	u32 bStopTime;
	u32 RunSecondsPrvDay;
	u16 ParkCount;
	u16 OverSpeedCount;
	u8 DriveContinueCount;
	u8 OverDriveContinueCount;
	double GPSMeterInday;
	u32 DriverInday;
	u32 bMeter;
	u32 DriverInWeek;
	u8 flg_reload_dvr_data;
}tibase_typedef;

extern u8 db_wait___;

int init_database(void);

int report_trip_info(u8 day);

int report_driver_data(u8 day, u8 state);

int report_speed(u8 day);

void report_basic_data(void);

int insert_TripInfo(trip_data_typedef GPSData);

int inset_DriverData(driver_data_typedef DrvData);

int delete_database_ByDay(u8 day);

int delete_all_tripinfo(void);

int read_trip_info(trip_data_typedef* trip, time_t stamp);

int clear_flg_send(u32 stamp);

void* thread_backup(void* arg);

int set_backup_request(u8* btime, u8* etime);

int update_tibase(tibase_typedef tib);

void db_wait(void);

void db_post(void);

int load_driver_info_by_uid(u64 uid);

int update_driver_sync_by_uid(u64 uid, u8 sync);

int update_driver_info_by_uid(driver_info_def drv, u64 uid);

int load_driver_data_by_uid(u64 uid, u32 time__t);

int load_driver_summary_data_by_uid(u64 uid);

int clear_summary_data(u8 clear_day, u8 clear_week);

void upload_database(void);

#endif

