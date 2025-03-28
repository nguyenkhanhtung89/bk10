#include <ql_oe.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mcu.h"
#include "gps.h"
#include "fw.h"
#include "network.h"
#include "log.h"
#include "database.h"
#include "utils.h"
#include "setting.h"
#include "driver.h"
#include "devconfig.h"
#include "audio.h"
#include "rfid.h"
#include "camera.h"
#include "filter.h"

static eNumCMDCode SetKeyActive(u8* msg){
	s32 len = 0;
	len = strlen(msg);
	if(len > 60) return INVALID_MESSAGE;
	msg[len] = 0;

	strcpy(SystemSetting.data.KeyActive, msg);
	write_string(KEYACTIVE_PATH, SystemSetting.data.KeyActive);
	if(strlen(SystemSetting.data.KeyActive) <= 10){
		SystemSetting.data.certified = 1;
		write_number(CERTIFIED_PATH, (u64)SystemSetting.data.certified);
	}else{
		/* recertify  */
		SystemSetting.data.certified = 0;
	}

	write_number(CERTIFIED_PATH, (u64)SystemSetting.data.certified);

	return CMD_ACTIVE_KEY;
}

static eNumCMDCode SetRunLevel(u8* msg){
	u64 level = ec21_atoi(msg);

	/* check value */
	if(level > 40) return INVALID_MESSAGE;

	//compare with old deviceid
	if(level != SystemSetting.data.RunLevel){
		/* setting new id and reconnect */
		SystemSetting.data.RunLevel = level;
		write_number(RUN_LEVEL_PATH, (u64)SystemSetting.data.RunLevel);

		return CMD_RUNLEVEL;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetSlienceMode(u8* msg){
	u64 mode = ec21_atoi(msg);

	/* check value */
	if(mode > 1) return INVALID_MESSAGE;

	//compare with old deviceid
	if(mode != SystemSetting.data.SlienceMode){
		/* setting new id and reconnect */
		SystemSetting.data.SlienceMode = mode;
		write_number(SLIENCE_MODE_PATH, (u64)SystemSetting.data.SlienceMode);

		return CMD_SLIENCE;
	}
	return ALREADY_SETTING;
}

static void SetDisableCam(u8* msg){
	if(str_find(msg, "ON") >= 0){
		SystemSetting.data.disable_cam = 0;
	}else if(str_find(msg, "OFF") >= 0){
		SystemSetting.data.disable_cam = 1;
	}
	write_number(DISABLE_CAM_PATH, (u64)SystemSetting.data.disable_cam);
}

static void SetDisableCamRoaming(u8* msg){
	if(str_find(msg, "ON") >= 0){
		SystemSetting.data.disable_camera_roaming_mode = 0;
	}else if(str_find(msg, "OFF") >= 0){
		SystemSetting.data.disable_camera_roaming_mode = 1;
	}
	write_number(DIS_CAM_ROAMING_PATH, (u64)SystemSetting.data.disable_camera_roaming_mode);
}

static void SetDisableAudio(u8* msg){
	if(str_find(msg, "ON") >= 0){
		SystemSetting.data.disable_audio = 0;
	}else if(str_find(msg, "OFF") >= 0){
		SystemSetting.data.disable_audio = 1;
	}
	write_number(DISABLE_AUDIO_PATH, (u64)SystemSetting.data.disable_audio);
}

static void SetFuel3_to_2(u8* msg){
	if(str_find(msg, "ON") >= 0){
		SystemSetting.data.fuel_3_to_2 = 1;
	}else if(str_find(msg, "OFF") >= 0){
		SystemSetting.data.fuel_3_to_2 = 0;
	}
	write_number(FUEL_3_TO_2_PATH, (u64)SystemSetting.data.fuel_3_to_2);
}

static void SetFuel3_to_1(u8* msg){
	if(str_find(msg, "ON") >= 0){
		SystemSetting.data.fuel_3_to_1 = 1;
	}else if(str_find(msg, "OFF") >= 0){
		SystemSetting.data.fuel_3_to_1 = 0;
	}
	write_number(FUEL_3_TO_1_PATH, (u64)SystemSetting.data.fuel_3_to_1);
}

static void SetFuel2_to_1(u8* msg){
	if(str_find(msg, "ON") >= 0){
		SystemSetting.data.fuel_2_to_1 = 1;
	}else if(str_find(msg, "OFF") >= 0){
		SystemSetting.data.fuel_2_to_1 = 0;
	}
	write_number(FUEL_2_TO_1_PATH, (u64)SystemSetting.data.fuel_2_to_1);
}

static void SetDisRstDevWhenDisCam(u8* msg){
	if(str_find(msg, "OFF") >= 0){
		SystemSetting.data.dis_sys_rst_when_dis_cam = 1;
	}else if(str_find(msg, "ON") >= 0){
		SystemSetting.data.dis_sys_rst_when_dis_cam = 0;
	}
	write_number(DIS_RST_DEV_PATH, (u64)SystemSetting.data.dis_sys_rst_when_dis_cam);
}

static void SetDisableCapture(u8* msg){
	if(str_find(msg, "ON") >= 0){
		SystemSetting.data.disable_capture = 0;
	}else if(str_find(msg, "OFF") >= 0){
		SystemSetting.data.disable_capture = 1;
	}
	write_number(DIS_CAM_CAPTURE_PATH, (u64)SystemSetting.data.disable_capture);
}

static eNumCMDCode EnabeDecreaseSpeed(u8* msg){
	int val = atoi(msg);

	if((val == 0) || (val == 1)){
		DeviceSetting.Decrease_Speed = val;
		write_number(DECREASE_SPEED_PATH, (u64)DeviceSetting.Decrease_Speed);
		return CMD_DECREASE_SPEED;
	}
	return INVALID_MESSAGE;
}

static eNumCMDCode DisableWarningLXTN(u8* msg){
	int val = atoi(msg);

	if((val == 0) || (val == 1)){
		DeviceSetting.DisableWLXTN = val;
		write_number(DISABLE_WLXTN_PATH, (u64)DeviceSetting.DisableWLXTN);
		return CMD_DISABLE_WLXTN;
	}
	return INVALID_MESSAGE;
}

static eNumCMDCode DisableWarningLXTT(u8* msg){
	int val = atoi(msg);

	if((val == 0) || (val == 1)){
		DeviceSetting.DisableWLXTT = val;
		write_number(DISABLE_WLXTT_PATH, (u64)DeviceSetting.DisableWLXTT);
		return CMD_DISABLE_WLXTT;
	}
	return INVALID_MESSAGE;
}

static eNumCMDCode DisableWarningCAM(u8* msg){
	int val = atoi(msg);

	if((val == 0) || (val == 1)){
		DeviceSetting.DisableWCAM = val;
		write_number(DISABLE_WCAM_PATH, (u64)DeviceSetting.DisableWCAM);
		return CMD_DISABLE_WCAM;
	}
	return INVALID_MESSAGE;
}

static eNumCMDCode DisableWarningRFID(u8* msg){
	int val = atoi(msg);

	if((val == 0) || (val == 1)){
		DeviceSetting.DisableWRFID = val;
		write_number(DISABLE_WRFID_PATH, (u64)DeviceSetting.DisableWRFID);
		return CMD_DISABLE_WRFID;
	}
	return INVALID_MESSAGE;
}

static eNumCMDCode SetDomainPort(u8* msg){//ip_port
	u8 *p1=NULL;
	u8 domain[128];
	u32 port = 0;

    p1 = strstr(msg,"_");
    if(!p1) return INVALID_MESSAGE;

    if(strlen(msg) - strlen(p1) >= 64) return INVALID_MESSAGE;

    memset(domain, 0, 128);
    memcpy(domain, msg,(strlen(msg) - strlen(p1)));

    p1++;
    port = ec21_atoi(p1);

    if(!strstr(domain, ".") ||  (port == 0) || (port > 65535)){
    	return INVALID_MESSAGE;
    }

    if((strncmp(domain, SystemSetting.data.domain, strlen(SystemSetting.data.domain)) != 0) || (strlen(domain) != strlen(SystemSetting.data.domain)) || (port != SystemSetting.data.port)){
    	strim(domain);

    	strcpy(SystemSetting.data.domain, domain);
    	SystemSetting.data.port = port;

        /* store data */
    	write_string(DOMAIN_PATH, domain);
    	write_number(PORT_PATH, (u64)SystemSetting.data.port);

    	 /* recertify  */
    	if(strlen(SystemSetting.data.KeyActive) > 10){
			SystemSetting.data.certified = 0;
			write_number(CERTIFIED_PATH, (u64)SystemSetting.data.certified);
    	}
    	/* send config before reconnect */
    	send_device_setting();
    	/* reconnect */
    	tcp_reconnect(20);

        return CMD_IP_PORT;
    }
    return ALREADY_SETTING;
}

static eNumCMDCode SyncDriverData(u8* msg){
	int res = 0;
	u32 tmp_lxtn = 0, tmp_lxlt = 0, tmp_lxtt;
    u64 tmp_uid = 0;

    char_replace(msg, '_', ' ');

	res = sscanf(msg, "%llu %u %u %d", &tmp_uid, &tmp_lxlt, &tmp_lxtn, &tmp_lxtt);

	log_system("RES = %d \n", res);
	if(res == 4){

		log_system("RCV => UID: %llu, LXLT: %u, LXTN: %u, LXTT: %u\n", tmp_uid, tmp_lxlt, tmp_lxtn, tmp_lxtt);
		if(tmp_uid == tibase.UID){

			log_system("UPDATE NEW DRIVE RESULT -> TIBASE\n");

			tibase.bRunTime = (GPS_Data.timestamp - tmp_lxlt);
			tibase.DriverInday = tmp_lxtn;
			tibase.DriverInWeek = tmp_lxtt;
			sync_tibase();
			return CMD_DRIVER_RESULT;
		}

		return SETTING_FAILED;
	}
	return INVALID_MESSAGE;
}

static eNumCMDCode SetDeviceId(u8* msg){
	u64 deviceid = 0;

	//check max length
	if(strlen(msg) > 17) return INVALID_MESSAGE;

	deviceid = ec21_atoi(msg);

	//compare with old deviceid
	if(((SystemSetting.data.DeviceID == 0) && (deviceid != SystemSetting.data.DeviceID)) || (DeviceSetting.recover_active == true)){
		//if(GPS_Data.gps_status == 0) return SETTING_FAILED;

		if(SystemSetting.data.DeviceID == 0){
			memset(DeviceSetting.ManufactureDate, 0x0, 64);
			sprintf(DeviceSetting.ManufactureDate, "%04d/%02d/%02d", (GPS_Data.gps_time->tm_year + 1900), (GPS_Data.gps_time->tm_mon + 1), GPS_Data.gps_time->tm_mday);
			write_string(MANUFACTURE_DATE_PATH, DeviceSetting.ManufactureDate);//2018/01/25
		}

		/* setting new id and reconnect */
		SystemSetting.data.DeviceID = deviceid;
		write_number(DEVICE_ID_PATH, SystemSetting.data.DeviceID);
		tcp_reconnect(0);

		return CMD_DEVICEID;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetSpeedLimit(u8* msg){
	u64 Speed = 0;

	Speed = ec21_atoi(msg);

	if((Speed < 30) || (Speed > 200)) return INVALID_MESSAGE;

	//compare with old deviceid
	if(Speed != DeviceSetting.SpeedLimit){
		/* setting new id and reconnect */
		DeviceSetting.SpeedLimit = Speed;
		speed_limit_device = DeviceSetting.SpeedLimit;
		write_number(SPEED_LIMIT_PATH, (u64)DeviceSetting.SpeedLimit);

		return CMD_SPEED_LIMIT;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetPassword(u8* msg){
	s32 len = 0;
	len = strlen(msg);
	if(len > 30) return INVALID_MESSAGE;
	msg[len] = 0;

	strcpy(SystemSetting.data.PassWord, msg);
	write_string(PASSWORD_PATH, SystemSetting.data.PassWord);

	return CMD_PASSWORD;
}

static eNumCMDCode SetManufactureDate(u8* msg){//SET_11_2018/01/25
	s32 len = 0;
	len = strlen(msg);
	if((len != 10) || (msg[4] != '/') || (msg[7] != '/')) return INVALID_MESSAGE;
	memset(DeviceSetting.ManufactureDate, 0x0, 64);
	memcpy(DeviceSetting.ManufactureDate, msg, 10);
	write_string(MANUFACTURE_DATE_PATH, DeviceSetting.ManufactureDate);

	return CMD_MANUFACTURE_DATE;
}

static eNumCMDCode SetUpdateDate(u8* msg){//2018/01/01
	s32 len = 0;
	len = strlen(msg);
	if((len != 10) || (msg[4] != '/') || (msg[7] != '/')) return INVALID_MESSAGE;
	memset(DeviceSetting.DeviceUpdatedDate, 0x0, 64);
	memcpy(DeviceSetting.DeviceUpdatedDate, msg, 10);
	write_string(DEVICE_UPDATE_DATE_PATH, DeviceSetting.DeviceUpdatedDate);

	return CMD_FW_UPDATE_DATE;
}

static eNumCMDCode SetNumberPlate(u8* msg){
	s32 len = 0;
	len = strlen(msg);
	if((len < 5) || (len >= 32)) return INVALID_MESSAGE;
	msg[len] = 0;

	if((strlen(msg) != strlen(DeviceSetting.NumberPlate)) || (strncmp(msg, DeviceSetting.NumberPlate, strlen(DeviceSetting.NumberPlate)) != 0)){
		strcpy(DeviceSetting.NumberPlate, msg);
		write_string(NUMBER_PLATE_PATH, DeviceSetting.NumberPlate);
		setting_number_plate(DeviceSetting.NumberPlate);
		return CMD_NUMBER_PLATE;
	}
	setting_number_plate(DeviceSetting.NumberPlate);
	return ALREADY_SETTING;
}

static eNumCMDCode SetProvider(u8* msg){
	s32 len = 0;
	len = strlen(msg);
	if((len < 3) || (len >= 32)) return INVALID_MESSAGE;
	msg[len] = 0;

	if((strlen(msg) != strlen(SystemSetting.data.Provider)) || (strncmp(msg, SystemSetting.data.Provider, strlen(SystemSetting.data.Provider)) != 0)){
		strcpy(SystemSetting.data.Provider, msg);
		write_string(PROVIDER_PATH, SystemSetting.data.Provider);
		setting_provider(SystemSetting.data.Provider);
		return CMD_PROVIDER;
	}
	setting_provider(SystemSetting.data.Provider);
	return ALREADY_SETTING;
}

static eNumCMDCode SetCameraNumber(u8* msg){
	int val = atoi(msg);
	int i = 0;
	u8 tmp = 0;

	if(val > 8) return INVALID_MESSAGE;

	for(i = 0; i < val; i++){
		tmp |= (1 << i);
	}

	if(tmp != DeviceSetting.CameraNumber){
		DeviceSetting.CameraNumber = tmp;
		write_number(CAM_NUMBER_PATH, (u64)DeviceSetting.CameraNumber);

		return CMD_CAMERA_NUMBER;
	}
	return ALREADY_SETTING;
}

u8 GetCameraNumber(void){
	u8 i = 0;
	for(i = 8; i > 0; i--){
		if(((DeviceSetting.CameraNumber >> (i - 1)) & 0x01) == 1){
			return i;
		}
	}
}

static eNumCMDCode SetFrqCaptureRun(u8* msg){
	int val = atoi(msg);

	if((val < 5) || (val > 300)) return INVALID_MESSAGE;

	if(val != SystemSetting.data.FrqCaptureRun){
		SystemSetting.data.FrqCaptureRun = val;
		write_number(FRQ_CAPTURE_RUN_PATH, (u64)SystemSetting.data.FrqCaptureRun);

		return CMD_CAPTURE_RUN;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetTimeResetModule(u8* msg){
	int val = atoi(msg);

	if((val < 120) || (val > 7200)) return INVALID_MESSAGE;


	SystemSetting.data.time_reset_when_disconnect = val;
	write_number(TIME_RESET_MODULE_PATH, (u64)SystemSetting.data.time_reset_when_disconnect);

	return CMD_TIME_RESET_MODULE;
}

static eNumCMDCode SetFrqCaptureStop(u8* msg){
	int val = atoi(msg);

	if((val < 10) || (val > 14400)) return INVALID_MESSAGE;

	if(val != SystemSetting.data.FrqCaptureStop){
		SystemSetting.data.FrqCaptureStop = val;
		write_number(FRQ_CAPTURE_STOP_PATH, (u64)SystemSetting.data.FrqCaptureStop);

		return CMD_CAPTURE_STOP;
	}
	return ALREADY_SETTING;
}

static u8 fwd_PhoneNumber[16];
static u8 fwd_Content[64];
static eNumCMDCode FWD_SMS(u8* msg){//ip_port
	u8 *p1=NULL;

	memset(fwd_PhoneNumber, 0, 16);
	memset(fwd_Content, 0, 64);

    p1 = strstr(msg,"_");
    if(!p1) return INVALID_MESSAGE;

    if(strlen(msg) - strlen(p1) >= 16) return INVALID_MESSAGE;


    memcpy(fwd_PhoneNumber, msg,(strlen(msg) - strlen(p1)));

    p1++;
    strcpy(fwd_Content, p1);

    return CMD_FWD_SMS;
}

static eNumCMDCode Setting_RFID_ID(u8* msg, u8 type){
	u8 buf[16];
	u8 cs = 0;
	u32 ID = ec21_atoi(msg);
	buf[0] = '>';
	buf[1] = '>';
	buf[2] = 2;
	buf[3] = type;
	buf[4] = (ID >> 24) & 0xFF;;
	buf[5] = (ID >> 16) & 0xFF;
	buf[6] = (ID >> 8) & 0xFF;
	buf[7] = ID & 0xFF;
	cs = CRC_8BitsCompute(buf, 8);
	buf[8] = cs;
	rfid_send_msg(buf, 9);
}

static eNumCMDCode SetSpeakerVolume(u8* msg){
	u32 val = atoi(msg);

	if(val > 25) return INVALID_MESSAGE;

	if(val == SystemSetting.data.SpeakerVolume) return ALREADY_SETTING;

	/* store data */
	SystemSetting.data.SpeakerVolume = val;
	write_number(SPEAKER_VOLUME_PATH, (u64)SystemSetting.data.SpeakerVolume);
	return CMD_SPEAKER_VOLUME;
}

static eNumCMDCode SetBuzzerLevel(u8* msg){
	u32 val = atoi(msg);

	if(val > 6) return INVALID_MESSAGE;

	if(val == SystemSetting.data.BuzzerLevel) return ALREADY_SETTING;

	/* store data */
	SystemSetting.data.BuzzerLevel = val;
	write_number(BUZZER_LEVEL_PATH, (u64)SystemSetting.data.BuzzerLevel);
	return CMD_BUZZ_LEVEL;
}

static eNumCMDCode SetFilterLevel(u8* msg){
	u32 val = atoi(msg);

	if(val > 8) return INVALID_MESSAGE;

	if(val == SystemSetting.data.filter_level) return ALREADY_SETTING;

	/* store data */
	SystemSetting.data.filter_level = val;

	flg_full_load = FALSE;
	fuel_index = 0;

	write_number(FILTER_LEVEL_PATH, (u64)SystemSetting.data.filter_level);
	return CMD_FILTER_LEVEL;
}

static eNumCMDCode SetADThreshold_increase(u8* msg){
	u32 val = atoi(msg);

	if((val < 100) && (val > 4000)) return INVALID_MESSAGE;

	if(val == SystemSetting.data.ad_threshold_increase) return ALREADY_SETTING;

	/* store data */
	SystemSetting.data.ad_threshold_increase = val;

	write_number(AD_THRESHOLD_PATH, (u64)SystemSetting.data.ad_threshold_increase);
	return CMD_AD_THRESHOLD;
}

static eNumCMDCode SetAirRevert(u8* msg){
	u32 val = atoi(msg);

	if(val > 1) return INVALID_MESSAGE;

	if(val != SystemSetting.data.AirRevert){

		/* store data */
		SystemSetting.data.AirRevert = val;
		write_number(AIR_REVERT_PATH, (u64)SystemSetting.data.AirRevert);
		return CMD_AIR_REVERT;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetDoorRevert(u8* msg){
	u32 val = atoi(msg);

	if(val > 1) return INVALID_MESSAGE;

	if(val != SystemSetting.data.DoorRevert){

		/* store data */
		SystemSetting.data.DoorRevert = val;
		write_number(DOOR_REVERT_PATH, (u64)SystemSetting.data.DoorRevert);
		return CMD_DOOR_REVERT;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetFirmwareUrl(u8* msg){
	s32 len = 0;
	len = strlen(msg);
	if((len < 10) || (len >= MAX_SIZE_URL)) return INVALID_MESSAGE;

	if(str_find(msg, ".") <= 2) return INVALID_MESSAGE;

	msg[len] = 0;

	memset(SystemSetting.data.update_fw_url, 0x0, MAX_SIZE_URL);
	strcpy(SystemSetting.data.update_fw_url, msg);
	write_string(UPDATE_FW_URL_PATH, SystemSetting.data.update_fw_url);
	
	set_flg_ec21_update_fw();
	set_flg_mcu_update_fw();

	return CMD_FIRMWARE_URL;
}

static eNumCMDCode SetUploadFileUrl(u8* msg){
	s32 len = 0;
	len = strlen(msg);
	if((len < 10) || (len >= MAX_SIZE_URL)) return INVALID_MESSAGE;

	if(str_find(msg, ".") <= 2) return INVALID_MESSAGE;

	msg[len] = 0;

	memset(SystemSetting.data.upload_file_url, 0x0, MAX_SIZE_URL);
	strcpy(SystemSetting.data.upload_file_url, msg);
	write_string(UPLOAD_FILE_URL_PATH, SystemSetting.data.upload_file_url);

	return CMD_UPLOAD_URL;
}

static eNumCMDCode SetUploadDbUrl(u8* msg){
	s32 len = 0;
	len = strlen(msg);
	if((len < 10) || (len >= MAX_SIZE_URL)) return INVALID_MESSAGE;

	if(str_find(msg, ".") <= 2) return INVALID_MESSAGE;

	msg[len] = 0;

	memset(DeviceSetting.upload_db_url, 0x0, MAX_SIZE_URL);
	strcpy(DeviceSetting.upload_db_url, msg);
	write_string(UPLOAD_DB_URL_PATH, DeviceSetting.upload_db_url);

	return CMD_UPLOAD_DB_URL;
}

static eNumCMDCode SetFrequencyRun(u8* msg){
	u32 val = atoi(msg);

	if(val != SystemSetting.data.FrqRun){
		if(val <= FREQ_RUN){
			/* store data */
			SystemSetting.data.FrqRun = val;
			write_number(FRQ_RUN_PATH, (u64)SystemSetting.data.FrqRun);
			return CMD_FREQRUN;
		}
		return INVALID_MESSAGE;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetFrequencyStop(u8* msg){
	u32 val = atoi(msg);
	if(val != SystemSetting.data.FrqStop){
		if((val < FREQ_RUN) || (val > 3600)) val = FREQ_STOP;
		/* store data */
		SystemSetting.data.FrqStop = val;
		write_number(FRQ_STOP_PATH, (u64)SystemSetting.data.FrqStop);
		return CMD_FREQSTOP;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetACCMode(u8* msg){
	u32 val = atoi(msg);
	if(val > NO_ACC_LSM_NO_SPEED) return INVALID_MESSAGE;

	if((enum_AccMode)val != SystemSetting.data.AccMode){
		/* store data */
		SystemSetting.data.AccMode = (enum_AccMode)val;
		write_number(ACC_MODE_PATH, (u64)SystemSetting.data.AccMode);
		return CMD_ACC_MODE;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetFuelManual(u8* msg){
	u32 val = atoi(msg);

	//compare with old value
	if(val != DeviceSetting.FuelManualMode){
		DeviceSetting.FuelManualMode = val;
		write_number(FUEL_MANUAL_PATH, (u64)DeviceSetting.FuelManualMode);

		return CMD_FUEL_MANUAL_CFG;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetDrvForeWarningTime(u8* msg){
	u32 val = atoi(msg);
	if((val < TIME_5_MINUTES) || (val > TIME_1_HOUR)) return INVALID_MESSAGE;
	//compare with old value
	if(val != SystemSetting.data.DrvForeWarningTime){
		SystemSetting.data.DrvForeWarningTime = val;
		write_number(DRV_FORE_WARN_TIME_PATH, (u64)SystemSetting.data.DrvForeWarningTime);

		return CMD_DRV_FORCE_CTN_TIME;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetDrvStopTime(u8* msg){
	u32 val = atoi(msg);

	if((val < TIME_1_MINUTES) || (val > TIME_15_MINUTES)) return INVALID_MESSAGE;

	//compare with old value
	if(val != SystemSetting.data.DrvStopTime){
		SystemSetting.data.DrvStopTime = val;
		write_number(DRV_STOP_TIME_PATH, (u64)SystemSetting.data.DrvStopTime);

		return CMD_DRV_STOP_TIME;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetBaud2(u8* msg){
	u32 val = atoi(msg);

	//compare with old value
	if(val != DeviceSetting.BauRate2){
		DeviceSetting.BauRate2 = val;
		write_number(BAUD2_PATH, (u64)DeviceSetting.BauRate2);

		return CMD_BAUD2_CFG;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetBaud4(u8* msg){
	u32 val = atoi(msg);

	//compare with old value
	if(val != DeviceSetting.BauRate4){
		DeviceSetting.BauRate4 = val;
		write_number(BAUD4_PATH, (u64)DeviceSetting.BauRate4);

		return CMD_BAUD4_CFG;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode RequestBackup(u8* msg){
	u8 *p1=NULL;
	u8 start_time[64];

    p1 = strstr(msg,"_");
    if(!p1) return INVALID_MESSAGE;

    if(strlen(msg) - strlen(p1) >= 15) return INVALID_MESSAGE;
    memset(start_time, 0x0, 64);
    memcpy(start_time, msg,(strlen(msg) - strlen(p1)));

    p1++;

    log_info("START TIME -> %s, END TIME -> %s \n", start_time, p1);

    set_backup_request(start_time, p1);

    return CMD_REQUEST_BACKUP;
}

static eNumCMDCode SetDriverInfo(u8* msg){//@@SET_26_'UID', 'NAME', 'LICENSE', 'IISDATE', 'EXPDATE', 'sync'
	if(char_index_of(msg, '\'', 10) < 0){
		return INVALID_MESSAGE;
	}

	if(char_index_of(msg, ',', 4) < 0){
		return INVALID_MESSAGE;
	}

	if(!update_driver_info_by_server(msg)){
		load_driver_info_by_uid(tibase.UID);
		//send driver info to MCU
		log_system("DRIVER INFO UPDATE 5\n");
		update_rfid_driver_data();

		return CMD_UPDATE_DRIVER_INFO;
	}
	return SETTING_FAILED;
}

void get_device_info(char* src){
	sprintf(src, "BK10-%d-%d-%d-%s\n%llu\n%s\n%s:%d\nGPS:%d, SVS: %d-%d\nTCP:%d, MODE:%d, STR:%d\nACC:%d-%d, D:%d, A:%d\nFRQ: %d-%d, MSP:%d\nV %d-%d-%d", APP_VERSION, MCU_Data.MCU_Verison, MCU_Data.RFID_Verison, RELEASE_DATE_TIME, SystemSetting.data.DeviceID, DeviceSetting.NumberPlate,
			SystemSetting.data.domain, SystemSetting.data.port, GPS_Data.gps_status, GPS_Data.svs_in_view, GPS_Data.svs_in_use, (u8)network_data.tcp_status, network_data.net_work_info.radio_tech, network_data.net_work_info.signal_strength, MCU_Data.Hard_Acc, LSM_ACC, MCU_Data.Hard_Door, MCU_Data.Hard_Air, SystemSetting.data.FrqRun,
			SystemSetting.data.FrqStop, speed_limit_device,MCU_Data.main_power, MCU_Data.Vout_power, MCU_Data.bat_power);
}

static void get_full_device_info(char* src){
	sprintf(src, "Device Type: BK10\nEC21: %d\nMCU: %d\nRelease: %s\nID: %llu\nPlate: %s\nIP/PORT: %s:%d\nTCP: %d, MODE: %d, STR: %d\nGPS: %d, SVS:%d-%d\nACC Hard: %d, ACC Sensor: %d, Door: %d, Air: %d\nFRQ Run-Stop: %d-%d, MaxSpeed: %d\nVin: %d, Vout: %d, Vbat: %d\nICCID: %s",
		APP_VERSION, MCU_Data.MCU_Verison, RELEASE_DATE_TIME, SystemSetting.data.DeviceID, DeviceSetting.NumberPlate,
		SystemSetting.data.domain, SystemSetting.data.port, (u8)network_data.tcp_status, network_data.net_work_info.radio_tech, network_data.net_work_info.signal_strength,
		GPS_Data.gps_status, GPS_Data.svs_in_view, GPS_Data.svs_in_use, MCU_Data.Hard_Acc, LSM_ACC, MCU_Data.Hard_Door, MCU_Data.Hard_Air,
		SystemSetting.data.FrqRun, SystemSetting.data.FrqStop, speed_limit_device,
		MCU_Data.main_power, MCU_Data.Vout_power, MCU_Data.bat_power, DeviceSetting.ICCID);
}

void get_location(char* src){
	sprintf(src, "BK10 V-%d, DeviceID = %llu\nhttps://www.google.com/maps?q=%.5f,%.5f\nTime: %02d:%02d:%02d %02d-%02d-%d\nSpeed: %.2f\n%s\0",
			APP_VERSION, SystemSetting.data.DeviceID, GPS_Data.location.latitude, GPS_Data.location.longitude,GPS_Data.gps_time->tm_hour, GPS_Data.gps_time->tm_min, GPS_Data.gps_time->tm_sec, GPS_Data.gps_time->tm_mday, (GPS_Data.gps_time->tm_mon + 1), (GPS_Data.gps_time->tm_year + 1900), GPS_Data.gps_speed, DeviceSetting.IMEI);
}

eNumCMDCode ResetDevice(char *msg, SETTING_def Type){
	uint32_t timeout = atoi(msg);

	if(timeout == 1){
		sync();
		sleep(3);
		mcu_send_with_cs(">BATOFF", 7);
		return CMD_DEVICE_PW_OFF;
	}

	if(Type == SMS_SET){
		if((timeout < 30) || (timeout > 240)) timeout = 30;
	}

	reboot_camera();
	set_device_reset_timeout(timeout);

	return CMD_DEVICE_RESET;
}

static void SetAllowOffCamPark(u8* msg){
	if(str_find(msg, "OFF") >= 0){
		SystemSetting.data.disable_pw_off_cam_when_park = 0;
	}else if(str_find(msg, "ON") >= 0){
		SystemSetting.data.disable_pw_off_cam_when_park = 1;
	}
	write_number(DIS_OFF_CAM_PARK_PATH, (u64)SystemSetting.data.disable_pw_off_cam_when_park);
}

static eNumCMDCode SetTimeOffCamPark(u8* msg){
	u64 time_ = ec21_atoi(msg);

	/* check value */
	if((time_ < 120) || (time_ > 10800)) return INVALID_MESSAGE;

	//compare with old deviceid
	if(time_ != SystemSetting.data.time_pw_off_camera_when_park){
		/* setting new id and reconnect */
		SystemSetting.data.time_pw_off_camera_when_park = time_;
		write_number(TIME_OFF_CAM_PARK_PATH, (u64)SystemSetting.data.time_pw_off_camera_when_park);

		return CMD_TIME_OFF_CAM_PARK;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode set_phonenumber(u8* phone, u8 pos){
	if((!check_phone_valid(phone)) || (strlen(phone) > 15)){
		return INVALID_MESSAGE;
	}
	switch(pos){
		case 1:
			memset(SystemSetting.data.Phone1, 0x0, MAX_SIZE_PHONE_NUMBER);
			strcpy(SystemSetting.data.Phone1 ,phone);
			write_string(PHONE_1_PATH, SystemSetting.data.Phone1);
			return CMD_PHONE1;
		case 2:
			memset(SystemSetting.data.Phone2, 0x0, MAX_SIZE_PHONE_NUMBER);
			strcpy(SystemSetting.data.Phone2 ,phone);
			write_string(PHONE_2_PATH, SystemSetting.data.Phone2);
			return CMD_PHONE2;
		case 3:
			memset(SystemSetting.data.Phone_sos, 0x0, MAX_SIZE_PHONE_NUMBER);
			strcpy(SystemSetting.data.Phone_sos ,phone);
			write_string(PHONE_SOS_PATH, SystemSetting.data.Phone_sos);
			return CMD_PHONE_SOS;
	}
	return INVALID_MESSAGE;
}

static eNumCMDCode SetFuelType(u8* msg){
	u32 val = atoi(msg);
	if(val != SystemSetting.data.AdcPulseCfg){
		if(val > PULSE_MODE) return INVALID_MESSAGE;
		/* store data */
		SystemSetting.data.AdcPulseCfg = (Fuel_type)val;
		write_number(ADC_PULSE_PATH, (u64)SystemSetting.data.AdcPulseCfg);
		return CMD_FUEL_TYPE;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetOBDType(u8* msg){
	u32 val = atoi(msg);
	if(val != SystemSetting.data.OBDType){
		if(val > J1587_TYPE) return INVALID_MESSAGE;
		/* store data */
		SystemSetting.data.OBDType = (enum_OBD_type)val;
		write_number(OBD_TYPE_PATH, (u64)SystemSetting.data.OBDType);
		set_device_reset_timeout(30);
		return CMD_OBD_TYPE;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode SetMotionValue(u8* msg){
	float val = atof(msg);
	if(val != SystemSetting.data.MotionValue){
		if((val <= 0) || (val > 180)) return INVALID_MESSAGE;

		SystemSetting.data.MotionValue = val;

		write_string(MOTION_VALUE_PATH, msg);

		return CMD_MOTION_VALUE;
	}
	return ALREADY_SETTING;
}

static eNumCMDCode change_ip_cam(char *src){
	uint32_t from, to;

	if(char_index_of(src, ' ', 1) < 0) return INVALID_MESSAGE;

	sscanf(src, "%u %u", &from, &to);

	if((from >= 1) && (from <= 8) && (to >= 1) && (to <= 8)){
		change_ip_camera(from, to);
		DeviceSetting.camera_list[from - 1].detect = false;
		memset(DeviceSetting.camera_list[from - 1].camera_sn, 0x0, 32);
		GPS_Data.cam_up_time = 0;
		return CMD_CHANGE_IP_CAMERA;
	}
	return INVALID_MESSAGE;
}

void get_lan_status(char *text){
	struct ql_sgmii_info info = { 0 };
	if(ql_smgii_info_get(&info)) {
		printf("\nget sgmii info failure");
	}

	sprintf(text, "tx packets %lu, tx bytes %lu \nrx packets %lu, rx bytes %lu", info.tx_pkts, info.tx_bytes, info.rx_pkts, info.rx_bytes);
}

float LAT = 0, LONG = 0;
float speed_test = 0;
static int mcu_cmd_response = -1;
static char text[512];
//static char buffer[256];
int delete_trip_res = -2;
eNumCMDCode device_setting(u8* msg, SETTING_def type){//SET_04_121212
	eNumCMDCode cmd_code = 0;
	u8 *p1 = NULL;

    p1 = strstr(msg,"_");
    if(!p1) return 0;

    p1++;
	cmd_code = (eNumCMDCode)(atoi(msg));
	memset(text, 0x0, sizeof(text));
	switch(cmd_code){
		case CMD_ACTIVE_KEY://0
			if((DeviceSetting.recover_active == true) || (type == SERVER_SET)){
				cmd_code = SetKeyActive(p1);
			}else{
				cmd_code = DONOT_PERMISSION;
			}
			break;
		case CMD_RUNLEVEL://1
			cmd_code = SetRunLevel(p1);
			break;
		case CMD_SLIENCE://2
			cmd_code = SetSlienceMode(p1);
			break;
		case CMD_IP_PORT://3
			if(type == PC_SET){
				if(DeviceSetting.com_enset == false){
					return DONOT_PERMISSION;
				}
			}else if(type == SMS_SET){
				if((DeviceSetting.recover_active == false) && (DeviceSetting.sms_enset == false)){
					return DONOT_PERMISSION;
				}
			}
			cmd_code = SetDomainPort(p1);
			break;
		case CMD_DEVICEID://4
			cmd_code = SetDeviceId(p1);
			break;
		case CMD_PASSWORD://5
			if(type == PC_SET){
				if(DeviceSetting.com_enset == false){
					return DONOT_PERMISSION;
				}
			}else if(type == SMS_SET){
				if((DeviceSetting.recover_active == false) && (DeviceSetting.sms_enset == false)){
					return DONOT_PERMISSION;
				}
			}
			cmd_code = SetPassword(p1);
			break;
		case CMD_SPEED_LIMIT://6
			cmd_code = SetSpeedLimit(p1);
			break;
		case CMD_FREQRUN://7
			cmd_code = SetFrequencyRun(p1);
			break;
		case CMD_FREQSTOP://8
			cmd_code = SetFrequencyStop(p1);
			break;
		case CMD_REQUEST_BACKUP://9
			cmd_code = RequestBackup(p1);
			break;
		case CMD_DEVICE_PW_OFF://10
			break;
		case CMD_MANUFACTURE_DATE://11
			cmd_code = SetManufactureDate(p1);
			break;
		case CMD_FW_UPDATE_DATE://12
			cmd_code = SetUpdateDate(p1);
			break;
		case CMD_SEND_DEVICE_INFO://13
			GPS_Data.flg_send_cfg = 1;
			GPS_Data.time_to_send_evt = 120;
			return CMD_SEND_DEVICE_INFO;
		case CMD_DEV_INFO://14
			if(type != SMS_SET){
				get_device_info(text);
				Send_SMS(p1, text);
			}
			break;
		case CMD_DEVICE_RESET://15
			cmd_code = ResetDevice(p1, type);
			break;
		case CMD_CHECK_UPDATE_FW://16
			set_flg_ec21_update_fw();
			set_flg_mcu_update_fw();
			break;
		case CMD_SPEAKER_VOLUME://17
			cmd_code = SetSpeakerVolume(p1);
			break;
		case CMD_CAR_TYPE://18
			break;
		case CMD_CHANGE_IP_CAMERA://19
			cmd_code = change_ip_cam(p1);
			break;
		case CMD_GET_CAMERA_LIST://20
			break;
		case CMD_CLEAR_BACKUP://21
			clear_flg_send(0);
			break;
		case CMD_BUZZ_LEVEL://22
			cmd_code = SetBuzzerLevel(p1);
			break;
		case CMD_DIS_CAM_ROAMING://23
			SetDisableCamRoaming(p1);
			break;
		case CMD_AIR_REVERT://24
			cmd_code = SetAirRevert(p1);
			break;
		case CMD_DOOR_REVERT://25
			cmd_code = SetDoorRevert(p1);
			break;
		case CMD_UPDATE_DRIVER_INFO://26
			cmd_code = SetDriverInfo(p1);
			break;
		case CMD_PHONE1://27
			if(type == PC_SET){
				if(DeviceSetting.com_enset == false){
					return DONOT_PERMISSION;
				}
			}else if(type == SMS_SET){
				if((DeviceSetting.recover_active == false) && (DeviceSetting.sms_enset == false)){
					cmd_code = DONOT_PERMISSION;
					break;
				}
			}
			cmd_code = set_phonenumber(p1, 1);
			break;
		case CMD_PHONE2://28
			if(type == PC_SET){
				if(DeviceSetting.com_enset == false){
					return DONOT_PERMISSION;
				}
			}else if(type == SMS_SET){
				if((DeviceSetting.recover_active == false) && (DeviceSetting.sms_enset == false)){
					cmd_code = DONOT_PERMISSION;
					break;
				}
			}
			cmd_code = set_phonenumber(p1, 2);
			break;
		case CMD_PHONE_SOS://29
			cmd_code = set_phonenumber(p1, 3);
			break;
		case CMD_MOTION_VALUE://30
			cmd_code = SetMotionValue(p1);
			break;
		case CMD_NUMBER_PLATE://31
			cmd_code = SetNumberPlate(p1);
			break;
		case CMD_32:{//32
				u8 vol = ec21_atoi(p1);
				add_play_list("/usrdata/sound/", "startup.wav", vol);//64
				break;
			}
		case CMD_TIME_RESET_MODULE:
			cmd_code = SetTimeResetModule(p1);
			break;
		case CMD_GET_LAN_STATUS://34
			break;
		case CMD_CAMERA_NUMBER://35
			cmd_code = SetCameraNumber(p1);
			break;
		case CMD_CAPTURE_RUN://36
			cmd_code = SetFrqCaptureRun(p1);
			break;
		case CMD_CAPTURE_STOP://37
			cmd_code = SetFrqCaptureStop(p1);
			break;
		case CMD_RFID_ID_SET://38
			Setting_RFID_ID(p1, 1);
			cmd_code = NOT_RESPONSE_CMD;
			break;
		case CMD_FILTER_LEVEL://39
			cmd_code = SetFilterLevel(p1);
			break;
		case CMD_ACC_MODE://40
			cmd_code = SetACCMode(p1);
			break;
		case CMD_FUEL_TYPE://41
			cmd_code = SetFuelType(p1);
			break;
		case CMD_MCU_FORWARD:{//42
			u8 mcu_response[64];
			u16 response_len = 0;
			mcu_cmd_response = -1;
			memset(mcu_response, 0x0, 64);
			if(mcu_send_and_rcv(p1, strlen(p1), mcu_response, &response_len, 5000) == 0){
				if(!strncmp(mcu_response, "OK", 2)){
					mcu_cmd_response = 0;
				}
			}
			break;
		}
		case CMD_GET_DB://43
			upload_database();
			break;
		case CMD_BAUD2_CFG://44
			cmd_code = SetBaud2(p1);
			break;
		case CMD_BAUD4_CFG://45
			cmd_code = SetBaud4(p1);
			break;
		case CMD_FUEL_MANUAL_CFG://46
			cmd_code = SetFuelManual(p1);
			break;
		case CMD_DELETE_TRIP_INFO://47
			delete_trip_res = delete_all_tripinfo();
			break;
		case CMD_DRV_STOP_TIME://48
			cmd_code = SetDrvStopTime(p1);
			break;
		case CMD_FACTORY://49
			if(type == PC_SET){
				if(DeviceSetting.com_enset == false){
					return DONOT_PERMISSION;
				}
			}else if(type == SMS_SET){
				if((DeviceSetting.recover_active == false) && (DeviceSetting.sms_enset == false)){
					return DONOT_PERMISSION;
				}
			}
			system("rm -rf /usr/data/*.dat");
			sleep(1);
			sync();
			sleep(1);
			set_device_reset_timeout(30);
			break;
		case CMD_DRV_FORCE_CTN_TIME://51
			cmd_code = SetDrvForeWarningTime(p1);
			break;
		case CMD_DRIVER_INFO://52
			break;
		case CMD_CLEAR_DRIVER_DATA:{//53
				u32 param = atoi(p1);
				if(param == 0){
					clear_driver_result(1, 0);
					log_system("xoa du lieu trong ngay lai xe hien tai");
				}if(param == 1){
					/* clear current DATA */
					clear_driver_result(1, 1);
					log_system("xoa du lieu ca tuan lai xe hien tai");
				}if(param == 2){
					clear_driver_result(1, 0);
					clear_summary_data(1, 0);
					log_system("xoa du lieu theo ngay toan bo lai xe");
				}else if(param == 3){
					clear_driver_result(0, 1);
					clear_summary_data(0, 1);
					log_system("xoa du lieu theo tuan toan bo lai xe");
				}else if(param == 4){
					clear_driver_result(1, 1);
					clear_summary_data(1, 1);
					log_system("xoa toan bo du lieu ngay/tuan cua toan bo lai xe");
				}
				break;
			}
		case CMD_CAM_CFG://56,
			break;
		case CMD_OILD_CHANGE_NOTIFY:{//57
				u8 vol = ec21_atoi(p1);
				add_play_list("/usrdata/sound/", "oilchange.wav", vol);//64
				break;
			}
		case CMD_FEE_NOTIFY:{//58
				u8 vol = ec21_atoi(p1);
				add_play_list("/usrdata/sound/", "fee.wav", vol);//64
				break;
			}
		case CMD_VEHICLE_CHECK_NOTIFY:{//59
				u8 vol = ec21_atoi(p1);
				add_play_list("/usrdata/sound/", "checkvehicle.wav", vol);//64
				break;
			}
		case CMD_GET_OBD_INFO://60
			break;
		case CMD_FWD_SMS://61
			cmd_code = FWD_SMS(p1);
			break;
		case CMD_OBD_TYPE://62
			cmd_code = SetOBDType(p1);
			break;
		case CMD_FIRMWARE_URL:{//63
			if(type == PC_SET){
				if(DeviceSetting.com_enset == false){
					return NOT_RESPONSE_CMD;
				}
			}else if(type == SMS_SET){
				if((DeviceSetting.recover_active == false) && (DeviceSetting.sms_enset == false)){
					return NOT_RESPONSE_CMD;
				}
			}
			cmd_code = SetFirmwareUrl(p1);
			break;
		}
		case CMD_UPLOAD_URL://64
			cmd_code = SetUploadFileUrl(p1);
			break;
		case CMD_DRIVER_LOGIN:{//65
			u64 UID = ec21_atoi(p1);

			sv_lg_buff[0] = UID & 0xFF;
			sv_lg_buff[1] = (UID >> 8)  & 0xFF;
			sv_lg_buff[2] = (UID >> 16) & 0xFF;
			sv_lg_buff[3] = (UID >> 24) & 0xFF;
			sv_lg_buff[4] = (UID >> 32) & 0xFF;
			sv_lg_buff[5] = (UID >> 40) & 0xFF;
			sv_lg_buff[6] = (UID >> 48) & 0xFF;
			sv_lg_buff[7] = (UID >> 56) & 0xFF;
			flg_sv_lg = 1;

			cmd_code = CMD_DRIVER_INFO;

			break;
		}
		case CMD_AD_THRESHOLD://66
			cmd_code = SetADThreshold_increase(p1);
			break;
		case DIS_RST_SYS_CAM_CMD://68
			SetDisRstDevWhenDisCam(p1);
			break;
		case CMD_ALLOW_OFF_CAM_PARK://69
			SetAllowOffCamPark(p1);
			break;
		case CMD_TIME_OFF_CAM_PARK://70
			cmd_code = SetTimeOffCamPark(p1);
			break;
		case CMD_PING_CAMERA://71
			break;
		case CMD_DRIVER_LOGOUT://75
			flg_sv_lg = 2;
			break;
		case CMD_PROVIDER://76
			cmd_code = SetProvider(p1);
			break;
		case CMD_UPLOAD_DB_URL://77
			cmd_code = SetUploadDbUrl(p1);
			break;
		case CMD_DECREASE_SPEED://78
			cmd_code = EnabeDecreaseSpeed(p1);
			break;
		case CMD_DISABLE_WLXTN:// 79
			cmd_code = DisableWarningLXTN(p1);
			break;
		case CMD_DISABLE_WLXTT:// 80
			cmd_code = DisableWarningLXTT(p1);
			break;
		case CMD_DISABLE_WCAM:// 81
			cmd_code = DisableWarningCAM(p1);
			break;
		case CMD_DISABLE_WRFID:// 82
			cmd_code = DisableWarningRFID(p1);
			break;
		case CMD_DISABLE_CAPTURE://87
			SetDisableCapture(p1);
			break;
		case CMD_GET_DATA_MONITOR://88
			if(str_find(p1, "GPS") >= 0){
				send_realtime_data();
			}else if(str_find(p1, "CAM") >= 0){
				GPS_Data.flg_capture = true;
			}else if(str_find(p1, "CFG") >= 0){
				send_device_setting();
			}
			break;
		case CMD_RESET_CAMERA:{//92
			reboot_camera();
			lan_port_init();
			GPS_Data.cam_up_time = 0;
			break;
		}
		case CMD_CAMERA_DISABLE://93
			SetDisableCam(p1);
			break;
		case CMD_AUDIO_DISABLE://94
			SetDisableAudio(p1);
			break;
		case FUEL_3_TO_2_CMD://101
			SetFuel3_to_2(p1);
			break;
		case FUEL_3_TO_1_CMD://102
			SetFuel3_to_1(p1);
			break;
		case FUEL_2_TO_1_CMD://103
			SetFuel2_to_1(p1);
			break;
		case 104:
			//LAT = atof(p1);
			break;
		case 105:
			//LONG = atof(p1);
			break;
		case 106:
			//speed_test = atof(p1);
			break;
		case 107:
			GPS_Data.fact_time = ec21_atoi(p1);
			break;
		case SYNC_DRIVER_DATA://@@SET_222_UID_TGLXLT_TGLXTN_TGLXTT
			cmd_code = SyncDriverData(p1);
			break;
		default:break;
	}
	GPS_Data.flg_send_cfg = 1;
	GPS_Data.time_to_send_evt = 118;
	transfer_setting_mcu = 0;
	sync();
	return cmd_code;
}

char* get_device_setting(eNumCMDCode cmd, SETTING_def type){
	u64 number = 0;
	memset(text, 0x0, sizeof(text));
	//memset(buffer, 0x0, sizeof(buffer));
	switch(cmd){
		case CMD_ACTIVE_KEY:{//0
			if(DeviceSetting.recover_active == true){
				sprintf(text, "Certificate key : %s", SystemSetting.data.KeyActive);
				return text;
			}
			return "Ban khong co quyen doc thong tin nay";
		}
		case CMD_RUNLEVEL://1
			sprintf(text, "Toc do phat hien di chuyen la : %d km/h", SystemSetting.data.RunLevel);
			return text;
		case CMD_SLIENCE://2
			sprintf(text, "Che do im lang : %s", (SystemSetting.data.SlienceMode == 1) ? "Bat" : "Tat");
			return text;
		case CMD_IP_PORT://3
			sprintf(text, "Dia chi IP/PORT : %s:%d", SystemSetting.data.domain, SystemSetting.data.port);
			return text;
		case CMD_DEVICEID://4
			sprintf(text, "Ma thiet bi duoc cai dat : %llu", SystemSetting.data.DeviceID);
			return text;
		case CMD_PASSWORD://5
			if(type == PC_SET){
				if(DeviceSetting.com_enset == false){
					return "Ban khong co quyen thuc hien chuc nang nay";
				}
			}
			sprintf(text, "Mat khau cai dat thiet bi : %s", SystemSetting.data.PassWord);
			return text;
		case CMD_SPEED_LIMIT://6
			sprintf(text, "Toc do gioi han : %d", speed_limit_device);
			return text;
		case CMD_FREQRUN://7
			sprintf(text, "Thoi gian gui du lieu khi di chuyen : %d", SystemSetting.data.FrqRun);
			return text;
		case CMD_FREQSTOP://8
			sprintf(text, "Thoi gian gui du lieu khi dung/do : %d", SystemSetting.data.FrqStop);
			return text;
		case CMD_REQUEST_BACKUP://9
			return "Da nhan yeu cau gui lai du lieu";
		case CMD_DEVICE_PW_OFF://10
			return "TAT THIET BI";
		case CMD_MANUFACTURE_DATE://11
			sprintf(text, "Ngay xuat xuong thiet bi : %s", DeviceSetting.ManufactureDate);
			return text;
		case CMD_FW_UPDATE_DATE://12
			sprintf(text, "Ngay cap nhat thiet bi : %s", DeviceSetting.DeviceUpdatedDate);
			return text;
		case CMD_SEND_DEVICE_INFO://13
			return "Da gui ban tin khai bao cau hinh thiet bi";
		case CMD_DEV_INFO://14
			if(type == PC_SET){
				get_full_device_info(text);
			}else{
				get_device_info(text);
			}
			return text;
		case CMD_DEVICE_RESET://15
			sprintf(text, "Thiet bi se khoi dong lai sau %d (giay)", get_device_reset_timeout());
			return text;
		case CMD_CHECK_UPDATE_FW://16
			return "Check new firmware version...";
		case CMD_SPEAKER_VOLUME://17
			sprintf(text, "Am luong loa ngoai duoc cai dat la : %d", SystemSetting.data.SpeakerVolume);
			return text;
		case CMD_CAR_TYPE://18
			break;
		case CMD_CHANGE_IP_CAMERA://19
			return "Da gui lenh chuyen IP camera";
		case CMD_GET_CAMERA_LIST://20
			get_list_camera_index(text);
			return text;
		case CMD_CLEAR_BACKUP://21
			return "Da xoa du lieu backup";
		case CMD_BUZZ_LEVEL://22
			sprintf(text, "Am luong coi duoc cai dat la : %d", SystemSetting.data.BuzzerLevel);
			return text;
		case CMD_DIS_CAM_ROAMING://23
			sprintf(text, "Chup anh khi roaming : %s", ((SystemSetting.data.disable_camera_roaming_mode == 1) ? "Khong" : "Co"));
			return text;
		case CMD_AIR_REVERT://24
			sprintf(text, "Dao trang thai dieu hoa : %s", (SystemSetting.data.AirRevert == 1) ? "Co" : "Khong");
			return text;
		case CMD_DOOR_REVERT://25
			sprintf(text, "Dao trang thai cua : %s", (SystemSetting.data.DoorRevert == 1) ? "Co" : "Khong");
			return text;
		case CMD_UPDATE_DRIVER_INFO://26
			return "Cap nhat thong tin lai xe thanh cong";
		case CMD_PHONE1://27
			sprintf(text, "So dien thoai 1 : %s", SystemSetting.data.Phone1);
			return text;
		case CMD_PHONE2://28
			sprintf(text, "So dien thoai 2 : %s", SystemSetting.data.Phone2);
			return text;
		case CMD_PHONE_SOS://29
			sprintf(text, "So dien thoai SOS : %s", SystemSetting.data.Phone_sos);
			return text;
		case CMD_MOTION_VALUE://30
			sprintf(text, "Gia tri rung : %.2f", SystemSetting.data.MotionValue);
			return text;
		case CMD_NUMBER_PLATE://31
			sprintf(text, "Bien so xe : %s", DeviceSetting.NumberPlate);
			return text;
		case CMD_32://32
			return "Test Audio...";
		case CMD_TIME_RESET_MODULE://33
			sprintf(text, "Thoi gian khoi dong khi mat ket noi may chu : %d", SystemSetting.data.time_reset_when_disconnect);
			return text;
		case CMD_GET_LAN_STATUS://34
			get_lan_status(text);
			return text;
		case CMD_CAMERA_NUMBER://35
			sprintf(text, "So camera : %d", GetCameraNumber());//DeviceSetting.CameraNumber);
			return text;
		case CMD_CAPTURE_RUN://36
			sprintf(text, "Thoi gian chup anh khi chay : %d", SystemSetting.data.FrqCaptureRun);
			return text;
		case CMD_CAPTURE_STOP://37
			sprintf(text, "Thoi gian chup anh khi dung : %d", SystemSetting.data.FrqCaptureStop);
			return text;
		case CMD_RFID_ID_SET://38
			Setting_RFID_ID("0", 0);
			return "";
		case CMD_FILTER_LEVEL://39
			sprintf(text, "Filter time level : %d", SystemSetting.data.filter_level);
			return text;
		case CMD_ACC_MODE:{//40
			switch(SystemSetting.data.AccMode){
				case ACC_LSM:
					return "Che do su dung day ACC + CAM BIEN";
				case ACC_ONLY:
					return "Che do chi su dung day ACC";
				case LSM_ONLY:
					return "Che do chi su dung CAM BIEN";
				case NO_ACC_LSM_SPEED:
					return "Che do KHONG su dung day ACC + CAM BIEN (su dung van toc)";
				case NO_ACC_LSM_NO_SPEED:
					return "Che do ACC luon BAT";
				default: break;
			}
			break;
		}
		case CMD_FUEL_TYPE:{//41
			switch(SystemSetting.data.AdcPulseCfg){
				case CYCLE_MODE:
					return "Che do dau do rong xung";
				case ADC_MODE:
					return "Che do dau Analog";
				case PULSE_MODE:
					return "Che do dau xung (Hz)";
				default: break;
			}
			break;
		}
		case CMD_MCU_FORWARD://42
			sprintf(text, "Gui lenh %s", ((mcu_cmd_response == 0) ? "thanh cong" : "that bai"));
			return text;
		case CMD_GET_DB://43
			return "Da upload database len he thong!!!";
		case CMD_BAUD2_CFG://44
			sprintf(text, "Baudrate 2 : %d", DeviceSetting.BauRate2);
			return text;
		case CMD_BAUD4_CFG://45
			sprintf(text, "Baudrate 1 : %d", DeviceSetting.BauRate4);
			return text;
		case CMD_FUEL_MANUAL_CFG://46
			sprintf(text, "Che do dau thu cong : %d", DeviceSetting.FuelManualMode);
			return text;
		case CMD_DELETE_TRIP_INFO://47
			if(delete_trip_res == 0){
				delete_trip_res = -2;
				return "Xoa toan bo hanh trinh thanh cong";
			}else if(delete_trip_res == -1){
				delete_trip_res = -2;
				return "Xoa toan bo hanh trinh that bai";
			}else{
				delete_trip_res = -2;
				return "Co loi xay ra, Vui long thu lai";
			}
		case CMD_DRV_STOP_TIME://48
			sprintf(text, "Thoi gian xac lap 1 lan dung do : %d giay", SystemSetting.data.DrvStopTime);
			return text;
		case CMD_FACTORY://49
			return "Da khoi phuc mac dinh, thiet bi se khoi dong lai sau 30 giay";
		case CMD_LOCATION://50
			get_location(text);
			return text;
		case CMD_DRV_FORCE_CTN_TIME://51
			sprintf(text, "canh bao LXLT truoc %d giay", SystemSetting.data.DrvForeWarningTime);
			return text;
		case CMD_DRIVER_INFO://52
			sprintf(text, "Thong tin LX hien tai\nUID: %llu\nTen lai xe: %s\nGiay phep lai xe: %s\nNgay cap: %s\nNgay het han: %s", tibase.UID, tibase.driver_info.DriverName, tibase.driver_info.DriverLicense, tibase.driver_info.IssDate, tibase.driver_info.ExpDate);
			return text;
		case CMD_CLEAR_DRIVER_DATA://53
		case CMD_DRIVER_RESULT://54
			sprintf(text, "UID: %llu\n Thoi gian LXLT: %d(giay)\nThoi gian Dung: %d(giay)\nThoi gian LXTN: %d(giay)\nThoi gian LXTT: %d(giay)\nQuang duong: %0.02f(km)\nVuot toc: %d(lan)\nDung/Do: %d(lan)\nLXLT: %d(lan)\nVi pham LXLT: %d(lan)",
					tibase.UID,	(get_run_time()), (get_stop_time()), (tibase.DriverInday), (tibase.DriverInWeek), (tibase.GPSMeterInday / (double)1000.0), tibase.OverSpeedCount, tibase.ParkCount, tibase.DriveContinueCount, tibase.OverDriveContinueCount);
			return text;
		case CMD_FUEL_INFO://55
			sprintf(text,"Filter Level: %d\nGia tri cam bien nhien lieu:\nRS232 - 1 : %d\nRS232 - 2 : %d\nADC/PULSE : %d\nGia tri cam bien nhiet do: %.2f",SystemSetting.data.filter_level, MCU_Data.fuel_232_1, MCU_Data.fuel_232_2, MCU_Data.fuel_adc_in, (float)((float)0.0 / (float)10.0));
			return text;
		case CMD_CAM_CFG://56,
			sprintf(text,"So camera : %d, chup khi chay: %d, chup khi dung: %d, block : %d, uptime: %d", DeviceSetting.CameraNumber, SystemSetting.data.FrqCaptureRun, SystemSetting.data.FrqCaptureStop, SystemSetting.data.disable_cam, GPS_Data.cam_up_time);
			return text;
		case CMD_OILD_CHANGE_NOTIFY://57,
			return "Da phat canh bao thay dau";
		case CMD_FEE_NOTIFY://58,
			return "Da phat canh bao han phi";
		case CMD_VEHICLE_CHECK_NOTIFY://59,
			return "Da phat canh bao dang kiem xe";
		case CMD_GET_OBD_INFO://60
			sprintf(text,"SPEED -> %d, TEMP -> %d, RPM -> %d, PFUEL -> %d, S_KM -> %u, S_FUEL -> %d , BAT -> %d, AVG_ECON_FUEL-> %d, P_LOAD -> %d",
				MCU_Data.obd_data.sp, MCU_Data.obd_data.temp, MCU_Data.obd_data.rpm, MCU_Data.obd_data.p_fuel, MCU_Data.obd_data.s_km, MCU_Data.obd_data.s_fuel, MCU_Data.obd_data.bat, MCU_Data.obd_data.avg_econ_fuel, MCU_Data.obd_data.p_load);
			return text;
		case CMD_FWD_SMS:{//61
			int rs = Send_SMS(fwd_PhoneNumber, fwd_Content);
			sprintf(text, "Noi dung: %s, gui toi: %s, ket qua: %s", fwd_Content, fwd_PhoneNumber ,((rs == 0)? "Success" : "Failed"));
			return text;
			break;
		}
		case CMD_OBD_TYPE:{//62
			switch(SystemSetting.data.OBDType){
				case OBD_NONE:
					return "NONE";
				case OBDII_TYPE:
					return "Giao thuc OBD-II";
				case J1939_TYPE:
					return "Giao thuc J1939";
				case J1587_TYPE:
					return "Giao thuc J1587";
				default: break;
			}
			break;
		}
		case CMD_FIRMWARE_URL://63
			if(DeviceSetting.recover_active == true){
				sprintf(text, "Link Download Firmware: %s", SystemSetting.data.update_fw_url);
				return text;
			}
			return "Ban khong co quyen thuc hien chuc nang nay";
		case CMD_UPLOAD_URL://64
			sprintf(text, "URL upload file: %s", SystemSetting.data.upload_file_url);
			return text;
		case CMD_DRIVER_LOGIN://65
			return "DRIVER LOGIN";
			break;
		case CMD_AD_THRESHOLD://66
			sprintf(text, "Gia tri bien dong ad: %d", SystemSetting.data.ad_threshold_increase);
			return text;
		case DIS_RST_SYS_CAM_CMD://68
			sprintf(text, "Khoi dong thiet bi khi ngat ket noi camera : %s", ((SystemSetting.data.dis_sys_rst_when_dis_cam == 0) ? "Co" : "Khong"));
			return text;
		case CMD_ALLOW_OFF_CAM_PARK://69
			sprintf(text, "Ngat ket noi camera khi xe tat may : %s, thoi gian : %d (s)", ((SystemSetting.data.disable_pw_off_cam_when_park == 0) ? "Co" : "Khong"), SystemSetting.data.time_pw_off_camera_when_park);
			return text;
		case CMD_TIME_OFF_CAM_PARK://70
			sprintf(text, "Thoi gian ngat ket noi : %d (s), ngat ket noi camera khi xe tat may : %s", SystemSetting.data.time_pw_off_camera_when_park, ((SystemSetting.data.disable_pw_off_cam_when_park == 0) ? "Co" : "Khong"));
			return text;
		case CMD_PING_CAMERA:{//71
			int rs1, rs2, rs3, rs4;
			rs1 = camera_detect(11);
			rs2 = camera_detect(12);
			rs3 = camera_detect(13);
			rs4 = camera_detect(14);
			sprintf(text, "CAM 1: %d, CAM 2: %d, CAM 3: %d, CAM 4: %d", rs1, rs2, rs3, rs4);
			return text;
		}
		case CMD_DRIVER_LOGOUT://75
			return "DRIVER LOGOUT";
		case CMD_PROVIDER://76
			sprintf(text, "Nha cung cap : %s", SystemSetting.data.Provider);
			return text;
		case CMD_UPLOAD_DB_URL://77
			sprintf(text, "Link upload Database: %s", DeviceSetting.upload_db_url);
			return text;
		case CMD_DECREASE_SPEED://78
			sprintf(text, "Decrease SPD: %d", DeviceSetting.Decrease_Speed);
			return text;
		case CMD_DISABLE_WLXTN:// 79
			sprintf(text, "DISABLE LXTN: %s", DeviceSetting.DisableWLXTN ? "YES" : "NO");
			return text;
		case CMD_DISABLE_WLXTT:// 80
			sprintf(text, "DISABLE LXTT: %s", DeviceSetting.DisableWLXTT ? "YES" : "NO");
			return text;
		case CMD_DISABLE_WCAM:// 81
			sprintf(text, "DISABLE WARNING CAMERA: %s", DeviceSetting.DisableWCAM ? "YES" : "NO");
			return text;
		case CMD_DISABLE_WRFID:// 82
			sprintf(text, "DISABLE WARNING MOVE WITHOUT LOGIN: %s", DeviceSetting.DisableWRFID ? "YES" : "NO");
			return text;
		case CMD_DISABLE_CAPTURE://87
			sprintf(text, "CAPTURE: %s", SystemSetting.data.disable_capture ? "DISABLE" : "ENABLE");
			return text;
		case CMD_GET_DATA_MONITOR://88
			return "Lay du lieu thanh cong";
		case CMD_RESET_CAMERA://92
			return "RESET CAMERA";
		case CMD_CAMERA_DISABLE://93
			sprintf(text, "CAMERA: %s", SystemSetting.data.disable_cam ? "DISABLE" : "ENABLE");
			return text;
		case CMD_AUDIO_DISABLE://94
			sprintf(text, "AUDIO: %s", SystemSetting.data.disable_audio ? "DISABLE" : "ENABLE");
			return text;
		case FUEL_3_TO_2_CMD://101
			sprintf(text, "Fuel 3 -> 2 : %d", SystemSetting.data.fuel_3_to_2);
			return text;
		case FUEL_3_TO_1_CMD://102
			sprintf(text, "Fuel 3 -> 1 : %d", SystemSetting.data.fuel_3_to_1);
			return text;
		case FUEL_2_TO_1_CMD://103
			sprintf(text, "Fuel 2 -> 1 : %d", SystemSetting.data.fuel_2_to_1);
			return text;
		case PASSWORD_FAILED:
			return "Mat khau khong dung, vui long thu lai sau";
		case INVALID_MESSAGE:
			return "Tin nhan sai cu phap, vui long thu lai sau";
		case SETTING_FAILED:
			return "Cai dat that bai, vui long thu lai";
		case ALREADY_SETTING:
			return "Thong tin cai dat da co truoc do, vui long kiem tra lai";
		case DONOT_PERMISSION:
			return "Ban khong co quyen thuc hien chuc nang nay";
		case 104://30
			sprintf(text, "Gia tri lat : %.4f", LAT);
			return text;
		case 105://30
			sprintf(text, "Gia tri long : %.4f", LONG);
			return text;
		default:
			break;
	}
	return "Da xay ra loi, vui long thuc hien lai";
}

void read_rfid_id(void){
	Setting_RFID_ID("0", 0);
}
