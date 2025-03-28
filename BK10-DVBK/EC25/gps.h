#ifndef GPS_H
#define GPS_H

#include <stdio.h>
#include <stdarg.h>
#include "vibrate.h"

#define GMT_HN             	 	25200

#define TIMESTAMP_2025   		1735689600

#define FREQ_RUN 				10

#define FREQ_WRITE_GPS_RUN 		10

#define FREQ_WRITE_GPS_STOP 	120

#define FREQ_WRITE_LOSS_GPS 	60

#define FREQ_STOP 			600

#define TIME_30_SECOND 		30

#define TIME_1_MINUTES 		60

#define TIME_2_MINUTES 		120

#define TIME_3_MINUTES 		180

#define TIME_4_MINUTES 		240

#define TIME_5_MINUTES 		300

#define TIME_10_MINUTES 	600

#define TIME_15_MINUTES 	900

#define TIME_1_HOUR 		3600

#define TIME_2_HOUR 		7200

#define TIME_4_HOUR 		14400

#define TIME_10_HOUR 		36000

#define TIME_48_HOUR 		172800

#define TIME_1_DAY 			86400

#define TIME_3_DAY 			259200

u8 get_acc(void);

#define ACC_STATUS          get_acc()

#define LOCATON_BITMASK     LOC_IND_LOCATION_INFO_ON | LOC_IND_STATUS_INFO_ON | LOC_IND_SV_INFO_ON /*| LOC_IND_NMEA_INFO_ON*/ | LOC_IND_CAP_INFO_ON | LOC_IND_UTC_TIME_REQ_ON | LOC_IND_XTRA_DATA_REQ_ON | LOC_IND_AGPS_DATA_CONN_CMD_REQ_ON | LOC_IND_NI_NFY_USER_RESP_REQ_ON

#define GPS_PER_SECOND    10

#define TAG_UID_SIZE    	8

typedef enum{
	REQUEST_CERT_KEY = 0,

	REAL_TIME_DATA = 1,

	DRIVER_INFO	= 2,

	BACKUP_GPS_DATA = 3,

	XXXXXX = 5,

	XXXXXXX = 6,

	DRIVER_IN_OUT = 7,

	OVER_SPEED = 8,

	XXXXXXXX = 9,

	XXXXXXXXX = 10,

	xxxxxxxxxx = 11,

	DEVICE_SETTING = 13,

	SMS_CONFIG_DEV = 14,
} TYPE_DATA;

typedef enum{
	POWER_BALANCE,
	POWER_SAVE
}power_type;

typedef struct{
	double latitude;
	double longitude;
}loc_def;

typedef struct{
	int64_t  StartTime;
	loc_def  StartLocation;
	uint16_t speeds;
	uint8_t  ovSpeedHold;
}ovSpeedDef;

typedef struct{
    float       bearing;                /**<   Heading in degrees. */
    int64_t     timestamp;              /**<   Timestamp for the location fix in UTC million-second base.  */
    int64_t     fact_time;
    float       gps_speed;                  /**<   Speed in meters per second. */
    float       tmp_speed;
    float       raw_speed;                  /**<   Speed in meters per second. */
    u8 pulse_speed;
    u8 speeds[GPS_PER_SECOND+1];
    loc_def location;
    loc_def prv_location;
    loc_def locations[GPS_PER_SECOND];
	u8 gps_status;
	u16 freq_send;
	u16 freq_send_camera;
	bool flg_capture;
	bool flg_update_cam;
	bool flg_scan_cam;
	bool flg_reboot_sgmi;
	u32 gps_time_out;
	u8  acc_time;
	u16 stop_time;
	struct tm *gps_time;
	struct tm *sys_time;
	int week_of_year;
	ovSpeedDef ovSpeedData;
	u8 flg_send_cfg;
	u8 time_to_send_evt;
	u8 flg_new_day;
	u8 flg_new_week;
	u32 cam_up_time;
	u32 tm_up;
	u8 svs_in_view;
	u8 svs_in_use;
	u16 no_acc_time;
}GPS_typedef;

typedef struct{
	u8 connected;
	u8 typeOBD;
	u8 sp;						/* Speed */
	u16 rpm;
	int temp;					/* Temperature */
	u16 p_fuel;					/* Percent fuel */
	u32 s_km;					/* Km */
	u32 s_fuel;					/* Lit */
	u16 econ_fuel;				/* Km/Lit */
	u16 avg_econ_fuel;			/* Km/Lit */
	u8 p_load;					/* Percent power */
	u32 bat;					/* mV */
} ODBDef;

typedef struct{
	u8 MCU_Verison;
	u8 RFID_Verison;
	u32 ID_RFID;
	u8 last_Hard_Acc;
	u8 Hard_Acc;
	u8 Last_Hard_Door;
	u8 Hard_Door;
	u8 Hard_Air;
	power_type power_mode;
	power_type last_power_mode;
	u16 main_power;
	u16 bat_power;
	u16 Vout_power;
	u16 fuel_232_1;
	u16 fuel_232_2;
	u16 fuel_adc_in;
	ODBDef obd_data;
	u8 pwr_warning_delay_tm;
}mcu_type_def;

extern GPS_typedef GPS_Data;

extern mcu_type_def MCU_Data;

extern u8 GPS_READY;

extern uint8_t speed_limit_device;

extern u8 transfer_setting_mcu;

double calculate_distance(loc_def loc1, loc_def loc2, char unit);

void* thread_gps(void* arg);

u8 gps_ready(void);

void send_realtime_data(void);

void update_rfid_driver_data(void);

void update_rfid_driver_data(void);

void mcu_report_data_parser(u8 * data);

void send_device_setting(void);

void clear_camera_cnt(void);

u16 get_frq_reset(void);

void increase_frq_reset(void);

void clear_time_reset_increment(void);

#endif
