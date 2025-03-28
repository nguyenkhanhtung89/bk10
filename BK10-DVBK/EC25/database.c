#include <ql_oe.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utils.h>
#include <semaphore.h>
#include <sqlite3.h>

#include "database.h"
#include "log.h"
#include "utils.h"
#include "mcu.h"
#include "driver.h"
#include "network.h"
#include "devconfig.h"
#include "queue.h"
#include "camera.h"

sqlite3 *database;
sqlite3 *dbtrace;

char *zErrMsg = 0;
const char* data = "Callback function called";
static int callback(void *NotUsed, int argc, char **argv, char **azColName);

trip_data_typedef Trip_Info;
static u8 flg_trip_data = 0;
static char sql_cmd[2048];

extern int errno;

static sem_t db_sem;

void db_wait(void){
	sem_wait(&db_sem);
}

void db_post(void){
	sem_post(&db_sem);
}

int open_database(void){
	int rc;
	char *sql;
	if(access(DATA_BASE_PATH, F_OK) < 0){/* file not Found */
		rc = sqlite3_open(DATA_BASE_PATH, &database);
		if(rc){
			log_trace("Can't open database: \n");
			return -1;
		}else{
		   sql = "CREATE TABLE DrivingData ( \
					UID	INTEGER NOT NULL, \
					Status	INTEGER NOT NULL, \
					BTime	INTEGER NOT NULL, \
					ETime	INTEGER NOT NULL, \
					BLat	REAL NOT NULL, \
					BLon	REAL NOT NULL, \
					ELat	REAL NOT NULL, \
					ELon	REAL NOT NULL, \
					Distance	INTEGER NOT NULL, \
				    DrvDay	INTEGER NOT NULL, \
					FlgSend	INTEGER NOT NULL)";
		   sqlite3_exec(database, sql, callback, (void*)data, &zErrMsg);

		   sql =  "CREATE UNIQUE INDEX drv_time_idx ON DrivingData(BTime, ETime)";
		   sqlite3_exec(database, sql, callback, (void*)data, &zErrMsg);

		   sql = "CREATE TABLE DriverInfoData ( \
					UID	INTEGER NOT NULL, \
					DriverName TEXT NOT NULL, \
					DriverLicense TEXT NOT NULL, \
					IssDate TEXT, \
				   	ExpDate TEXT, \
					sync INTEGER)";
		   sqlite3_exec(database, sql, callback, (void*)data, &zErrMsg);

		   sql =  "CREATE UNIQUE INDEX tag_uid_idx ON DriverInfoData(UID)";
		   sqlite3_exec(database, sql, callback, (void*)data, &zErrMsg);

		   sql = "CREATE TABLE TripInfo ( \
				status1 INTEGER NOT NULL, \
				Status2 INTEGER NOT NULL, \
				uTime INTEGER NOT NULL, \
				Lat REAL NOT NULL, \
				Lon REAL NOT NULL, \
				Angle INTEGER NOT NULL, \
				PulseSpeed INTEGER NOT NULL, \
				SpeedLimit INTEGER NOT NULL, \
				GPSDistance INTEGER NOT NULL, \
				PulseDistance INTEGER NOT NULL, \
				Fuel1 INTEGER NOT NULL, \
				Fuel2 INTEGER NOT NULL, \
				Fuel3 INTEGER NOT NULL, \
				TagUID INTEGER NOT NULL, \
				GSMStreng INTEGER NOT NULL, \
				MainPower INTEGER NOT NULL, \
				BatteryPower INTEGER NOT NULL, \
				FrqSend INTEGER NOT NULL, \
				Speed1 INTEGER NOT NULL, \
				Speed2 INTEGER NOT NULL, \
				Speed3 INTEGER NOT NULL, \
				Speed4 INTEGER NOT NULL, \
				Speed5 INTEGER NOT NULL, \
				Speed6 INTEGER NOT NULL, \
				Speed7 INTEGER NOT NULL, \
				Speed8 INTEGER NOT NULL, \
				Speed9 INTEGER NOT NULL, \
				Speed10 INTEGER NOT NULL, \
				FlgSend INTEGER NOT NULL, \
				TripDay INTEGER NOT NULL, \
				TripHour INTEGER NOT NULL)";
		   sqlite3_exec(database, sql, callback, (void*)data, &zErrMsg);

		   sql =  "CREATE UNIQUE INDEX utime_idx ON TripInfo(uTime)";
		   sqlite3_exec(database, sql, callback, (void*)data, &zErrMsg);

		   sql = "CREATE TABLE TiBase ( \
				TiID INTEGER NOT NULL, \
				LoginStatus	INTEGER, \
				UID	INTEGER, \
				bLatRun	REAL, \
				bLonRun	REAL, \
				bLatStop REAL, \
				bLonStop REAL, \
				bRunTime	INTEGER, \
				bStopTime	INTEGER, \
				RunSecondsPrvDay	INTEGER, \
				ParkCount	INTEGER, \
				OverSpeedCount	INTEGER, \
				DriveContinueCount	INTEGER, \
				OverDriveContinueCount	INTEGER, \
				GPSMeter	INTEGER, \
				PulseMeter	INTEGER, \
				DriverInday	INTEGER, \
				bMeter	INTEGER, \
				PRIMARY KEY(TiID))";
		   sqlite3_exec(database, sql, callback, (void*)data, &zErrMsg);

		   sql =  "CREATE UNIQUE INDEX tibase_idx ON TiBase(TiID)";
		   sqlite3_exec(database, sql, callback, (void*)data, &zErrMsg);

		   /* disconnect database */
		   sqlite3_close(database);
		}
	}

	rc = sqlite3_open(DATA_BASE_PATH, &database);
	if(rc){
	  log_trace("Can't open database: \n");
	  sleep(3);
	  ec21_reboot();
	  return -1;
	}

	if(rc != SQLITE_OK){
	  log_trace("SQL error: %s\n", zErrMsg);
	  sqlite3_free(zErrMsg);
	  sleep(3);
	  ec21_reboot();
	  return -1;
	}

	return 0;
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   log_trace("\n CALL BACK \n");
   for(i = 0; i<argc; i++){
	   log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   log_trace("\n");
   return 0;
}

static int callback_load_tibase(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	log_trace("\n LOAD TIBASE \n");
	for(i = 0; i<argc; i++) {
		log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	log_trace("\n");

	if(argc < 18){
		log_trace(" ERROR DATABASE \r\n ");
		return -1;
	}

	tibase.LoginStatus = (enum_login_status)ec21_atoi(argv[1]);
	tibase.UID = ec21_atoi(argv[2]);
	log_trace("UID STRING = %s \n", argv[2]);
	tibase.b_loc_run.latitude = atof(argv[3]);
	tibase.b_loc_run.longitude = atof(argv[4]);
	tibase.b_loc_stop.latitude = atof(argv[5]);
	tibase.b_loc_stop.longitude = atof(argv[6]);

	tibase.bRunTime = ec21_atoi(argv[7]);
	tibase.bStopTime = ec21_atoi(argv[8]);
	tibase.RunSecondsPrvDay = ec21_atoi(argv[9]);
	tibase.ParkCount = ec21_atoi(argv[10]);
	tibase.OverSpeedCount = ec21_atoi(argv[11]);
	tibase.DriveContinueCount = ec21_atoi(argv[12]);
	tibase.OverDriveContinueCount = ec21_atoi(argv[13]);
	tibase.GPSMeterInday = atof(argv[14]);
	tibase.DriverInWeek = (ec21_atoi(argv[15]));
	tibase.DriverInday = ec21_atoi(argv[16]);
	tibase.bMeter = ec21_atoi(argv[17]);
}

static int callback_load_tibase_driver_info(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	log_trace("\n GET Driver Info from DB \n");
	for(i = 0; i<argc; i++) {
		log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	log_trace("\n");

	if(argc < 5){
		log_trace(" ERROR DATABASE \r\n ");
		return -1;
	}

	memset(tibase.driver_info.DriverName, 0x0, MAX_SIZE_DRIVER_NAME + 10);
	strcpy(tibase.driver_info.DriverName, argv[1]);

	memset(tibase.driver_info.DriverLicense, 0x0, MAX_SIZE_DRIVER_LICENSE + 10);
	strcpy(tibase.driver_info.DriverLicense, argv[2]);

	memset(tibase.driver_info.IssDate, 0x0, MAX_SIZE_TAG_DATE + 10);
	strcpy(tibase.driver_info.IssDate, argv[3]);

	memset(tibase.driver_info.ExpDate, 0x0, MAX_SIZE_TAG_DATE + 10);
	strcpy(tibase.driver_info.ExpDate, argv[4]);

	tibase.driver_info.sync = ec21_atoi(argv[5]);

	log_trace("\nName: %s\nLicense: %s\nIssDate: %s\nExpDate: %s \nSync : %d \n", tibase.driver_info.DriverName, tibase.driver_info.DriverLicense, tibase.driver_info.IssDate, tibase.driver_info.ExpDate, tibase.driver_info.sync);
}

static int callback_load_current_driver_data_by_UID(void *NotUsed, int argc, char **argv, char **azColName){
	u64 _UID = 0;
	u64 _METER = 0;
	int i;
	log_trace("\n GET LAST DRIVER DATA \n");
	for(i = 0; i<argc; i++) {
		log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	log_trace("\n");

	_UID = ec21_atoi(argv[0]);

	_METER = ec21_atoi(argv[8]);

	tibase.bRunTime = ec21_atoi(argv[2]);
	tibase.b_loc_run.latitude = atof(argv[4]);
	tibase.b_loc_run.longitude = atof(argv[5]);
	tibase.bMeter = tibase.GPSMeterInday - _METER;

}

static int callback_load_current_driver_summary_data_by_UID(void *NotUsed, int argc, char **argv, char **azColName){
	u64 _UID = 0;
	int i;
	log_trace("\n GET DRIVER SUMMARY \n");
	for(i = 0; i<argc; i++) {
		log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	log_trace("\n");

	_UID = ec21_atoi(argv[0]);

	tibase.DriverInday = ec21_atoi(argv[1]);
	tibase.DriverInWeek = ec21_atoi(argv[2]);
	tibase.ParkCount = ec21_atoi(argv[3]);
	tibase.OverSpeedCount = ec21_atoi(argv[4]);
	tibase.DriveContinueCount = ec21_atoi(argv[5]);
	tibase.OverDriveContinueCount = ec21_atoi(argv[6]);

	log_trace("LOAD IN DAY AND WEEK: %u - %u\n", tibase.DriverInday, tibase.DriverInWeek);
}

static int callback_get_tripinfo(void *NotUsed, int argc, char **argv, char **azColName){
//	int i;
//	log_trace("\n");
//	for(i = 0; i<argc; i++) {
//	  log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	log_trace("\n");

	if(argc < P_TRIPHOUR){
		log_trace("ERROR DATABASE \r\n");
		return -1;
		flg_trip_data = 2;
	}

	Trip_Info.status_1 = ec21_atoi(argv[P_STATUS1]);
	Trip_Info.status_2 = ec21_atoi(argv[P_STATUS2]);
	Trip_Info.tm_stamp = ec21_atoi(argv[P_UTIME]);
	Trip_Info.location.latitude = atof(argv[P_LAT]);
	Trip_Info.location.longitude = atof(argv[P_LON]);
	Trip_Info.angle = ec21_atoi(argv[P_ANGLE]);
	Trip_Info.pulse_speed = ec21_atoi(argv[P_PULSESPEED]);
	Trip_Info.speed_limit = ec21_atoi(argv[P_TRIPSPEEDLIMIT]);
	Trip_Info.gps_distance = (ec21_atoi(argv[P_GPSDISTANCE]) * 10);
	Trip_Info.DriveInWeek = (int16_t)(ec21_atoi(argv[P_PULSEDISTANCE]));
	Trip_Info.fuel1 = ec21_atoi(argv[P_FUEL1]);
	Trip_Info.fuel2 = ec21_atoi(argv[P_FUEL2]);
	Trip_Info.fuel3 = ec21_atoi(argv[P_FUEL3]);
	Trip_Info.tagUID = ec21_atoi(argv[P_TAGUID]);
	Trip_Info.GSMStreng = ec21_atoi(argv[P_GSMSTRENG]);
	Trip_Info.main_power = ec21_atoi(argv[P_MAIN_PW]);
	Trip_Info.batery_power = ec21_atoi(argv[P_BAT_PW]);
	Trip_Info.frqsend = ec21_atoi(argv[P_FRQSEND]);
	Trip_Info.speed1 = ec21_atoi(argv[P_SPEED1]);
	Trip_Info.speed2 = ec21_atoi(argv[P_SPEED2]);
	Trip_Info.speed3 = ec21_atoi(argv[P_SPEED3]);
	Trip_Info.speed4 = ec21_atoi(argv[P_SPEED4]);
	Trip_Info.speed5 = ec21_atoi(argv[P_SPEED5]);
	Trip_Info.speed6 = ec21_atoi(argv[P_SPEED6]);
	Trip_Info.speed7 = ec21_atoi(argv[P_SPEED7]);
	Trip_Info.speed8 = ec21_atoi(argv[P_SPEED8]);
	Trip_Info.speed9 = ec21_atoi(argv[P_SPEED9]);
	Trip_Info.speed10 = ec21_atoi(argv[P_SPEED10]);
	Trip_Info.FlgSend = ec21_atoi(argv[P_FLGSEND]);
	flg_trip_data = 1;
	return 0;
}

static char sql_command[512];
static int callback_copy_tripinfo(void *NotUsed, int argc, char **argv, char **azColName){
//	int i;
//	log_trace("\n");
//	for(i = 0; i<argc; i++) {
//	  log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	log_trace("\n");

	if(argc < P_TRIPHOUR){
		log_trace("ERROR DATABASE \r\n");
		return -1;
	}

	sprintf(sql_command, "INSERT INTO TripInfo  VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
			argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11], argv[12], argv[13], argv[14], argv[15], argv[16], argv[17], argv[18], argv[19], argv[20], argv[21], argv[22], argv[23], argv[24], argv[25], argv[26], argv[27], argv[28], argv[29], argv[30]);

	if(sqlite3_exec(dbtrace, sql_command, callback, (void*)data, &zErrMsg) != SQLITE_OK){
	   log_system("SQL COPY DATA error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}
	return 0;
}

int init_database(void){
	int rc;
	char *sql;
	/* init database semaphore */
	if(sem_init(&db_sem, 0, 1) != 0){
		log_trace("error!!! semaphore database in create\n");
	}

	if(open_database() < 0){
		return -1;
	}

	sql = "CREATE TABLE IF NOT EXISTS DriverSummary (\
			UID	INTEGER UNIQUE NOT NULL, \
	        DriveInday	INTEGER NOT NULL, \
	        DriveInweek	INTEGER NOT NULL, \
			ParkCount	INTEGER NOT NULL, \
			OverSpeedCount	INTEGER NOT NULL, \
			DriveContinueCount	INTEGER NOT NULL, \
			OverDriveContinueCount	INTEGER NOT NULL)";

	/* Execute SQL statement */
	rc = sqlite3_exec(database, sql, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_trace("SQL error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}

	/* LOAD TiBase */
	sql = "SELECT * FROM TiBase LIMIT 1";

	/* Execute SQL statement */
	rc = sqlite3_exec(database, sql, callback_load_tibase, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_trace("SQL error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);

	load_driver_info_by_uid(tibase.UID);

	//send driver info to MCU
	update_rfid_driver_data();

   return 1;
}

int insert_TripInfo(trip_data_typedef GPSData){
	int rc;

	/* Lock Database */
	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	if((GPSData.d_time->tm_year == 80) || (GPSData.d_time->tm_mday < 1) || (GPSData.d_time->tm_mday > 31)){
		sqlite3_close(database);
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "INSERT OR REPLACE INTO TripInfo VALUES ('%d', '%d', '%u', '%f', '%f', '%d', '%d', '%d', '%u', '%d', '%d', '%d', '%d', '%llu', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			GPSData.status_1, GPSData.status_2, GPSData.tm_stamp, GPSData.location.latitude, GPSData.location.longitude, GPSData.angle, GPSData.pulse_speed, GPSData.speed_limit, GPSData.gps_distance,
			tibase.DriverInWeek, GPSData.fuel1, GPSData.fuel2, GPSData.fuel3, GPSData.tagUID, GPSData.GSMStreng, GPSData.main_power, GPSData.batery_power, GPSData.frqsend,
			GPSData.speed1, GPSData.speed2, GPSData.speed3, GPSData.speed4, GPSData.speed5, GPSData.speed6, GPSData.speed7, GPSData.speed8, GPSData.speed9, GPSData.speed10, GPSData.FlgSend, GPSData.d_time->tm_mday, GPSData.d_time->tm_hour);

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
		log_trace("SQL INSERT TRIP error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int inset_DriverData(driver_data_typedef DrvData){
	int rc;

	/* Lock Database */
	sem_wait(&db_sem);
	if (open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	if((DrvData.DrvDay < 1) || (DrvData.DrvDay > 31)){
		sqlite3_close(database);
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "INSERT OR REPLACE INTO DrivingData  VALUES ('%llu', '%d', '%u', '%u', '%f', '%f', '%f', '%f', '%u', '%d', '%d')",
			DrvData.UID, DrvData.Status, DrvData.BTime, DrvData.ETime,
			DrvData.begin_location.latitude, DrvData.begin_location.longitude, DrvData.end_location.latitude, DrvData.end_location.longitude, DrvData.Distance, DrvData.DrvDay, DrvData.FlgSend);
	log_trace(sql_cmd);

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
		log_trace("SQL INSERT DRIVER DATA error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}else{
		log_trace("SQL INSERT DRIVER DATA ok\n");
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int delete_all_tripinfo(void){
	int rc;
	/* Lock Database */
	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	rc = sqlite3_exec(database, "DELETE FROM TripInfo", callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
		log_trace("SQL DLETE ALL TRIPINFO ERROR: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}else{
		log_trace("SQL DLETE  ALL TRIPINFO SUCCESS\n");
	}

	rc = sqlite3_exec(database, "DELETE FROM DrivingData", callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
		log_trace("SQL DLETE ALL DRIVE DATA ERROR: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}else{
		log_trace("SQL DLETE  ALL DRIVE DATA SUCCESS\n");
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int delete_database_ByDay(u8 day){
	int rc;
	/* Lock Database */
	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	if((day < 1) || (day > 31)){
		sqlite3_close(database);
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "DELETE FROM TripInfo WHERE TripDay = %d", day);
	log_trace(sql_cmd);

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
		log_trace("SQL DLETE TRIP error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}else{
		log_trace("SQL DLETE TRIP ok\n");
	}

	sprintf(sql_cmd, "DELETE FROM DrivingData WHERE DrvDay = %d", day);
	log_trace(sql_cmd);

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
		log_trace("SQL DLETE DRIVER DATA error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}else{
		log_trace("SQL DLETE DRIVER DATA ok\n");
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int read_trip_info(trip_data_typedef* trip, time_t stamp){
	u32 time_out = 0;
	int rc;
	/* Lock Database */
	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	flg_trip_data = 0;
	sprintf(sql_cmd, "SELECT * FROM TripInfo WHERE uTime >= %u and uTime <= %u LIMIT 1\0", stamp, (stamp + 30));
	//log_trace(sql_cmd);
	sqlite3_exec(database, sql_cmd, callback_get_tripinfo, (void*)data, &zErrMsg);

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	if(flg_trip_data == 1){
		memcpy(trip, &Trip_Info, sizeof(trip_data_typedef));
		return 0;
	}

	return -1;
}

/*********************************************REPORT QC31VN***********************************************************/
static char str_report[1024];
static char tmp_report[1024];
static int callback_report_tripinfo(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	u8 cs;
	time_t rptime;
	struct tm *d_time;
	u8 frq_send = 0;

//	log_trace("\n");
//	for(i = 0; i<argc; i++) {
//	  log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	log_trace("\n");

	if(argc < 14){
		log_trace(" ERROR DATA \r\n ");
		return -1;
		flg_trip_data = 2;
	}

	rptime = ec21_atoi(argv[0]) - GMT_HN;
	d_time = localtime( &rptime );

	frq_send = ec21_atoi(argv[1]);

	if(frq_send > 10) return -1;

	sprintf(str_report, "%02d:%02d:%02d,%s,%s,%s,%s",
			d_time->tm_hour,
			d_time->tm_min,
			d_time->tm_sec,
			argv[3],//long
			argv[2],//lat
			argv[4 + frq_send],//speed
			argv[4]);//pulse speed

	sprintf(tmp_report, "$GSHT,4,%d,<%s>,", strlen((char*)str_report), str_report);

	cs = cal_sum_str(tmp_report);

	sprintf(str_report, "%s%d#", tmp_report, cs);

	REPORT_DATA(str_report);

	return 0;
}

int report_trip_info(u8 day){
	int rc;
	u8 hour = 0;
	/* Lock Database */
	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}
	/* get tripinfo each 1 hour */
	for(hour = 0; hour < 24; hour ++){
		sprintf(sql_cmd, "SELECT  uTime, FrqSend, Lat, Lon, PulseSpeed, Speed1, Speed2, Speed3, Speed4, Speed5, Speed6, Speed7, Speed8, Speed9, Speed10 FROM TripInfo WHERE TripDay = %d AND TripHour = %d", day, hour);
		log_trace(sql_cmd);

		sqlite3_exec(database, sql_cmd, callback_report_tripinfo, (void*)data, &zErrMsg);

		usleep(1000000);
	}

	/* END REPORT */
	REPORT_DATA("$GSHT,4,0,<>,232#");

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return 0;
}

//SELECT Name, License, BTime, BLon, BLat, ETime, ELon, ELat  FROM DrivingData
static int callback_report_drvcontinue(void *NotUsed, int argc, char **argv, char **azColName){
	u8 cs;
	time_t btime;
	time_t etime;
	struct tm *bd_time;
	struct tm *ed_time;
	int bhour,bmin,bsec;
	int ehour,emin,esec;

//	int i;
//	log_trace("\n");
//	for(i = 0; i<argc; i++) {
//	  log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	log_trace("\n");

	if(argc < 8){
		log_trace(" ERROR DATA \r\n ");
		return -1;
		flg_trip_data = 2;
	}

	btime = (ec21_atoi(argv[2]) - GMT_HN);
	bd_time = localtime( &btime );

	bhour = bd_time->tm_hour;
	bmin = bd_time->tm_min;
	bsec = bd_time->tm_sec;

	etime = (ec21_atoi(argv[5]) - GMT_HN);
	ed_time = localtime( &etime );
	ehour = ed_time->tm_hour;
	emin = ed_time->tm_min;
	esec = ed_time->tm_sec;

	/* Create message */
	memset(str_report, 0x0, sizeof(str_report));
	sprintf(str_report, "%s,%s,%02d:%02d:%02d,%s,%s,%02d:%02d:%02d,%s,%s",
			argv[0],
			argv[1],
			bhour,
			bmin,
			bsec,
			argv[3],
			argv[4],
			ehour,
			emin,
			esec,
			argv[6],
			argv[7]);

	sprintf(tmp_report, "$GSHT,2,%d,<%s>,", strlen((char*)str_report), str_report);

	cs = cal_sum_str(tmp_report);

	sprintf(str_report, "%s%d#", tmp_report, cs);

	REPORT_DATA(str_report);

	return 0;
}

static int callback_report_drvstop(void *NotUsed, int argc, char **argv, char **azColName){
	u8 cs;
	time_t btime;
	time_t etime;
	struct tm *bd_time;
	struct tm *ed_time;
	u32 stop_time;
	int bhour, bmin, bsec;

//	int i;
//	log_trace("\n");
//	for(i = 0; i<argc; i++) {
//	  log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	log_trace("\n");
//  SELECT DriverInfoData.DriverName, DriverInfoData.DriverLicense, DrivingData.BTime, DrivingData.BLon, DrivingData.BLat, DrivingData.ETime, DrivingData.ELon, DrivingData.ELat

	if(argc < 8){
		log_trace(" ERROR DATA \r\n ");
		return -1;
		flg_trip_data = 2;
	}

	btime = ec21_atoi(argv[2]) - GMT_HN;
	bd_time = localtime( &btime );

	bhour = bd_time->tm_hour;
	bmin = bd_time->tm_min;
	bsec = bd_time->tm_sec;

	etime = ec21_atoi(argv[5]) - GMT_HN;
	ed_time = localtime( &etime );

	stop_time = etime - btime;
	stop_time = stop_time / 60;

	/* Create message */
	sprintf(str_report, "%02d:%02d:%02d,%s,%s,%d",
			bhour,
			bmin,
			bsec,
			argv[3],
			argv[4],
			stop_time);

	sprintf(tmp_report, "$GSHT,3,%d,<%s>,", strlen((char*)str_report), str_report);

	cs = cal_sum_str(tmp_report);

	sprintf(str_report, "%s%d#", tmp_report, cs);

	REPORT_DATA(str_report);

	return 0;
}

int report_driver_data(u8 day, u8 state){
	/* Lock Database */
	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd,"SELECT DriverInfoData.DriverName, DriverInfoData.DriverLicense, DrivingData.BTime, DrivingData.BLon, DrivingData.BLat, DrivingData.ETime, DrivingData.ELon, DrivingData.ELat FROM DrivingData LEFT JOIN DriverInfoData ON  DriverInfoData.UID =  DrivingData.UID WHERE DrivingData.DrvDay = %d AND DrivingData.Status = %d", day, state);

	//sprintf(sql_cmd, "SELECT Name, License, BTime, BLon, BLat, ETime, ELon, ELat  FROM DrivingData WHERE DrvDay = %d AND Status = %d", day, state);
	log_trace(sql_cmd);

	if(state == 1){
		sqlite3_exec(database, sql_cmd, callback_report_drvcontinue, (void*)data, &zErrMsg);
		/* END REPORT */
		REPORT_DATA("$GSHT,2,0,<>,230#");
	}else{
		sqlite3_exec(database, sql_cmd, callback_report_drvstop, (void*)data, &zErrMsg);
		/* END REPORT */
		REPORT_DATA("$GSHT,3,0,<>,231#");
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return 0;
}

static int callback_report_speed(void *NotUsed, int argc, char **argv, char **azColName){
	u8 cs;
	time_t rptime;
	u8 frq_send = 0;
	struct tm *d_time;

//	int i;
//	log_trace("\n");
//	for(i = 0; i<argc; i++) {
//	  log_trace("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	log_trace("\n");

	if(argc < 11){
		log_trace(" ERROR DATA \r\n ");
		return -1;
		flg_trip_data = 2;
	}

	rptime = ec21_atoi(argv[0]) - GMT_HN;
	d_time = localtime( &rptime );

	frq_send = ec21_atoi(argv[1]);

	switch(frq_send){
		case 1:{
			sprintf(str_report, "%02d:%02d:%02d,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2]);
			break;
		}
		case 2:{
			sprintf(str_report, "%02d:%02d:%02d,%s,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2],
					argv[3]);
			break;
		}
		case 3:{
			sprintf(str_report, "%02d:%02d:%02d,%s,%s,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2],
					argv[3],
					argv[4]);
			break;
		}
		case 4:{
			sprintf(str_report, "%02d:%02d:%02d,%s,%s,%s,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2],
					argv[3],
					argv[4],
					argv[5]);
			break;
		}
		case 5:{
			sprintf(str_report, "%02d:%02d:%02d,%s,%s,%s,%s,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2],
					argv[3],
					argv[4],
					argv[5],
					argv[6]);
			break;
		}
		case 6:{
			sprintf(str_report, "%02d:%02d:%02d,%s,%s,%s,%s,%s,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2],
					argv[3],
					argv[4],
					argv[5],
					argv[6],
					argv[7]);
			break;
		}
		case 7:{
			sprintf(str_report, "%02d:%02d:%02d,%s,%s,%s,%s,%s,%s,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2],
					argv[3],
					argv[4],
					argv[5],
					argv[6],
					argv[7],
					argv[8]);
			break;
		}
		case 8:{
			sprintf(str_report, "%02d:%02d:%02d,%s,%s,%s,%s,%s,%s,%s,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2],
					argv[3],
					argv[4],
					argv[5],
					argv[6],
					argv[7],
					argv[8],
					argv[9]);
			break;
		}
		case 9:{
			sprintf(str_report, "%02d:%02d:%02d,%s,%s,%s,%s,%s,%s,%s,%s,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2],
					argv[3],
					argv[4],
					argv[5],
					argv[6],
					argv[7],
					argv[8],
					argv[9],
					argv[10]);
			break;
		}
		case 10:{
			sprintf(str_report, "%02d:%02d:%02d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
					d_time->tm_hour,
					d_time->tm_min,
					d_time->tm_sec,
					argv[2],
					argv[3],
					argv[4],
					argv[5],
					argv[6],
					argv[7],
					argv[8],
					argv[9],
					argv[10],
					argv[11]);
			break;
		}
		default: return -1;
	}

	sprintf(tmp_report, "$GSHT,5,%d,<%s>,", strlen((char*)str_report), str_report);

	cs = cal_sum_str(tmp_report);

	sprintf(str_report, "%s%d#", tmp_report, cs);

	REPORT_DATA(str_report);

	return 0;
}

int report_speed(u8 day){
	int rc;
	u8 hour = 0;
	/* Lock Database */
	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}
	/* get tripinfo each 1 hour */
	for(hour = 0; hour < 24; hour ++){
		sprintf(sql_cmd, "SELECT  uTime, FrqSend, Speed1, Speed2, Speed3, Speed4, Speed5, Speed6, Speed7, Speed8, Speed9, Speed10 FROM TripInfo WHERE TripDay = %d AND TripHour = %d", day, hour);
		//log_trace(sql_cmd);

		sqlite3_exec(database, sql_cmd, callback_report_speed, (void*)data, &zErrMsg);
		usleep(1000000);
	}

	/* END REPORT */
	REPORT_DATA("$GSHT,5,0,<>,233#");

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return 0;
}

void report_basic_data(void){
	u32 sum = 0;
	u8 status = 0;
	char tmp[128];

	while(!gps_ready());

	status = (network_data.tcp_status == true) ? 3 : 2;

	/* 1. Units provided */
	sprintf(tmp_report, "$GSHT,1,%d,<1,%s>,", strlen(UNITS_PROVIDED), UNITS_PROVIDED);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 2.device Type */
	sprintf(tmp_report, "$GSHT,1,%d,<2,%s>,", strlen(DEVICE_NAME), DEVICE_NAME);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 3. Seri of device */
	sprintf(tmp, "%llu", SystemSetting.data.DeviceID);
	sprintf((char*)tmp_report, "$GSHT,1,%d,<3,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 4. Information number plate */
	sprintf(tmp_report, "$GSHT,1,%d,<4,%s>,", strlen(DeviceSetting.NumberPlate), DeviceSetting.NumberPlate);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 5. Measurement speed */
	sprintf(tmp_report, "$GSHT,1,1,<5,1>,");
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 6. Setup plues/km */
	sprintf(tmp, "%d", 2560);
	sprintf(tmp_report, "$GSHT,1,%d,<6,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 7. Setup max speed */
	sprintf(tmp, "%d", speed_limit_device);
	sprintf(tmp_report, "$GSHT,1,%d,<7,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 8. Created date  */
	sprintf(tmp_report, "$GSHT,1,%d,<8,%s>,", strlen(DeviceSetting.ManufactureDate), DeviceSetting.ManufactureDate);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 9. Updated fw date */
	sprintf(tmp_report, "$GSHT,1,%d,<9,%s>,", strlen(DeviceSetting.DeviceUpdatedDate), DeviceSetting.DeviceUpdatedDate);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 10. GSM status */
	sprintf(tmp, "%d", status);
	sprintf(tmp_report, "$GSHT,1,%d,<10,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 11. GPS status */
	sprintf(tmp, "%d", GPS_Data.gps_status);
	sprintf(tmp_report, "$GSHT,1,%d,<11,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 12. Memory status */
	sprintf(tmp_report, "$GSHT,1,1,<12,1>,");
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 13. Memory info */
	sprintf(tmp_report, "$GSHT,1,9,<13,104857600>,");
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 14. Current driver info */
	sprintf(tmp, "%s, %s", tibase.driver_info.DriverName, tibase.driver_info.DriverLicense);
	sprintf(tmp_report, "$GSHT,1,%d,<14,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 15. Continuous driving time */
	sprintf(tmp, "%d", (get_run_time() / 60));
	sprintf(tmp_report, "$GSHT,1,%d,<15,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 16. Current coordinates */
	sprintf(tmp, "%f, %f", GPS_Data.location.latitude, GPS_Data.location.longitude);
	sprintf(tmp_report, "$GSHT,1,%d,<16,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 17. Current Speed */
	sprintf(tmp, "%0.2f", GPS_Data.gps_speed);
	sprintf(tmp_report, "$GSHT,1,%d,<17,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 18. Current date time */
	sprintf(tmp, "%d/%02d/%02d %02d:%02d:%02d", (GPS_Data.gps_time->tm_year + 1900), (GPS_Data.gps_time->tm_mon + 1), GPS_Data.gps_time->tm_mday, GPS_Data.gps_time->tm_hour, GPS_Data.gps_time->tm_min, GPS_Data.gps_time->tm_sec);
	sprintf(tmp_report, "$GSHT,1,%d,<18,%s>,", strlen(tmp), tmp);
	sum = cal_sum_str(tmp_report);
	sprintf(str_report, "%s%d#", tmp_report, sum);
	REPORT_DATA(str_report);

	/* 19. END */
	REPORT_DATA("$GSHT,1,0,<>,229#");
}

/*********************************************END REPORT QC31VN****************************************************/

/*********************************************BACKUP GPS***********************************************************/

void send_backup_data(void){
	tcp_mail_queue_typedef buf;

	u8 crc = 0;
	u16 tmp16 = 0;
	u32 tmp32 = 0;
	u64 tmp64 = 0;
	u16 tcp_len = 0;

	/* Frame header */
	buf.data[0] = 0xF0;
	buf.data[1] = 0xF0;

	/* Device ID */
	buf.data[2] =  SystemSetting.data.DeviceID & 0xFF;
	buf.data[3] = (SystemSetting.data.DeviceID >> 8) & 0xFF;
	buf.data[4] = (SystemSetting.data.DeviceID >> 16) & 0xFF;
	buf.data[5] = (SystemSetting.data.DeviceID >> 24) & 0xFF;
	buf.data[6] = (SystemSetting.data.DeviceID >> 32) & 0xFF;
	buf.data[7] = (SystemSetting.data.DeviceID >> 40) & 0xFF;
	buf.data[8] = (SystemSetting.data.DeviceID >> 48) & 0xFF;
	buf.data[9] = (SystemSetting.data.DeviceID >> 56) & 0xFF;
	/* Version */
	buf.data[10] = BACKUP_GPS_DATA;
	/* Status of device  GPS - TCP - ACC - DOOR - FLASH  - power mode (0 - nomal, 1 low pw) - AIR - ST95 */
	buf.data[13] = Trip_Info.status_1;
	buf.data[14] = Trip_Info.status_2;
	/* Date Time*/
	*(int32_t*)(buf.data + 15) = Trip_Info.tm_stamp;

	/* Latitude */
	*(float*)(buf.data + 19) = Trip_Info.location.latitude;

	*(float*)(buf.data + 23) = Trip_Info.location.longitude;
	/* Course */
	tmp16 = (u16)Trip_Info.angle;
	buf.data[27] = tmp16 & 0xFF;
	buf.data[28] = tmp16 >> 8;
	/* Speed */
	buf.data[29] = Trip_Info.speed10;

	/* gps meter */
	tmp16 = (Trip_Info.gps_distance / 100);
	buf.data[30] = tmp16 & 0xFF;
	buf.data[31] = tmp16 >> 8;

	/* pulse speed */
	buf.data[32] = Trip_Info.pulse_speed;
	/* temp */
	//buf.data[33] = Trip_Info.temperature & 0xFF;
	//buf.data[34] = Trip_Info.temperature >> 8;

	buf.data[33] = 0;
	buf.data[34]= 0;

	/* main power value */
	buf.data[35] = Trip_Info.main_power & 0xFF;
	buf.data[36] = Trip_Info.main_power >> 8;

	/* battery power value */
	buf.data[37] = Trip_Info.batery_power & 0xFF;
	buf.data[38] = Trip_Info.batery_power >> 8;

	/* max speed */
	buf.data[39] = Trip_Info.speed_limit;
	/* Signal Strength */
	buf.data[40] = Trip_Info.GSMStreng;
	/* Fuel 1 */
	tmp16 = Trip_Info.fuel1;
	buf.data[41] = tmp16 & 0xFF;
	buf.data[42] = tmp16 >> 8;
	/* Fuel 2 */
	tmp16 = Trip_Info.fuel2;
	buf.data[43] = tmp16 & 0xFF;
	buf.data[44] = tmp16 >> 8;
	/* fuel3 data */
	tmp16 = Trip_Info.fuel3;
	buf.data[45] = tmp16 & 0xFF;
	buf.data[46] = tmp16 >> 8;

	/* ID of rfid card */
	tmp64 = Trip_Info.tagUID;
	buf.data[47] =  tmp64 & 0xFF;
	buf.data[48] = (tmp64 >> 8) & 0xFF;
	buf.data[49] = (tmp64 >> 16) & 0xFF;
	buf.data[50] = (tmp64 >> 24) & 0xFF;
	buf.data[51] = (tmp64 >> 32) & 0xFF;
	buf.data[52] = (tmp64 >> 40) & 0xFF;
	buf.data[53] = (tmp64 >> 48) & 0xFF;
	buf.data[54] = (tmp64 >> 56) & 0xFF;

	/* frequency send gps data */
	buf.data[55] = Trip_Info.frqsend & 0xFF;
	buf.data[56] = Trip_Info.frqsend >> 8;

	/* speeds over seconds */
	buf.data[57] = Trip_Info.speed1;
	buf.data[58] = Trip_Info.speed2;
	buf.data[59] = Trip_Info.speed3;
	buf.data[60] = Trip_Info.speed4;
	buf.data[61] = Trip_Info.speed5;
	buf.data[62] = Trip_Info.speed6;
	buf.data[63] = Trip_Info.speed7;
	buf.data[64] = Trip_Info.speed8;
	buf.data[65] = Trip_Info.speed9;
	buf.data[66] = Trip_Info.speed10;

	/* Length data */
	tcp_len = 68;
	buf.data[11] = tcp_len & 0xFF;
	buf.data[12] = tcp_len >> 8;

	/* CRC */
	crc = CRC_8BitsCompute(buf.data, 67);
	buf.data[67] = crc;

	buf.size = tcp_len;
	buf.Data_Type = BACKUP_GPS_DATA;
	buf.trace = Trip_Info.tm_stamp;

	log_nw("GUI BACKUP %d \n", buf.trace);

	Push_TCP_Data(buf);
}

int clear_flg_send(u32 stamp){
	int rc;

	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	if(stamp == 0){
		sprintf(sql_cmd, "UPDATE TripInfo SET FlgSend = '1'\0");
	}else{
		sprintf(sql_cmd, "UPDATE TripInfo SET FlgSend = '1' WHERE uTime = '%u'\0", stamp);
	}

	//log_trace(sql_cmd);

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_nw("SQL error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

void* thread_backup(void* arg){
	u32 this_time = 0;
	int rc;
	static useconds_t sleep__ = 10000000;
	for(;;){
		sleep__ = 10000000;//10 seconds

		/* wait server connected */
		if(network_data.tcp_status == false){
			sleep(10);
			continue;
		}

		if(!isEmpty(&queue_tcp)){
			usleep(100000);
			continue;
		}

		/* Lock Database */
		sem_wait(&db_sem);
		if(open_database() < 0){
			/* release Database */
			sem_post(&db_sem);
			sleep(10);
			continue;
		}

		this_time = GPS_Data.timestamp;

		sprintf(sql_cmd, "SELECT * FROM TripInfo WHERE FlgSend = 0 AND uTime < '%u' LIMIT 1\0", (this_time - 60));
		//log_trace(sql_cmd);
		flg_trip_data = 0;
		sqlite3_exec(database, sql_cmd, callback_get_tripinfo, (void*)data, &zErrMsg);

		if(flg_trip_data == 1){
			send_backup_data();
			sleep__ = 500000;//500ms
		}else{
			sleep__ = 10000000;//10s
		}

		sqlite3_close(database);
		/* release Database */
		sem_post(&db_sem);
		usleep(sleep__);
	}
}

/*******************************************END BACKUP GPS*********************************************************/
int set_backup_request(u8* btime, u8* etime){
	int rc;
	/* Lock Database */
	sem_wait(&db_sem);
	if (open_database() < 0 ){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "UPDATE TripInfo SET FlgSend = '0' WHERE uTime >= '%s'and uTime <= '%s'\0", btime, etime);
	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if( rc != SQLITE_OK ) {
	   log_trace("SQL error: %s\n", zErrMsg);
	  sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

u8 db_wait___ = 0;
int update_tibase(tibase_typedef tib){
	int rc;

	if(db_wait___ != 0) return -1;

	/* Lock Database */
	sem_wait(&db_sem);

	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	/************************Tibase****************************/
	sprintf(sql_cmd, "INSERT OR REPLACE INTO TiBase VALUES (1, '%d', '%llu', '%f', '%f', '%f', '%f', '%u', '%u', '%u', '%d', '%d', '%d', '%d', '%f', '%u', '%u', '%u')\0",
			(u8)tib.LoginStatus, tib.UID, tib.b_loc_run.latitude, tib.b_loc_run.longitude, tib.b_loc_stop.latitude, tib.b_loc_stop.longitude, tib.bRunTime, tib.bStopTime, tib.RunSecondsPrvDay,
			tib.ParkCount, tib.OverSpeedCount, tib.DriveContinueCount, tib.OverDriveContinueCount, tib.GPSMeterInday, tib.DriverInWeek, tib.DriverInday, tib.bMeter);

	log_trace("\n");
	log_trace(sql_cmd);
	log_trace("\n");

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_trace("SQL error: %s\n", zErrMsg);
	  sqlite3_free(zErrMsg);
	}
	/*************************End_Tibase***************************/


	/************************DriverSummary****************************/
	sprintf(sql_cmd, "INSERT OR REPLACE INTO DriverSummary VALUES ( '%llu', '%u', '%u', '%d', '%d', '%d', '%d')\0",
			tib.UID, tib.DriverInday, tib.DriverInWeek, tib.ParkCount, tib.OverSpeedCount, tib.DriveContinueCount, tib.OverDriveContinueCount);

	log_trace("\n");
	log_trace(sql_cmd);
	log_trace("\n");

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_trace("SQL error: %s\n", zErrMsg);
	  sqlite3_free(zErrMsg);
	}
	/************************End_DriverSummary****************************/

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int clear_summary_data(u8 clear_day, u8 clear_week){
	int rc;

	/* Lock Database */
	sem_wait(&db_sem);

	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	/*************************Clear day***************************/
	if(clear_day == 1){
		/*************************Clear day***************************/
		sprintf(sql_cmd, "UPDATE DriverSummary SET DriveInday = 0, ParkCount = 0, OverSpeedCount = 0, DriveContinueCount = 0, OverDriveContinueCount = 0");

		log_trace("\n");
		log_trace(sql_cmd);
		log_trace("\n");

		rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
		if(rc != SQLITE_OK){
		   log_trace("SQL error: %s\n", zErrMsg);
		  sqlite3_free(zErrMsg);
		}
	}
	/*************************Clear day***************************/


	/*************************Clear week***************************/
	if(clear_week == 1){
		sprintf(sql_cmd, "UPDATE DriverSummary SET DriveInweek = 0, DriveInday = 0, ParkCount = 0, OverSpeedCount = 0, DriveContinueCount = 0, OverDriveContinueCount = 0");

		log_trace("\n");
		log_trace(sql_cmd);
		log_trace("\n");

		rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
		if(rc != SQLITE_OK){
		   log_trace("SQL error: %s\n", zErrMsg);
		  sqlite3_free(zErrMsg);
		}
	}
	/*************************Clear week***************************/

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int load_driver_info_by_uid(u64 uid){
	int rc;

	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "SELECT * FROM DriverInfoData WHERE UID = '%llu' LIMIT 1\0", uid);
	log_trace(sql_cmd);

	rc = sqlite3_exec(database, sql_cmd, callback_load_tibase_driver_info, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_nw("SQL error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int load_driver_summary_data_by_uid(u64 uid){
	int rc;

	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "SELECT * FROM DriverSummary WHERE UID = '%llu' LIMIT 1\0", uid);

	log_trace("\n");
	log_trace(sql_cmd);
	log_trace("\n");

	rc = sqlite3_exec(database, sql_cmd, callback_load_current_driver_summary_data_by_UID, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_nw("SQL error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int load_driver_data_by_uid(u64 uid, u32 time__t){
	int rc;

	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "SELECT * FROM DrivingData WHERE Status = '1' AND UID = '%llu' AND ETime > %u LIMIT 1\0", uid, (time__t - TIME_15_MINUTES));

	log_trace("\n");
	log_trace(sql_cmd);
	log_trace("\n");
	rc = sqlite3_exec(database, sql_cmd, callback_load_current_driver_data_by_UID, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_nw("SQL error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int update_driver_sync_by_uid(u64 uid, u8 sync){
	int rc;

	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "UPDATE DriverInfoData SET sync = %d where UID = %llu\0", sync, uid);
	//log_trace(sql_cmd);

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_nw("SQL error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int update_driver_info_by_uid(driver_info_def drv, u64 uid){
	int rc;

	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "INSERT OR REPLACE INTO DriverInfoData VALUES ('%llu', '%s', '%s', '%s', '%s')\0",
			uid, drv.DriverName, drv.DriverLicense, drv.IssDate, drv.ExpDate);
	//log_trace(sql_cmd);

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_nw("SQL error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

int update_driver_info_by_server(u8 * cmd){
	int rc;

	sem_wait(&db_sem);
	if(open_database() < 0){
		/* release Database */
		sem_post(&db_sem);
		return -1;
	}

	sprintf(sql_cmd, "INSERT OR REPLACE INTO DriverInfoData VALUES (%s)\0",
			cmd);

	log_trace("\n");
	log_trace(sql_cmd);
	log_trace("\n");

	rc = sqlite3_exec(database, sql_cmd, callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
	   log_nw("SQL error: %s\n", zErrMsg);
	   sqlite3_free(zErrMsg);
	}

	sqlite3_close(database);
	/* release Database */
	sem_post(&db_sem);

	return (rc == SQLITE_OK) ? 0 : -1;
}

void upload_database(void){
	char str_cmd[160];

	sprintf(str_cmd,"curl -v -F filename=/usrdata/database.db -F upload=@/usrdata/database.db %s\0", DeviceSetting.upload_db_url);
	system(str_cmd);
}

