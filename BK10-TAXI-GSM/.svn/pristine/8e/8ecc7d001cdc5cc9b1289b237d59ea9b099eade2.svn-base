#ifndef DEVCFG_H
#define DEVCFG_H

#include <stdio.h>
#include <stdarg.h>

#define	MAX_SIZE_IMSI	  		  32
#define	MAX_SIZE_IMEI	  		  32
#define	MAX_SIZE_ICCID	  		  32
#define	MAX_SIZE_NUMBER_PLATE	  32
#define	MAX_SIZE_URL	      	  64
#define	MAX_SIZE_PHONE_NUMBER	  16

#define USE_ENCRYPT  0

typedef enum{
	CYCLE_MODE = 0,
	ADC_MODE = 1,
	PULSE_MODE = 2
}Fuel_type;

typedef enum{
	OBD_NONE = 0,
	OBDII_TYPE = 1,
	J1939_TYPE = 2,
	J1587_TYPE = 3,
	VFE34_TYPE = 4,
	VF8_TYPE = 5
}enum_OBD_type;

typedef enum{
	ACC_LSM = 0,
	ACC_ONLY = 1,
	LSM_ONLY = 2,
	NO_ACC_LSM_SPEED = 3,
	NO_ACC_LSM_NO_SPEED = 4
}enum_AccMode;

typedef struct{
	char camera_ip[32];
	char camera_sn[32];
	bool detect;
	bool active;
	bool block;
	bool set_plate;
}camera_inf_def;

/* 1: Xe oto con cho nguoi den 30 cho(tru xe bus)
	   Xe oto tai co trong tai den 3.5 tan
   2: Xe cho nguoi tren 30 cho(tru xe bus)
			   Xe oto tai co trong tai tren 3.5 tan
   3: Oto buyt, Oto dau keo ro mooc
			   Oto chuyen dung, xe mo to
   4: Oto keo ro mooc, oto keo xe khac
*/
typedef enum{
	GroupVehicle_1 = 1,
	GroupVehicle_2 = 2,
	GroupVehicle_3 = 3,
	GroupVehicle_4 = 4,
}enum_GroupVehicle_typedef;

typedef struct{
	u16 FuelManualMode;
	u8 SpeedLimit;
	u16 time_reset_increment;
	char NumberPlate[64];
	u32 BauRate2;
	u32 BauRate4;
	bool recover_active;
	bool sms_enset;
	bool com_enset;
	camera_inf_def camera_list[8];
	u8 CurrentDay;
	u8 CameraNumber;          //192.168.1.11    -> 11 + n
	u8 CarType;
	u8 cam_num_detected;
	u8 recover_pass[32];
	u8 count_cam_reset;
	u8 device_reset_when_park;
	char IMEI[MAX_SIZE_IMEI + 1];
	char IMSI[MAX_SIZE_IMSI + 1];
	char ICCID[MAX_SIZE_ICCID + 1];
	char ManufactureDate[64];
	char DeviceUpdatedDate[64];
}device_setting_typedef;

typedef struct{
	u64 DeviceID;
	char domain[MAX_SIZE_URL + 1];
	u16 port;
	u8 FrqRun;
	u16 FrqStop;
	char PassWord[16];
	u8 BuzzerLevel;
	enum_AccMode AccMode;
	char upload_file_url[MAX_SIZE_URL + 1];
	u8 filter_level;
	Fuel_type AdcPulseCfg;
	enum_OBD_type OBDType;
	char KeyActive[16];
	u8 certified;
	u8 SlienceMode;
	u8 SpeakerVolume;
	u8 DeltaSpeedPG;
	u8 disable_camera_roaming_mode;
	u16 time_reset_when_disconnect;
	u8 DoorRevert;
	u8 AirRevert;
	u8 disable_cam;
	u8 disable_capture;
	u8 disable_audio;
	u16 FrqCaptureRun;
	u16 FrqCaptureStop;
	u16 DrvForeWarningTime;
	u16 DrvStopTime;
	u8 RunLevel;
	u16 ad_threshold_increase;
	float MotionValue;
	char Phone1[MAX_SIZE_PHONE_NUMBER];
	char Phone2[MAX_SIZE_PHONE_NUMBER];
	char Phone_sos[MAX_SIZE_PHONE_NUMBER];
	char Provider[32];
	char update_fw_url[MAX_SIZE_URL + 1];
	u8 fuel_3_to_2;
	u8 fuel_3_to_1;
	u8 fuel_2_to_1;
	u8 dis_sys_rst_when_dis_cam;
	u8 disable_pw_off_cam_when_park;
	u16 time_pw_off_camera_when_park;
	u8 taxi_mode;
	u8 hirring_ctrl;
}sys_setting;//SIZE SETTING 384byte

typedef union{
	sys_setting data;
	u8 bytes[sizeof(sys_setting)];
}system_setting_typedef;

extern system_setting_typedef SystemSetting;

extern device_setting_typedef DeviceSetting;

#define KEYACTIVE_PATH     		"/usr/data/keyactive.cfg"
#define CERTIFIED_PATH     		"/usr/data/certified.cfg"
#define DEVICE_ID_PATH          "/usr/data/DeviceId.cfg"
#define DOMAIN_PATH             "/usr/data/Domain.cfg"
#define PORT_PATH               "/usr/data/Port.cfg"
#define DISABLE_CAM_PATH     	"/usr/data/DisableCam.cfg"
#define DISABLE_AUDIO_PATH     	"/usr/data/DisableAudio.cfg"
#define UPLOAD_FILE_URL_PATH    "/usr/data/UpLoadFileUrl.cfg"
#define MANUFACTURE_DATE_PATH   "/usr/data/ManufactureDate.cfg"
#define DEVICE_UPDATE_DATE_PATH "/usr/data/DeviceUpdatedDate.cfg"
#define SLIENCE_MODE_PATH  		"/usr/data/SlienceMode.dat"

#define DIS_CAM_ROAMING_PATH    "/usr/data/DisCamRoaming.dat"
#define DIS_CAM_CAPTURE_PATH    "/usr/data/DisCamCapture.dat"
#define NUMBER_PLATE_PATH       "/usr/data/NumberPlate.dat"
#define FRQ_RUN_PATH            "/usr/data/FrqRun.dat"
#define FRQ_STOP_PATH           "/usr/data/FrqStop.dat"
#define SPEED_LIMIT_PATH        "/usr/data/SpeedLimit.dat"
#define PASSWORD_PATH           "/usr/data/PassWord.dat"
#define PHONE_1_PATH          	"/usr/data/Phone1.dat"
#define PHONE_2_PATH          	"/usr/data/Phone2.dat"
#define PHONE_SOS_PATH          "/usr/data/PhoneSos.dat"
#define ADC_PULSE_PATH       	"/usr/data/AdcPulseCfg.dat"
#define AD_THRESHOLD_PATH      	"/usr/data/AdThreshold.dat"
#define BAUD2_PATH      	 	"/usr/data/Baud2.dat"
#define BAUD4_PATH       		"/usr/data/Baud4.dat"
#define FUEL_MANUAL_PATH       	"/usr/data/FuelManual.dat"
#define BUZZER_LEVEL_PATH 		"/usr/data/BuzzerLevel.dat"
#define DOOR_REVERT_PATH 		"/usr/data/DoorRevert.dat"
#define AIR_REVERT_PATH 		"/usr/data/AirRevert.dat"
#define RUN_LEVEL_PATH 			"/usr/data/RunLevel.dat"
#define MOTION_VALUE_PATH 		"/usr/data/MotionValue.dat"
#define CURRENT_DAY_PATH        "/usr/data/CurrentDay.dat"
#define ACC_MODE_PATH        	"/usr/data/ACCMode.dat"
#define OBD_TYPE_PATH        	"/usr/data/OBDType.dat"
#define DRV_FORE_WARN_TIME_PATH "/usr/data/DrvForceWarningTime.dat"
#define DRV_STOP_TIME_PATH 		"/usr/data/DrvStopTime.dat"
#define UPDATE_FW_URL_PATH      "/usr/data/UpdateFwUrl.dat"
#define CAM_NUMBER_PATH   		"/usr/data/CameraNumber.dat"
#define FRQ_CAPTURE_RUN_PATH    "/usr/data/FrqCaptureRun.dat"
#define FRQ_CAPTURE_STOP_PATH   "/usr/data/FrqCaptureStop.dat"
#define SPEAKER_VOLUME_PATH     "/usr/data/SpeakerVolume.dat"
#define CAR_TYPE_PATH     		"/usr/data/CarType.dat"
#define FILTER_LEVEL_PATH     	"/usr/data/FilterLevel.dat"
#define FUEL_3_TO_2_PATH     	"/usr/data/Fuel3To2.dat"
#define FUEL_3_TO_1_PATH     	"/usr/data/Fuel3To1.dat"
#define FUEL_2_TO_1_PATH     	"/usr/data/Fuel2To1.dat"
#define DIS_RST_DEV_PATH     	"/usr/data/DisRstDev.dat"
#define DIS_OFF_CAM_PARK_PATH 	"/usr/data/DisableOffCamPark.dat"
#define TIME_OFF_CAM_PARK_PATH  "/usr/data/TimeOffCamPark.dat"
#define TAXI_MODE_PATH  		"/usr/data/TaxiMode.dat"
#define TAXI_HIRRING_PATH  		"/usr/data/TaxiHirring.dat"
#define DELTA_SPD_PG_PATH  		"/usr/data/DeltaSpeedPG.dat"

#define TIME_RESET_MODULE_PATH  "/usr/data/TimeResetModule.dat"

#define TIME_RESET_INCREMENT 	"/usr/data/time_reset_increment.dat"

#define COUNT_RESET_BY_CAM_PATH "/usr/data/CountDeviceReset.dat"

#define CAM_NUM_DETECTED_PATH  	"/usr/data/CamNumDetected.dat"

#define DEV_RST_WHEN_PARK_PATH  "/usr/data/DeviceResetPark.dat"

#define PROVIDER_PATH      		 "/usr/data/Provider.dat"

void load_device_setting(void);

#define WARNING_PHONE_NUMBER   	"0979795791"

void sync_dev_setting(void);

#endif
