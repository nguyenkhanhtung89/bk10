#include <ql_oe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver.h"
#include "utils.h"
#include "log.h"
#include "network.h"
#include "devconfig.h"

tibase_typedef tibase;

static CAR_STATUS CarStatus = STATUS_UNKNOW;

static void SendDriverData(CAR_STATUS status, time_t BeginTime, time_t EndTime, loc_def b_location, loc_def e_location, u32 meter);

static void SendLoginState(void);

static int vehicle_mov_time = 0;
static int vehicle_stop_time = 0;
static u16 notify_time_cnt = 0;
static loc_def tmp_blocrun;
static u8 flg_tmp_blocrun = 0;
static u32  time_start_tmp = 0;

void clear_driver_result(void){
	vehicle_mov_time = 0;
	vehicle_stop_time = 0;
	tmp_blocrun.latitude = 0;
	tmp_blocrun.longitude = 0;
	flg_tmp_blocrun = 0;
	time_start_tmp = 0;

	tibase.bRunTime = 0;
	tibase.bStopTime = 0;
	tibase.OverSpeedCount = 0;
	tibase.ParkCount = 0;
	tibase.GPSMeter= 0;
	tibase.DriverInday = 0;
	tibase.DriveContinueCount = 0;
	tibase.OverDriveContinueCount = 0;
	tibase.b_loc_run.latitude = tibase.b_loc_run.longitude = 0;
	tibase.b_loc_stop.latitude = tibase.b_loc_stop.longitude = 0;
	tibase.bMeter = 0;
	/* update database */
	update_tibase(tibase);
	log_debug("XOA DU LIEU LX THANH CONG\n");
}

void clear_driver_info(void){
	/* Clear driver infomation */
	memset(tibase.driver_info.DriverName, 0x0, MAX_SIZE_DRIVER_NAME);
	memset(tibase.driver_info.DriverLicense, 0x0, MAX_SIZE_DRIVER_LICENSE);
	memset(tibase.driver_info.IssDate, 0x0, MAX_SIZE_TAG_DATE);
	memset(tibase.driver_info.ExpDate, 0x0, MAX_SIZE_TAG_DATE);
	tibase.UID = 1;
}

int write_driver_data(CAR_STATUS state, time_t btime, time_t etime, loc_def this_loc, u8 current_day){
	driver_data_typedef tmp;
	memset(&tmp, 0x0, sizeof(driver_data_typedef));

	/*Insert Driver data to database*/
	tmp.BTime = btime;
	tmp.ETime = etime;
	tmp.Status = state;
	tmp.UID = tibase.UID;

	if(state == STATUS_RUNNING){
		/* begin run location */
		tmp.begin_location = tibase.b_loc_run;

		tmp.Distance = tibase.GPSMeter - tibase.bMeter;

		tibase.DriveContinueCount += 1;
		if((etime - btime) > TIME_4_HOUR){
			tibase.OverDriveContinueCount += 1;
		}
	}else{
		/* begin stop location */
		tmp.begin_location = this_loc;

		tmp.Distance = 0;
		/* increment park count */
		tibase.ParkCount += 1;
	}

	/* end location */
	tmp.end_location = this_loc;

	tmp.FlgSend = 0;
	tmp.DrvDay = current_day;

	/*send driver data to server*/
	SendDriverData(state, btime, etime, tmp.begin_location, tmp.end_location, tmp.Distance);

	if(inset_DriverData(tmp) < 0){
		log_debug("WRITE DRIVER DATA ERROR\n");
		return -1;
	}

	log_debug("WRITE DRIVER DATA SUCCESS\n");
	return 0;
}

u32 get_run_time(void){
	if((GPS_Data.timestamp < TIMESTAMP_2022) || (tibase.bRunTime < TIMESTAMP_2022)) return 0;

	if(tibase.bStopTime > TIMESTAMP_2022) return (u32)(tibase.bStopTime - tibase.bRunTime) + tibase.RunSecondsPrvDay;

	return (u32)(GPS_Data.timestamp - tibase.bRunTime) + tibase.RunSecondsPrvDay;
}

u32 get_stop_time(void){
	if((GPS_Data.timestamp < TIMESTAMP_2022) || (tibase.bStopTime < TIMESTAMP_2022)) return 0;

	return (u32)(GPS_Data.timestamp - tibase.bStopTime);
}

void calculate_drive_data(void){
	if(GPS_Data.timestamp < TIMESTAMP_2022) return;
	//log_trace("RUN -> %d, STOP -> %d \n", tibase.RunSeconds, tibase.StopSeconds);
	if(GPS_Data.gps_speed > SystemSetting.data.RunLevel){/* RUNNING... */

		if(flg_tmp_blocrun == 0){
			tmp_blocrun = GPS_Data.location;
			time_start_tmp = GPS_Data.timestamp;
			flg_tmp_blocrun = 1;
		}

		if((vehicle_mov_time >= 0) && (vehicle_mov_time <= 20)){
			vehicle_mov_time ++;
			return;
		}

		flg_tmp_blocrun = 0;
		/* Check car status */
		if((CarStatus == STATUS_STOPPING) && (tibase.bStopTime != 0)){//truoc do dung do
			/* increase driver inday if stop less than 15 min  */
			if(tibase.bRunTime != 0){
				tibase.DriverInday += get_stop_time();
			}
			/* Write date stop */
			if(get_stop_time() >= SystemSetting.data.DrvStopTime){
				/* Write driver data */
				write_driver_data(STATUS_STOPPING, (time_t)tibase.bStopTime, (time_t)GPS_Data.timestamp, tibase.b_loc_stop, (u8)GPS_Data.gps_time->tm_mday);
			}
			update_tibase(tibase);
		}

//		if(get_run_time() > (tibase.DriverInday + TIME_1_MINUTES + tibase.RunSecondsPrvDay)){
//			tibase.bRunTime = 0;
//		}

		/* clear stop data */
		if(tibase.bStopTime != 0){
			if(get_stop_time() >= TIME_15_MINUTES){
				/* reset driver data and update tibase */
				tibase.bRunTime = 0;
				/* Reset time driving pre time */
				tibase.RunSecondsPrvDay = 0;
				/* clear trace */
				tibase.b_loc_run.latitude = tibase.b_loc_run.longitude = 0;
				tibase.bMeter = 0;
			}
			
			tibase.bStopTime = 0;
			/* clear trace */
			tibase.b_loc_stop.latitude = tibase.b_loc_stop.longitude = 0;
			update_tibase(tibase);

			//log_system("xoa thoi gian dung: => %u, %f, %f \n", tibase.bStopTime, tibase.b_loc_stop.latitude, tibase.b_loc_stop.longitude);
		}

		/* get start running data */
		if(tibase.bRunTime == 0){
			tibase.b_loc_run = tmp_blocrun;
			tibase.bMeter = tibase.GPSMeter;
			tibase.bRunTime = time_start_tmp;
			update_tibase(tibase);

			//log_system("loc start = %f, %f \n", tibase.b_loc_run.latitude, tibase.b_loc_run.longitude);
		}

		/* Change car status */
		CarStatus = STATUS_RUNNING;

		/* increment run seconds */
		if(vehicle_mov_time > 0){
			tibase.DriverInday += vehicle_mov_time;
			vehicle_mov_time = -1;
		}else{
			tibase.DriverInday += 1;
		}

		/* Check next day */
		if(GPS_Data.flg_new_day == 1){
			/* Write current driver to new day*/
			SendLoginState();

			if((GPS_Data.gps_time->tm_hour == 0) && (GPS_Data.gps_time->tm_min == 0)){
				/* Check time driving */
				if((get_run_time() > TIME_1_MINUTES) && (get_run_time() < TIME_1_DAY)){
					struct tm *last_tm;
					time_t yesterday;

					yesterday = (tibase.bRunTime - GMT_HN);
					last_tm = localtime( &yesterday );

					/* Write driver data */
					write_driver_data(STATUS_RUNNING, (time_t)tibase.bRunTime, (time_t)(GPS_Data.timestamp - 60), GPS_Data.location, (u8)(last_tm->tm_mday - 1));

					/* Store pre sec run time */
					tibase.RunSecondsPrvDay = get_run_time();
				}
			}else{
				tibase.RunSecondsPrvDay = 0;
			}

			/* reset driver data */
			clear_driver_result();
			/* update tibase to database */
			update_tibase(tibase);
			/* clear flag new day */
			GPS_Data.flg_new_day = 0;
		}

		/* Check over driving time notify */
		if(get_run_time() > (TIME_4_HOUR - SystemSetting.data.DrvForeWarningTime)){
			notify_time_cnt ++;
			if(get_run_time() > TIME_4_HOUR){
				if((notify_time_cnt % 20) == 0){
					add_play_list("/usrdata/sound/", "violationdrcontinue.wav", SystemSetting.data.SpeakerVolume);
				}
			}else{
				if((notify_time_cnt % 20) == 0){
					add_play_list("/usrdata/sound/", "warningdrcontinue.wav", SystemSetting.data.SpeakerVolume);
				}
			}

			/* notify */
			tibase.over_driver_continue_status = true;
		}else{
			tibase.over_driver_continue_status = false;
		}
	}else{/* STOPPING... */
		tibase.over_driver_continue_status = false;
		flg_tmp_blocrun = 0;

		/* get begin stop time */
		if(tibase.bStopTime == 0){
			tibase.bStopTime = GPS_Data.timestamp;
			tibase.b_loc_stop = GPS_Data.location;
			update_tibase(tibase);
			//log_system("loc stop = %f, %f \n", tibase.b_loc_stop.latitude, tibase.b_loc_stop.longitude);
		}

		/* increment stop seconds */
		if(vehicle_mov_time != 0){
			vehicle_mov_time = 0;
		}

		/* Check time driving */
		if(((tibase.bRunTime != 0) && (tibase.bStopTime != 0)) || (tibase.RunSecondsPrvDay != 0)){
			if(get_stop_time() > TIME_15_MINUTES){
				/* Write driver data */
				write_driver_data(STATUS_RUNNING, (time_t)tibase.bRunTime, (time_t)tibase.bStopTime, GPS_Data.location, (u8)GPS_Data.gps_time->tm_mday);

				tibase.bRunTime = 0;

				/* Reset time driving pre time */
				tibase.RunSecondsPrvDay = 0;

				/* clear trace */
				tibase.b_loc_run.latitude = tibase.b_loc_run.longitude = 0;

				tibase.bMeter = 0;

				update_tibase(tibase);

				//log_system("xoa thoi gian chay: => %u, %f, %f \n", tibase.bRunTime, tibase.b_loc_run.latitude, tibase.b_loc_run.longitude);
			}
		}

		/* Change status car */
		CarStatus = STATUS_STOPPING;

		/* Check next day */
		if(GPS_Data.flg_new_day == 1){
			struct tm *last_tm;
			time_t yesterday;

			/* Write current driver to new day*/
			SendLoginState();

			if((GPS_Data.gps_time->tm_hour == 0) && (GPS_Data.gps_time->tm_min == 0)){
				/* Check time driving */
				if((tibase.bRunTime != 0) && (tibase.bStopTime != 0)){
					yesterday = (tibase.bRunTime - GMT_HN);
					last_tm = localtime( &yesterday );
					/* Write driver data */
					write_driver_data(STATUS_RUNNING, (time_t)tibase.bRunTime, (time_t)tibase.bStopTime, GPS_Data.location, (u8)(last_tm->tm_mday - 1));
				}else if(tibase.bStopTime != 0){
					yesterday = (tibase.bStopTime - GMT_HN);
					last_tm = localtime( &yesterday );
					/* Write driver data */
					write_driver_data(STATUS_STOPPING, (time_t)tibase.bStopTime, (time_t)(GPS_Data.timestamp - 60), tibase.b_loc_stop, (u8)(last_tm->tm_mday - 1));
				}
			}
			/* reset driver data */
			clear_driver_result();
			/* update tibase to database */
			update_tibase(tibase);
			/* clear flag new day */
			GPS_Data.flg_new_day = 0;
		}
	}
	//log_system("run = %u, stop = %u, blat_run = %f, blon_run = %f, blat_stop = %f, blon_stop = %f\n", get_run_time(), get_stop_time(), tibase.b_loc_run.latitude, tibase.b_loc_run.longitude, tibase.b_loc_stop.latitude, tibase.b_loc_stop.longitude);
}

#define SIZE_OF_UID 8
int driver_login(u8 * info, u8 chk){
	u64 UID = 0;
	u8 bUID[10];

	if(GPS_Data.gps_status == 0){
		add_play_list("/usrdata/sound/", "loginfailgps.wav", SystemSetting.data.SpeakerVolume);
		return -1;
	}else if(GPS_Data.gps_speed > SystemSetting.data.RunLevel){
		add_play_list("/usrdata/sound/", "loginfailmove.wav", SystemSetting.data.SpeakerVolume);
		return -1;
	}

	/* get UID */
	memset(bUID, 0x0, 10);
	memcpy(bUID, info, SIZE_OF_UID - 3);

	UID = *(u64*)(bUID);
	log_debug("-> UID = %llu\n", UID);

	sleep(3);

	if(tibase.LoginStatus == STATUS_LOGIN){//current driver card
		if(tibase.UID == UID){
			log_system("DRIVER LOGOUT\n");
			add_play_list("/usrdata/sound/", "logoutsuccess.wav", SystemSetting.data.SpeakerVolume);

			driver_logout();
			//Load driver infor by UID
			load_driver_info_by_uid(tibase.UID);
			//send driver info to RFID

			log_system("DRIVER INFO UPDATE 2\n");
			update_rfid_driver_data();
			if(chk == 1)
				return  0;
		}
		driver_logout();
	}else{
		/* Write information current driver */
		if((tibase.bStopTime != 0) && (tibase.bRunTime != 0)){
			tibase.DriverInday += get_stop_time();
			/* Write infomation run */
			write_driver_data(STATUS_RUNNING, (time_t)tibase.bRunTime, (time_t)tibase.bStopTime, tibase.b_loc_run, (u8)GPS_Data.gps_time->tm_mday);

		}else if(get_stop_time() >= SystemSetting.data.DrvStopTime){
			/* Write infomation stop */
			write_driver_data(STATUS_STOPPING, (time_t)tibase.bStopTime, (time_t)GPS_Data.timestamp, tibase.b_loc_stop, (u8)GPS_Data.gps_time->tm_mday);
		}
	}

	/* Save Driver information to Tibase */
	tibase.UID = UID;
	memset(tibase.driver_info.DriverName, 0x0, MAX_SIZE_DRIVER_NAME + 10);
	memset(tibase.driver_info.DriverLicense, 0x0, MAX_SIZE_DRIVER_LICENSE + 10);
	memset(tibase.driver_info.IssDate, 0x0, MAX_SIZE_TAG_DATE + 10);
	memset(tibase.driver_info.ExpDate, 0x0, MAX_SIZE_TAG_DATE + 10);

	/* Reset time driving time */
	tibase.bRunTime = 0;
	tibase.bStopTime = 0;

	/* Reset time driving pre time */
	tibase.RunSecondsPrvDay = 0;

	/* Set login status */
	tibase.LoginStatus = STATUS_LOGIN;

	//Load driver infor by UID
	load_driver_info_by_uid(tibase.UID);

	//send driver info to MCU
	log_system("DRIVER INFO UPDATE 3\n");

	update_rfid_driver_data();

	/* reload last driver result */
	load_driver_data_by_uid(tibase.UID, GPS_Data.timestamp);

	/* update Tibase */
	update_tibase(tibase);

	log_system("DRIVER LOGIN\n");

	add_play_list("/usrdata/sound/", "loginsuccess.wav", SystemSetting.data.SpeakerVolume);

	/* Send Driver information to SV */
	SendLoginState();

	return 0;
}

u8 update_sync_status(u8 * info){
	u64 UID = 0;
	u8 bUID[10];

	/* get UID */
	memset(bUID, 0x0, 10);
	memcpy(bUID, info, SIZE_OF_UID - 3);

	UID =  *(u64*)(bUID);
	log_debug("DRIVER CARD HAS SYNC, UID = %llu\n", UID);
	//update database
	update_driver_sync_by_uid(UID, 0);

	return 1;
}

void driver_logout(void){
	if(tibase.LoginStatus == STATUS_LOGIN){
		/* Write information current driver */
		if((tibase.bStopTime != 0) && (tibase.bRunTime != 0)){
			tibase.DriverInday += get_run_time();
			tibase.DriverInday += get_stop_time();

			/* Car running 3 minute */
			if(get_run_time()  > TIME_3_MINUTES){
				/* Write infomation run */
				write_driver_data(STATUS_RUNNING, (time_t)tibase.bRunTime, (time_t)tibase.bStopTime, tibase.b_loc_stop, (u8)GPS_Data.gps_time->tm_mday);
			}
		}

		if(get_stop_time() >= SystemSetting.data.DrvStopTime){
			/* Write infomation stop */
			write_driver_data(STATUS_STOPPING, (time_t)tibase.bStopTime, (time_t)GPS_Data.timestamp, tibase.b_loc_stop, (u8)GPS_Data.gps_time->tm_mday);
		}

		/* clear driver information */
		clear_driver_info();

		/* Reset time driving time */
		tibase.bRunTime = 0;
		tibase.bStopTime = 0;

		/* Reset time driving pre time */
		tibase.RunSecondsPrvDay = 0;

		/* change login status */
		tibase.LoginStatus = STATUS_LOGOUT;

		/* update Tibase */
		update_tibase(tibase);

		/* Send Driver information to SV */
		SendLoginState();
	}
}

static void SendLoginState(void){
	tcp_mail_queue_typedef buf;
	u8 crc = 0;

	memset(buf.data, 0, TCP_MAX_LENGTH);

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
	buf.data[10] = DRIVER_INFO;

	buf.data[13] = tibase.LoginStatus;

	buf.data[14] =  tibase.UID & 0xFF;
	buf.data[15] = (tibase.UID >> 8) & 0xFF;
	buf.data[16] = (tibase.UID >> 16) & 0xFF;
	buf.data[17] = (tibase.UID >> 24) & 0xFF;
	buf.data[18] = (tibase.UID >> 32) & 0xFF;
	buf.data[19] = (tibase.UID >> 40) & 0xFF;
	buf.data[20] = (tibase.UID >> 48) & 0xFF;
	buf.data[21] = (tibase.UID >> 56) & 0xFF;

	buf.data[22] = tibase.driver_info.sync;

	buf.size = 24;
	/* Length data */
	buf.data[11] = buf.size & 0xFF;
	buf.data[12] = (buf.size >> 8);

	crc = CRC_8BitsCompute(buf.data, 23);
	buf.data[23] = crc;

	buf.Data_Type = DRIVER_INFO;
	buf.trace = 0;

	Push_TCP_Data(buf);
}

static void SendDriverData(CAR_STATUS status, time_t BeginTime, time_t EndTime, loc_def b_location, loc_def e_location, u32 meter){
	u8 crc = 0;
	tcp_mail_queue_typedef buf;

	memset(buf.data, 0, TCP_MAX_LENGTH);

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
	buf.data[10] = DRIVER_IN_OUT;

	/* Car Status */
	buf.data[13] = status;

	/* Begin time */
	*(int32_t*)(buf.data + 14) =  BeginTime;
	/* Begin Lat */
	*(float*)(buf.data + 18) = b_location.latitude;
	/* Begin long */
	*(float*)(buf.data + 22) = b_location.longitude;
	/* End time */
	*(int32_t*)(buf.data + 26) = EndTime;
	/* End Lat */
	*(float*)(buf.data + 30) = e_location.latitude;
	/* End Long */
	*(float*)(buf.data + 34) = e_location.longitude;
	/* mile meter in trip */
	*(int32_t*)(buf.data + 38) =  meter;

	buf.data[42] =  tibase.UID & 0xFF;
	buf.data[43] = (tibase.UID >> 8) & 0xFF;
	buf.data[44] = (tibase.UID >> 16) & 0xFF;
	buf.data[45] = (tibase.UID >> 24) & 0xFF;
	buf.data[46] = (tibase.UID >> 32) & 0xFF;
	buf.data[47] = (tibase.UID >> 40) & 0xFF;
	buf.data[48] = (tibase.UID >> 48) & 0xFF;
	buf.data[49] = (tibase.UID >> 56) & 0xFF;

	buf.size = 51;
	buf.data[11] = buf.size & 0xFF;
	buf.data[12] = buf.size >> 8;

	crc = CRC_8BitsCompute(buf.data, 50);
	buf.data[50] = crc;

	buf.Data_Type = DRIVER_IN_OUT;
	buf.trace = BeginTime;

	Push_TCP_Data(buf);
}


void sync_tibase(void){
	update_tibase(tibase);
}
