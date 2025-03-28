#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/poll.h>
#include "ql_sgmii.h"

#include "gps.h"
#include "network.h"
#include "ql_network.h"
#include "log.h"
#include "database.h"
#include "atc.h"
#include "fw.h"
#include "setting.h"
#include "devconfig.h"
#include "camera.h"
#include "gps.h"
#include "driver.h"
#include "ql_sgmii.h"
#include <curl/curl.h>
#include "encrypt.h"
#include "queue.h"

#define MAX_QUEUE_IMAGE_SIZE	50

Queue_t Qimg_delete;
Queue_t queue_image;

static u8 mcu_response[160] = {0x0};
u16 mcu_response_len = 0;

int get_snapshot(u32 stamp, u8 chanel);

#if (USE_ENCRYPT == 1)
	void http_upload_img_encrypt(u32 stamp, u8 chanel);
#else
	void http_upload_img(u32 stamp, u8 chanel);
#endif  /* USE_ENCRYPT */

void setting_number_plate(char * plate);

void update_realtime_overlay_for_fake_camera(u8 chanel);

void update_realtime_overlay(loc_def this_loc, u8 speed, u8 chanel);

static void* thrd_upload_image(void* arg);
static pthread_t thrd_img_upload;

char camera_tmp[256];
char sn_tmp[32];

int camera_detect(u8 chanel){
	char ip[32];
	if(SystemSetting.data.taxi_mode == 1) return -1;
	sprintf(ip, "192.168.1.%d\0", chanel);
	return check_ipaddress(ip);
}

void Qdelete_img(char *fName){
	if(!enqueue(&Qimg_delete, fName, 64)){
		log_fatal("Queue delete IMG full\n");
	}
}

bool Push_IMG_Data(u32 sttime, u8 chanel){
	image_mail_queue_typedef obj;
	obj.sttime = sttime;
	obj.chanel = chanel;

	return enqueue(&queue_image, &obj, sizeof(image_mail_queue_typedef));
}

int flg_disable_sgmi = -1;
int flg_enable_sgmi = -1;
u8 sgmi_set_time = 0;
u8 flg_cam_on = 0;
u8 flg_cam_off = 0;
u8 pw_camera_status = 0;
void* thread_camera_handler(void* arg){
	u8 cam_index = 0;
	GPS_Data.flg_update_cam = false;

	if (pthread_create(&thrd_img_upload, NULL, thrd_upload_image, NULL) != 0){
		log_system("Fail to create thread thrd_upload_image !\n");
	}

	for(;;){
		//log_system("disable_pw_off_cam_when_park %d, no_acc_time %d, time_pw_off_camera_when_park %d/n", DeviceSetting.allow_pw_off_cam_when_park, GPS_Data.no_acc_time, SystemSetting.data.time_pw_off_camera_when_park);
		if((SystemSetting.data.taxi_mode == 1) || ((SystemSetting.data.disable_pw_off_cam_when_park == 0) && (GPS_Data.no_acc_time > SystemSetting.data.time_pw_off_camera_when_park) && (SystemSetting.data.time_pw_off_camera_when_park >= 120))){
			//">CAMOFF"
			flg_cam_on = 0;
			if(flg_cam_off == 0){
				if(mcu_send_and_rcv(">CAMOFF", strlen(">CAMOFF"), mcu_response, &mcu_response_len, 5000) == 0){
					if(!strncmp(mcu_response, "OK", 2)){
						flg_cam_off = 1;
						GPS_Data.flg_send_cfg = 1;
						GPS_Data.time_to_send_evt = 88;
						log_system("POWER OFF CAMERA SUCCSESS!!!\n");
					}
				}
			}
			pw_camera_status = 0;
			sleep(5);
			continue;
		}else{
			//">CAMON"
			pw_camera_status = 1;
			flg_cam_off = 0;
			if(flg_cam_on == 0){
				if(mcu_send_and_rcv(">CAMON", strlen(">CAMON"), mcu_response, &mcu_response_len, 5000) == 0){
					if(!strncmp(mcu_response, "OK", 2)){
						GPS_Data.flg_scan_cam = true;
						GPS_Data.flg_send_cfg = 1;
						GPS_Data.time_to_send_evt = 0;
						flg_cam_on = 1;
						clear_camera_cnt();
						log_system("POWER ON CAMERA SUCCSESS!!!\n");
					}
				}
			}
		}

		if((SystemSetting.data.disable_cam == 1) || ((SystemSetting.data.disable_camera_roaming_mode == 1) && (MobileCountryCode != VIETNAM_MCC))){
			if(flg_disable_sgmi != 0){
				flg_disable_sgmi = ql_sgmii_disable();
				sgmi_set_time++;
				if(sgmi_set_time >= 3){
					sgmi_set_time = 0;
					flg_disable_sgmi = 0;
					flg_enable_sgmi = -1;
				}
			}
			sleep(5);
			continue;
		}

		if(flg_enable_sgmi != 0){
			flg_enable_sgmi = lan_port_init();
			sgmi_set_time++;
			if(sgmi_set_time >= 3){
				sgmi_set_time = 0;
				flg_enable_sgmi = 0;
				flg_disable_sgmi = -1;
			}
		}

		if(GPS_Data.flg_reboot_sgmi == true){
			GPS_Data.flg_reboot_sgmi = false;
			lan_port_init();
			sleep(3);
		}

		if(GPS_Data.flg_scan_cam == true){//detect camera
			GPS_Data.flg_scan_cam = false;
			//scan camera serial number
			for(cam_index = 0; cam_index < 8; cam_index++){
				if(strlen(DeviceSetting.camera_list[cam_index].camera_sn) < 5){
					if(!camera_detect(11 + cam_index)){
						memset(sn_tmp, 0x0, 32);
						sprintf(camera_tmp, "http://admin:bk9108vn@192.168.1.%d/ISAPI/System/deviceInfo\0", (11 + cam_index));
						if(!get_camera_serial_number(camera_tmp, "/tmp/cam.xml", sn_tmp)){
							memset(DeviceSetting.camera_list[cam_index].camera_sn, 0x0, 32);
							strcpy(DeviceSetting.camera_list[cam_index].camera_sn, sn_tmp);
							if(DeviceSetting.camera_list[cam_index].detect == false){
								u8 tmp = DeviceSetting.CameraNumber;
								tmp |= (1 << cam_index);
								if(DeviceSetting.CameraNumber != tmp){
									DeviceSetting.CameraNumber = tmp;
									write_number(CAM_NUMBER_PATH, (u64)DeviceSetting.CameraNumber);
								}
							}
							DeviceSetting.camera_list[cam_index].active = true;
							DeviceSetting.camera_list[cam_index].detect = true;

							/* store camera number has detected */
							DeviceSetting.cam_num_detected = (cam_index + 1);
							write_number(CAM_NUM_DETECTED_PATH, (u64)DeviceSetting.cam_num_detected);

							/* clear cam reset count */
							DeviceSetting.count_cam_reset = 0;
							write_number(COUNT_RESET_BY_CAM_PATH, (u64)DeviceSetting.count_cam_reset);

							DeviceSetting.device_reset_when_park = 0;
							write_number(DEV_RST_WHEN_PARK_PATH, (u64)DeviceSetting.device_reset_when_park);

							sync();

							log_system("DETECTED CAMERA -> %d, IMEI %s\n", cam_index + 1, DeviceSetting.camera_list[cam_index].camera_sn);
						}
					}else{
						log_debug("CAMERA 192.168.1.%d NOT FOUND \n", (11 + cam_index));
					}
				}
			}
		}//detect camera

		//get snapshot
		if(DeviceSetting.CameraNumber != 0){
			//update text overlay
			if(GPS_Data.flg_update_cam == true){
				GPS_Data.flg_update_cam = false;
				for(cam_index = 0; cam_index < 8; cam_index++){
					//capture image
					if((DeviceSetting.camera_list[cam_index].active == true) && (DeviceSetting.camera_list[cam_index].block != true)){
						update_realtime_overlay(GPS_Data.location, GPS_Data.gps_speed, (cam_index + 1));
						usleep(100000);
					}else if(DeviceSetting.camera_list[cam_index].block == true){
						update_realtime_overlay_for_fake_camera(cam_index + 1);
					}
				}
			}

			if(GPS_Data.flg_capture == true){
				u32 sttime = GPS_Data.timestamp;
				for(cam_index = 0; cam_index < 8; cam_index++){
					if((SystemSetting.data.disable_camera_roaming_mode == 1) && (current_nw_mode == E_QL_WWAN_REG_ROAMING)){
						break;
					}

					if((DeviceSetting.camera_list[cam_index].active == true) && (DeviceSetting.camera_list[cam_index].block != true)){
						if((DeviceSetting.camera_list[cam_index].detect == true) && (DeviceSetting.camera_list[cam_index].set_plate != true)){
							setting_number_plate(DeviceSetting.NumberPlate);
							setting_provider(SystemSetting.data.Provider);
							cam_setting_driver_info(tibase.driver_info.DriverName, tibase.driver_info.DriverLicense);
							DeviceSetting.camera_list[cam_index].set_plate = true;
						}
						//capture image
						if(!get_snapshot((sttime + cam_index), (cam_index + 1))){
							DeviceSetting.camera_list[cam_index].detect = true;
							if(strlen(DeviceSetting.camera_list[cam_index].camera_sn) == 0){
								GPS_Data.flg_scan_cam = true;
								GPS_Data.time_to_send_evt = 88;
							}
#if (USE_ENCRYPT == 1)
							char fname_in[128];
							char Fname_out[128];
							sprintf(fname_in, "/tmp/%u.jpg", (sttime + cam_index));
							sprintf(Fname_out, "/tmp/encrypt_%u.jpg", (sttime + cam_index));

							encrypt_picture(fname_in, Fname_out);
							while(access(fname_in, F_OK) == 0){
								unlink(fname_in);
							}

							if(Push_IMG_Data((sttime + cam_index), (cam_index + 1))){
								log_fatal("ADD queue image success!!!\n");
							}else{
								while(access(Fname_out, F_OK) == 0){
									unlink(Fname_out);
								}
								log_fatal("ADD queue image failed!!!\n");
							}
#else
							if(Push_IMG_Data((sttime + cam_index), (cam_index + 1))){
								log_fatal("ADD queue image success!!!\n");
							}else{
								u8 img_name[64];
								sprintf(img_name, "/tmp/%u.jpg", (sttime + cam_index));
								while(access(img_name, F_OK) == 0){
									unlink(img_name);
								}
								log_fatal("ADD queue image failed!!!\n");
							}
#endif /* USE_ENCRYPT */
						}else if(DeviceSetting.camera_list[cam_index].detect == true){
							if(SystemSetting.data.dis_sys_rst_when_dis_cam == 0){
								if(get_snapshot((sttime + cam_index), (cam_index + 1)) < 0){
									/* increase cam reset count */
									DeviceSetting.count_cam_reset = DeviceSetting.count_cam_reset + 1;
									write_number(COUNT_RESET_BY_CAM_PATH, (u64)DeviceSetting.count_cam_reset);
									sync();
									/* reset all camera */
									reboot_camera();

									/* reboot device */
									set_device_reset_timeout(10);

									log_system("CAMERA %d DISCONNECTED -> SYSTEM REBOOT\n", (cam_index + 1));

									continue;
								}
							}

							log_system("CAMERA %d DISCONNECTED -> REBOOT CAMERA\n", (cam_index + 1));
							reboot_camera();
							lan_port_init();
						}
					}
				}
				GPS_Data.flg_capture = false;
			}
		}
		sleep(1);
	}
}

static void* thrd_upload_image(void* arg){
	__useconds_t usleep___ = 1000000;
	initialize(&Qimg_delete, 10);
	initialize(&queue_image, MAX_QUEUE_IMAGE_SIZE);
	for(;;){
		usleep___ = 3000000;
		if(network_data.internet_status){
			if(!isEmpty(&queue_image)){
				u8 img_name[64];
				image_mail_queue_typedef img_buf;
				dequeue(&queue_image, &img_buf, sizeof(image_mail_queue_typedef));

#if (USE_ENCRYPT == 1)
				http_upload_img_encrypt(img_buf.sttime, img_buf.chanel);
#else
				http_upload_img(img_buf.sttime, img_buf.chanel);
#endif /* USE_ENCRYPT */

				sprintf(img_name, "/tmp/%u.jpg", img_buf.sttime);
				Qdelete_img(img_name);
				usleep___ = 200000;//500ms
			}
		}

		while(get_queue_size(&Qimg_delete) > 2){
			char fname[64];
			dequeue(&Qimg_delete, fname, 64);

			while(access(fname, F_OK) == 0){
				unlink(fname);
			}
		}
		usleep(usleep___);
	}
}

int get_snapshot(u32 stamp, u8 chanel){
	char cmdline[256];
	long len = 0;
	FILE *file;
	sprintf(cmdline, "curl -o /tmp/%u.jpg http://admin:bk9108vn@192.168.1.%d/Streaming/Channels/1/picture > /tmp/%u.jpg\0", stamp, (10 + chanel), stamp);

	system(cmdline); //get snapshot to /tmp

	//step 3: check exist dowloaded file
	memset(cmdline, 0x0, 256);
	sprintf(cmdline, "/tmp/%u.jpg", stamp);

	if(access(cmdline, F_OK) == 0){//check file in dir
		file = fopen(cmdline, "r");
		if(file == NULL){
			return -1;
			log_fatal("\nCAPTURE IMAGE FAILED (0)\n");
		}
		fseek(file, 0L, SEEK_END);
		len = ftell(file);

		if(len < 1000){
			fclose(file);
			unlink(cmdline);
			log_fatal("\nCAPTURE IMAGE FAILED (1)\n");
			return -1;
		}

		//step 3: check exist after download file
		log_fatal("\nCAPTURE IMAGE SUCCESS\n");
		return 0;
	}
	log_fatal("\nCAPTURE IMAGE FAILED (2)\n");
	return -1;
}

char str_put_method[1024];
#if (USE_ENCRYPT == 1)
	void http_upload_img_encrypt(u32 stamp, u8 chanel){
		sprintf(str_put_method,"curl -v -F filename=/tmp/encrypt_%u.jpg -F upload=@/tmp/encrypt_%u.jpg %s?data=%llu-%d-%u-%.5f-%.5f\0", stamp, stamp, SystemSetting.data.upload_file_url, SystemSetting.data.DeviceID, chanel, stamp, GPS_Data.location.latitude, GPS_Data.location.longitude);
		//log_system(str_put_method);
		system(str_put_method); //upload file to server
		log_fatal("\nUPLOAD IMAGE TO SERVER\n");
	}
#else
	void http_upload_img(u32 stamp, u8 chanel){
		sprintf(str_put_method,"curl -v -F filename=/tmp/%u.jpg -F upload=@/tmp/%u.jpg %s?data=%llu-%d-%u-%.5f-%.5f\0", stamp, stamp, SystemSetting.data.upload_file_url, SystemSetting.data.DeviceID, chanel, stamp, GPS_Data.location.latitude, GPS_Data.location.longitude);
		//log_system(str_put_method);
		system(str_put_method); //upload file to server
		log_fatal("\nUPLOAD IMAGE TO SERVER\n");
	}
#endif  /* USE_ENCRYPT */

void setting_number_plate(char * plate){//ok
	u8 i = 0;
	for(i = 0; i < 8; i++){
		//capture image
		if((DeviceSetting.camera_list[i].active == true) && (DeviceSetting.camera_list[i].block != true)){
			sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>2</id><enabled>true</enabled><positionX>10</positionX><positionY>96</positionY><displayText>Bien so: %s</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/\0",
					plate, (11 + i));//500
			system(str_put_method);
			usleep(100000);
		}else if(DeviceSetting.camera_list[i].block == true){
			sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>1</id><enabled>true</enabled><positionX>10</positionX><positionY>96</positionY><displayText>PHAT HIEN CAMERA GIA MAO</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text\0", (11 + i));//500
			system(str_put_method);
			usleep(100000);
		}
	}
}

void setting_provider(char *provider){//ok
	u8 i = 0;
	for(i = 0; i < 8; i++){
		//capture image
		if((DeviceSetting.camera_list[i].active == true) && (DeviceSetting.camera_list[i].block != true)){
			sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>1</id><enabled>true</enabled><positionX>10</positionX><positionY>600</positionY><displayText>%s</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/\0", provider, (11 + i));//500
			system(str_put_method);
			usleep(100000);
		}
	}
}

//void setting_number_plate(char * plate){
//	u8 i = 0;
//	for(i = 0; i < 8; i++){
//		//capture image
//		if((DeviceSetting.camera_list[i].active == true) && (DeviceSetting.camera_list[i].block != true)){
//			sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>1</id><enabled>true</enabled><positionX>10</positionX><positionY>128</positionY><displayText>Bien so: %s</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/1\0",
//					plate, (11 + i));//500
//			system(str_put_method);
//			usleep(100000);
//		}else if(DeviceSetting.camera_list[i].block == true){
//			sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>1</id><enabled>true</enabled><positionX>10</positionX><positionY>128</positionY><displayText>PHAT HIEN CAMERA GIA MAO</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/1\0", (11 + i));//500
//			system(str_put_method);
//			usleep(100000);
//		}
//	}
//}

void update_realtime_overlay(loc_def this_loc, u8 speed, u8 chanel){//ok
	sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>3</id><enabled>true</enabled><positionX>10</positionX><positionY>64</positionY><displayText>Toa do: %0.5f %0.5f %dKM/h</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/\0",
			this_loc.latitude, this_loc.longitude, speed, (10 + chanel));//480
	system(str_put_method);
}

//void update_realtime_overlay(loc_def this_loc, u8 speed, u8 chanel){
//	sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>2</id><enabled>true</enabled><positionX>10</positionX><positionY>96</positionY><displayText>Toa do: %0.5f %0.5f %dKM/h</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/2\0",
//			this_loc.latitude, this_loc.longitude, speed, (10 + chanel));//480
//	system(str_put_method);
//}

void cam_setting_driver_info(char * name, char * license){//ok
	u8 i = 0;
	for(i = 0; i < 8; i++){
		//capture image
		if((DeviceSetting.camera_list[i].active == true) && (DeviceSetting.camera_list[i].block != true)){
			sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>4</id><enabled>true</enabled><positionX>10</positionX><positionY>32</positionY><displayText>Lai xe: %s %s</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/\0",
					name, license, (11 + i));
			system(str_put_method);
			usleep(100000);
		}else if(DeviceSetting.camera_list[i].block == true){
			sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>4</id><enabled>true</enabled><positionX>10</positionX><positionY>32</positionY><displayText>PHAT HIEN CAMERA GIA MAO</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/\0", (11 + i));
			system(str_put_method);
			usleep(100000);
		}
	}
}

//void cam_setting_driver_info(char * name, char * license){
//	u8 i = 0;
//	for(i = 0; i < 8; i++){
//		//capture image
//		if((DeviceSetting.camera_list[i].active == true) && (DeviceSetting.camera_list[i].block != true)){
//			sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlayList><TextOverlay><id>3</id><enabled>true</enabled><positionX>10</positionX><positionY>64</positionY><displayText>Ten LX: %s</displayText></TextOverlay><TextOverlay><id>4</id><enabled>true</enabled><positionX>10</positionX><positionY>32</positionY><displayText>GPLX: %s</displayText></TextOverlay></TextOverlayList>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/\0",
//					name, license, (11 + i));
//			system(str_put_method);
//			usleep(100000);
//		}else if(DeviceSetting.camera_list[i].block == true){
//			sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlayList><TextOverlay><id>3</id><enabled>true</enabled><positionX>10</positionX><positionY>64</positionY><displayText>PHAT HIEN CAMERA GIA MAO</displayText></TextOverlay><TextOverlay><id>4</id><enabled>true</enabled><positionX>10</positionX><positionY>32</positionY><displayText>PHAT HIEN CAMERA GIA MAO</displayText></TextOverlay></TextOverlayList>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/\0", (11 + i));
//			system(str_put_method);
//			usleep(100000);
//		}
//	}
//}

void init_cam(void){
	int i = 0;
	memset(&DeviceSetting.camera_list, 0x0, sizeof(DeviceSetting.camera_list));
	for(i = 0; i < 8; i++){
		sprintf(DeviceSetting.camera_list[i].camera_ip, "192.168.1.%d\0", 11 + i);
		if(((DeviceSetting.CameraNumber >> i) & 0x01) == 1){
			DeviceSetting.camera_list[i].active = 1;
		}
	}
}

u8 get_list_camera_sn(char *out){
	int i = 0;
	u8 pos = 0;
	u8 num = 0;
	for(i = 0; i < 8; i++){
		if(DeviceSetting.camera_list[i].detect == true){
			if(pos != 0){
				out[pos] = '&';
				pos = pos + 1;
			}
			memcpy(out + pos, DeviceSetting.camera_list[i].camera_sn, 9);
			pos = pos + 9;
			num = num + 1;
		}
	}
	return num;
}

u8 get_list_camera_index(char *out){
	sprintf(out, "1: %d %s\n2: %d %s\n3: %d %s\n4: %d %s\n5: %d %s\n6: %d %s\n7: %d %s\n8: %d %s\n", DeviceSetting.camera_list[0].detect, DeviceSetting.camera_list[0].camera_sn, DeviceSetting.camera_list[1].detect, DeviceSetting.camera_list[1].camera_sn, DeviceSetting.camera_list[2].detect, DeviceSetting.camera_list[2].camera_sn, DeviceSetting.camera_list[3].detect, DeviceSetting.camera_list[3].camera_sn, DeviceSetting.camera_list[4].detect, DeviceSetting.camera_list[4].camera_sn, DeviceSetting.camera_list[5].detect, DeviceSetting.camera_list[5].camera_sn, DeviceSetting.camera_list[6].detect, DeviceSetting.camera_list[6].camera_sn, DeviceSetting.camera_list[7].detect, DeviceSetting.camera_list[7].camera_sn);
}

//FAKE CAMERA
void update_realtime_overlay_for_fake_camera(u8 chanel){
	sprintf(str_put_method,	"curl -H 'Content-Type:application/xml' -X PUT -d '<TextOverlay><id>2</id><enabled>true</enabled><positionX>10</positionX><positionY>96</positionY><displayText>PHAT HIEN CAMMERA GIA MAO</displayText></TextOverlay>' http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Video/inputs/channels/1/overlays/text/2\0", (10 + chanel));
	system(str_put_method);
}

void reboot_camera(void){
	u8 i = 0;
	if(SystemSetting.data.taxi_mode == 1) return;
	GPS_Data.cam_up_time = 0;
	for(i = 0; i < 8; i++){
		memset(DeviceSetting.camera_list[i].camera_sn, 0x0, 32);
		DeviceSetting.camera_list[i].detect = false;
		//capture image
		//if(DeviceSetting.camera_list[i].detect == true){
		sprintf(str_put_method,	"curl -X PUT http://admin:bk9108vn@192.168.1.%d/ISAPI/System/reboot\0", (11 + i));
		system(str_put_method);
		usleep(100000);
		//}
	}

	sleep(1);
	//>CAMRST
	if(mcu_send_and_rcv(">CAMRST", strlen(">CAMRST"), mcu_response, &mcu_response_len, 5000) == 0){
		if(!strncmp(mcu_response, "OK", 2)){
			log_system("RESET CAMERA SUCCSESS!!!\n");
		}
	}

	GPS_Data.no_acc_time = 0;
	GPS_Data.flg_scan_cam = true;
	GPS_Data.time_to_send_evt = 0;
	flg_cam_on = 1;
	clear_camera_cnt();
}

int change_ip_camera(u8 src, u8 des){
	char temp[128];
	sprintf(str_put_method, "http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Network/interfaces/1/ipAddress", (10 + src));
	sprintf(temp, "/tmp/%d.xml", (10 + src));

	if(!get_xml_file(str_put_method, temp)){
		//create new file with new ip
		sprintf(str_put_method, "sed 's/192.168.1.%d/192.168.1.%d/g' /tmp/%d.xml > /tmp/%d.xml", (10 + src), (10 + des), (10 + src), (10 + des));
		system(str_put_method);

		//change new ip
		sprintf(str_put_method, "curl -X PUT -d @/tmp/%d.xml \"http://admin:bk9108vn@192.168.1.%d/ISAPI/System/Network/interfaces/1/ipAddress\"", (10 + des), (10 + src));
		system(str_put_method);

		//reboot camera
		sprintf(str_put_method,	"curl -X PUT http://admin:bk9108vn@192.168.1.%d/ISAPI/System/reboot\0", (10 + src));
		system(str_put_method);
	}

	return -1;
}
