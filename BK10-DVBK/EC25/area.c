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

#include "area.h"
#include "log.h"
#include "audio.h"
#include "queue.h"
#include "database.h"
#include "devconfig.h"

#define AREA_DB_PATH          "/usrdata/Area.db"

sqlite3 *areadb;
char *aErrMsg_ = 0;
const char* adata = "Callback function called";
Queue_t queue_area_found;

int this_area_index = 0;
float this_area[1000][2];

static int callback(void *NotUsed, int argc, char **argv, char **azColName);

int open_areadb(void){
	int rc;
	if(access(AREA_DB_PATH, F_OK) < 0){/* file not Found */
		return -1;
	}

	rc = sqlite3_open(AREA_DB_PATH, &areadb);
	if( rc ) {
		log_debug("Can't open areadb: \n");
		return -1;
	}

	if( rc != SQLITE_OK ){
		log_debug("SQL error: %s\n", aErrMsg_);
		sqlite3_free(aErrMsg_);
		return -1;
	}

	return 0;
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
//	int i;
//	for(i = 0; i<argc; i++) {
//		log_debug("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	log_debug("\n");
	return 0;
}

static int callback_Area(void *NotUsed, int argc, char **argv, char **azColName){
//	int i;
//	for(i = 0; i<argc; i++){
//		log_debug("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	log_debug("\n");

	if(argc < 2){
		log_debug(" ERROR AREA TABLE \r\n ");
		return -1;
	}

	this_area[this_area_index][0] = atof(argv[2]);
	this_area[this_area_index][1] = atof(argv[1]);
	this_area_index++;

	return 0;
}

//lat 2 long 1
int pnpoly(int nvert, float vert[nvert][2], float testx, float testy){
	int i, j, c = 0;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if(((vert[i][0] > testy) != (vert[j][0] > testy)) && (testx < (vert[j][1] - vert[i][1]) * (testy - vert[i][0]) / (vert[j][0] - vert[i][0]) + vert[i][1]))
			c = !c;
	}
	//log_debug("Gia tri tra ve cua ham pnpoly la %d",c);
	return c;
}

static volatile u16 report_arealist_timeout = 0;
area_typedef area_found;

static int callback_report_area_list(void *NotUsed, int argc, char **argv, char **azColName){
//	int i;
//	log_debug("\n");
//	log_debug("Da vao ham callback report area list\n");
//	log_debug("Gia tri cua argc la %d\n",argc);
//	for(i = 0; i<argc; i++){
//		log_debug("i = %d, %s = %s\n",i, azColName[i], argv[i] ? argv[i] : "NULL");
//		log_debug("i = %d, %s = %s\n",i, azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	log_debug("\n");
//	log_debug("\n");

	if(argc < 11){
		log_debug(" ERROR AREA TABLE \r\n ");
		return -1;
	}

	memset(&area_found, 0x0, sizeof(area_typedef));

	area_found.AreaID = ec21_atoi(argv[0]);
	strcpy(area_found.AreaName, argv[1]);
	area_found.nVert = ec21_atoi(argv[6]);
	area_found.TrafficSigns = ec21_atoi(argv[7]);
	area_found.TypeRoad = ec21_atoi(argv[8]);
	area_found.TypeArea = ec21_atoi(argv[9]);
	area_found.TypeDirection = ec21_atoi(argv[10]);

	if((DeviceSetting.CarType >= 1)  && (DeviceSetting.CarType <= 4))
		area_found.TypeCar = DeviceSetting.CarType;
	else
		area_found.TypeCar = GroupVehicle_1;

	if(!enqueue(&queue_area_found, &area_found, sizeof(area_typedef))){
		log_debug("Queue area full\n");
	}
}

//char test_note[512];
void* thread_area_handler(void* arg){
	//log_debug("Khoi tao cho phan area\n");
	initialize(&queue_area_found, MAX_QUEUE_SIZE*2);
	char cmd[512];
	int rc;
	u8 countCheckDirection = 0;
	u8 checkDirection = 0;
	u8 checkSpeedLimit = 0;
	u8 checkArea = 0;
	u8 checkAreaLimit = 0;
    int temp;
	u8 recv[160];
	u16 recv_size = 0;
	double distance = 0;
	u8 speedLimit = 0;
	loc_def loc_area;
	u8 checkBreakAreaLimit = 0;
	for(;;){
		if((GPS_Data.gps_speed > 3) && (GPS_Data.gps_status == 1)){
			if(open_areadb() < 0){
				log_debug("OPEN AREA DB ERROR!! \n");
				sleep(10);
				continue;
			}

			//log_debug("OPEN AREA DB Okay!! \n");
			sprintf(cmd, "SELECT * FROM Area WHERE MinLat <= '%f' AND MaxLat >= '%f' AND  MinLon <= '%f' AND MaxLon >= '%f'", GPS_Data.location.latitude, GPS_Data.location.latitude, GPS_Data.location.longitude, GPS_Data.location.longitude);
			//log_debug("\n%s\n", cmd);

			rc = sqlite3_exec(areadb, cmd, callback_report_area_list, (void*)adata, &aErrMsg_);
			//log_debug("Gia tri cua rc la %d\n",rc);
			//log_debug("Noi dung cua aErrMsg la %s\n",aErrMsg_);
			if(rc != SQLITE_OK){
				log_debug("SQL error: %s\n", aErrMsg_);
				sqlite3_free(aErrMsg_);
			}

			while(!isEmpty(&queue_area_found)){//check tung vung 1
				area_typedef buf;
				dequeue(&queue_area_found, &buf, sizeof(area_typedef));

				//lay ra cac dinh cua da giac
				this_area_index = 0;
				sprintf(cmd, "SELECT * FROM AreaData WHERE AreaID = %d", buf.AreaID);
				rc = sqlite3_exec(areadb, cmd, callback_Area, (void*)adata, &aErrMsg_);
				//log_debug("Gia tri cua rc la %d\n",rc);
				if(rc != SQLITE_OK){
					log_debug("SQL error: %s\n", aErrMsg_);
					sqlite3_free(aErrMsg_);
				}

				checkAreaLimit = 0;

				if(pnpoly(this_area_index, this_area, GPS_Data.location.latitude, GPS_Data.location.longitude) == 1){
					//dang trong vung
//					log_file(" BAN DANG TRONG VUNG %s, AreaID la %d \n", buf.AreaName, buf.AreaID);
//					log_file("Gia tri cua TrafficSigns la %d\n",buf.TrafficSigns);
//					log_file("Gia tri cua type car la %d\n",buf.TypeCar);
					if((GPS_Data.location.latitude != 0) && (GPS_Data.location.longitude != 0))
					{
						loc_area.latitude = GPS_Data.location.latitude;
						loc_area.longitude = GPS_Data.location.longitude;
					}

//					log_file("Gia tri cua location cua vi tri bat dau la lat = %f, long = %f",loc_area.latitude,loc_area.longitude);
                    if(buf.TrafficSigns <= 0)
                    {
						if(buf.TypeArea == GroupOutsideAreaResidential)
						{
							if(buf.TypeCar == GroupVehicle_1)
							{
								if((buf.TypeRoad == TypeRoad1) || (buf.TypeRoad == TypeRoad2))
								{
									add_play_list("/usrdata/sound/", "begin90.wav", SystemSetting.data.SpeakerVolume);
									speedLimit = 90;
								}
								else
								{
									add_play_list("/usrdata/sound/", "begin80.wav", SystemSetting.data.SpeakerVolume);
									speedLimit = 80;
								}
							}
							else if(buf.TypeCar == GroupVehicle_2)
							{
								if((buf.TypeRoad == TypeRoad1) || (buf.TypeRoad == TypeRoad2))
								{
									add_play_list("/usrdata/sound/", "begin80.wav", SystemSetting.data.SpeakerVolume);
									speedLimit = 80;
								}
								else
								{
									add_play_list("/usrdata/sound/", "begin70.wav", SystemSetting.data.SpeakerVolume);
									speedLimit = 70;
								}
							}
							else if(buf.TypeCar == GroupVehicle_3)
							{
								if((buf.TypeRoad == TypeRoad1) || (buf.TypeRoad == TypeRoad2))
								{
									add_play_list("/usrdata/sound/", "begin70.wav", SystemSetting.data.SpeakerVolume);
									speedLimit = 70;
								}
								else
								{
									add_play_list("/usrdata/sound/", "begin60.wav", SystemSetting.data.SpeakerVolume);
									speedLimit = 60;
								}
							}
							else
							{
								if((buf.TypeRoad == TypeRoad1) || (buf.TypeRoad == TypeRoad2))
								{
									add_play_list("/usrdata/sound/", "begin60.wav", SystemSetting.data.SpeakerVolume);
									speedLimit = 60;
								}
								else
								{
									add_play_list("/usrdata/sound/", "begin50.wav", SystemSetting.data.SpeakerVolume);
									speedLimit = 50;
								}
							}
						}
						else
						{
							if((buf.TypeRoad == TypeRoad1) || (buf.TypeRoad == TypeRoad2))
							{
								add_play_list("/usrdata/sound/", "begin60.wav", SystemSetting.data.SpeakerVolume);
								speedLimit = 60;
							}
							else
							{
								add_play_list("/usrdata/sound/", "begin50.wav", SystemSetting.data.SpeakerVolume);
								speedLimit = 50;
							}
						}
                    }
                    else
                    {
                    	if(buf.TrafficSigns == speedLimit40km)
						{
							add_play_list("/usrdata/sound/", "begin40.wav", SystemSetting.data.SpeakerVolume);
							speedLimit = 40;
						}
                    	else if(buf.TrafficSigns == speedLimit50km)
                    	{
                    		add_play_list("/usrdata/sound/", "begin50.wav", SystemSetting.data.SpeakerVolume);
                    		speedLimit = 50;
                    	}
                    	else if(buf.TrafficSigns == speedLimit60km)
                    	{
                    		add_play_list("/usrdata/sound/", "begin60.wav", SystemSetting.data.SpeakerVolume);
                    		speedLimit = 60;
                    	}
                    	else if(buf.TrafficSigns == speedLimit70km)
                    	{
                    		add_play_list("/usrdata/sound/", "begin70.wav", SystemSetting.data.SpeakerVolume);
                    		speedLimit = 70;
                    	}
                    	else if(buf.TrafficSigns == speedLimit80km)
                    	{
                    		add_play_list("/usrdata/sound/", "begin70.wav", SystemSetting.data.SpeakerVolume);
                    		speedLimit = 80;
                    	}
                    	else if(buf.TrafficSigns == speedLimit100km)
                    	{
                    		//add_play_list("/usrdata/sound/", "begin70.wav", SystemSetting.data.SpeakerVolume);
                    		speedLimit = 100;
                    	}
                    	else if(buf.TrafficSigns == speedLimit120km)
                    	{
                    		//add_play_list("/usrdata/sound/", "begin70.wav", SystemSetting.data.SpeakerVolume);
                    		speedLimit = 120;
                    	}
                    	else // Truong hop chua co trong danh sach
                    	{
                    		continue;
                    	}
                    }

 //                   log_file("Gia tri toc do cho phep la %d",speedLimit);
                    checkBreakAreaLimit = 0;
                    while(checkBreakAreaLimit < 3)
                    {
                    	if(!pnpoly(this_area_index, this_area, GPS_Data.location.latitude, GPS_Data.location.longitude))
                    	{
                    		checkBreakAreaLimit++;
 //                   		log_file("Check lan %d da ra khoi khu vuc dang xet voi lat = %f, long = %f\n",checkBreakAreaLimit,GPS_Data.location.latitude, GPS_Data.location.longitude);
                    	}
                    	else
                    	{
                    		checkBreakAreaLimit = 0;
                    	}

                    	if(( distance = calculate_distance(loc_area, GPS_Data.location, 'm') > 200) && (checkArea == 0))
                    	{
//                    		log_file("Ban da vao khu vuc bi gioi han toc do");
//                    		log_file("Gia tri cua location cua diem xet la lat = %f, long = %f",loc_area.latitude,loc_area.longitude);
//                    		log_file("Gia tri cua location hien tai la lat = %f, long = %f",GPS_Data.location.latitude,GPS_Data.location.longitude);
//                    		log_file("Gia tri distance do duoc la distance = %.2f",distance);
                    		sprintf(cmd, "%d.wav",speedLimit );
                    		add_play_list("/usrdata/sound/", (char *)cmd, SystemSetting.data.SpeakerVolume);
                    		checkArea = 1;
                    		checkAreaLimit = 1;
                    	}
                    	if((buf.TypeRoad == 2) || (buf.TypeRoad == 4))
                    	{
//                    		log_file("Da vao duong 1 chieu");
//                    		log_file("Gia tri cua bering hien tai la %f", GPS_Data.bearing);
//                    		log_file("Gia tri cua typeDirection la %d\n", buf.TypeDirection);
                    		temp = abs(GPS_Data.bearing - buf.TypeDirection);
                    		if((((temp > 180) ? (360 - temp) : temp) <= 30)  && (checkDirection == 0) && (checkAreaLimit == 1))
                    		{
//                    			log_file("Set dieu kien di nguoc chieu");
                    			countCheckDirection++;
                    		}
                    		else
                    		{
//                    			log_file("Reset dieu kien di nguoc chieu");
                    			countCheckDirection = 0;
                    		}
                    		if(countCheckDirection == 3)
                    		{
                    			checkDirection = 1;
//                    			log_file("Canh bao di nguoc chieu\n");
                    			add_play_list("/usrdata/sound/", " DiNguocChieu.wav", SystemSetting.data.SpeakerVolume);
                    		}
                    	}

                    	if(speedLimit < (u8)(GPS_Data.gps_speed) && (checkSpeedLimit == 0) && (checkAreaLimit == 1))
                    	{
                    		checkSpeedLimit = 1;
                    		add_play_list("/usrdata/sound/", " speedlimit.wav", SystemSetting.data.SpeakerVolume);
 //                   		log_file("Ban dang di qua toc do cho phep, toc do cho phep la %d, toc do cua ban la %d\n",speedLimit, (u8)(GPS_Data.gps_speed));
                    	}

                    	sleep(1);
                    }
//                    log_file(" BAN DA RA KHOI VUNG %s, AreaID la %d \n", buf.AreaName, buf.AreaID);
//                    log_file("Tai vi tri lat = %f, long = %f",GPS_Data.location.latitude, GPS_Data.location.longitude);
                    sprintf(cmd,"finish%d.wav",speedLimit );
                    add_play_list("/usrdata/sound/", (char *)cmd, SystemSetting.data.SpeakerVolume);
                    freequeue(&queue_area_found);
				}
			}
			sqlite3_close(areadb);
			checkDirection == 0;
			checkSpeedLimit = 0;
			checkArea = 0;
		}
		sleep(1);
	}
}



