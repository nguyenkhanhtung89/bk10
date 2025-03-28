#include "fw.h"
#include "utils.h"
#include "log.h"
#include "mcu.h"
#include "devconfig.h"

u8 flg_ec21_update_fw = 0;
u8 flg_ec21_check_version = 1;
static u8 link_download[320];
static u8 MCU_VERSION = 0;
int ec21_update_firmware(void){
	int temp = 0;
	int res = 0;
	FILE* fp;

	char buffer[256];
	if(flg_ec21_check_version == 1){
		memset(link_download, 0x0, 320);
		sprintf(link_download, "%s/bk10/ec21/version.txt\0", SystemSetting.data.update_fw_url);
		res = file_download(link_download, "/tmp/firmware/ec21/", "version.txt");
		if(res == 0){//tai file version thanh cong
			//check version in file txt vs this version
			memset(buffer, 0x0, 256);
			res = read_text_file("/tmp/firmware/ec21/version.txt", buffer);
			if(res < 2){
				log_debug("->OPEN FILE version.txt ERROR<-\n");
				flg_ec21_check_version = 0;
			}else{
				log_debug("->EC21 Server Ver: %s, Current Ver: %d<-\n", buffer, APP_VERSION);
				temp = ec21_atoi(buffer);
				if((APP_VERSION != temp) && (temp >= 10) && (temp < 255)){
					flg_ec21_update_fw = 1;

					log_debug("->THERE ARE NEW UPDATE FOR EC21<-\n");
				}
				flg_ec21_check_version = 0;
			}
		}else if(res == -1){
			log_debug("->DOWNLOAD version.txt ERROR<-\n");
			flg_ec21_check_version = 0;
			flg_ec21_update_fw = 0;
		}else if(res == -2){
			log_debug("->wait internet and retry<-\n");
			//wait internet and retry
			sleep(1);
		}else if(res == -3){
			flg_ec21_check_version = 0;
			flg_ec21_update_fw = 0;
			log_debug("file EC21 version.txt not found<-\n");
		}
	}else if(flg_ec21_update_fw == 1){
		memset(link_download, 0x0, 320);
		sprintf(link_download, "%s/bk10/ec21/xbkgps.zip\0", SystemSetting.data.update_fw_url);
		res = file_download(link_download, "/tmp/firmware/ec21/", "xbkgps.zip");
		if(res == 0){
			log_debug("->DOWNLOAD EC21 FIRMWARE SUCCESS!!!<-\n");

			res = unzip_file("/tmp/firmware/ec21/", "xbkgps.zip", "/tmp/firmware/ec21/", "xbkgps");
			if(res == 0){
				log_debug("unzip ok\n");
				//check firmware crc
				memset(buffer, 0x0, 256);
				res = read_text_file("/tmp/firmware/ec21/crc.txt", buffer);
				if(res < 2){
					log_debug("->EC21 crc.txt not exist or file error!!! <-\n");
					flg_ec21_update_fw = 0;
				}else{
					u32 ec_crc = ec21_atoi(buffer);
					u32 ec_file_crc = FileCRC32Software("/tmp/firmware/ec21/xbkgps");

					if(ec_file_crc == ec_crc){
						u8 loop__ = 0;

						log_debug("->EC21 crc cal: %u, file crc: %u check success <-\n", ec_crc, ec_file_crc);

						for(loop__ = 0; loop__ < 60; loop__ ++){
							if(FileCRC32Software("/tmp/firmware/ec21/xbkgps") == FileCRC32Software("/usr/bin/bkgps")){
								log_debug("UPDATE SUCCESS -> REBOOT\n");
								sleep(5);
								ec21_reboot();
								break;
							}else{
								system("cp -f /tmp/firmware/ec21/xbkgps /usr/bin/");
								usleep(500000);
								unlink("/usr/bin/bkgps");
								usleep(500000);
								chmod("/usr/bin/xbkgps", S_IXUSR);
								rename("/usr/bin/xbkgps", "/usr/bin/bkgps");
							}
							sleep(1);
						}
						flg_ec21_update_fw = 0;
						flg_ec21_check_version = 1;
						return -1;
					}else{
						log_debug("->ERROR EC21 crc cal: %u != file crc: %u !!! <-\n", ec_crc, ec_file_crc);
						flg_ec21_update_fw = 0;
					}
				}
				flg_ec21_update_fw = 0;
			}else{
				log_debug("->UNZIP ERROR!!!<-\n");
				flg_ec21_update_fw = 0;
			}
		}else{
			flg_ec21_update_fw = 0;
			log_debug("->DOWNLOAD EC21 FIRMWARE ERROR!!!<-\n");
		}
	}else{
		return 0;
	}
	return -1;
}


u8 flg_mcu_update_fw = 0;
u8 flg_mcu_check_version = 1;
u8 flg_mcu_boot_mode = 0;
u8 flg_downloaded_mcu_fw = 0;
long firmware_lenght = 0;
u32 firmware_crc = 0;
u16 firmware_nPKG = 0;
static u8 mcu_res[160] = {0x0};

int set_mcu_update_fw(void){
	u8 fw_msg_load[32];
	u8 cs = 0;
	int res = 0;
	u16 mcu_res_len = 0;

	log_debug("FW LOADING...\n");
	fw_msg_load[0] = '>';
	fw_msg_load[1] = 0xFF;
	fw_msg_load[2] = firmware_nPKG >> 8;
	fw_msg_load[3] = firmware_nPKG & 0xFF;
	fw_msg_load[4] = (firmware_crc >> 24) & 0xFF;
	fw_msg_load[5] = (firmware_crc >> 16) & 0xFF;
	fw_msg_load[6] = (firmware_crc >> 8) & 0xFF;
	fw_msg_load[7] = (firmware_crc & 0xFF);
	memset(mcu_res, 0x0, 160);
	res = mcu_send_and_rcv(fw_msg_load, 8, mcu_res, &mcu_res_len, 5000);
	log_info("~~~~firmware_nPKG = %d,firmware_crc = %u\n",firmware_nPKG, firmware_crc);
	if(res == 0){
		if(!strncmp(mcu_res, "OK", 2)){
			flg_mcu_update_fw = 0;
			flg_mcu_check_version = 0;
			log_debug("FW UPDATE START OK\n");
			return 0;
		}else{
			log_debug("FW UPDATE START ERROR\n");
		}
	}else{
		log_debug("SEND COMMAND TO MCU ERROR, RES = %d\n", res);
	}
	return -1;
}

enum_MCU_type MCU_type = UNKNOWN_MCU;
void mcu_update_firmware(void){
	int temp = 0;
	char buffer[256];
	u16 mcu_res_len = 0;
	int res = 0;
	if(flg_mcu_check_version == 1){
		if(MCU_VERSION == 0){
			memset(mcu_res, 0x0, 160);
			if(mcu_send_and_rcv(">VERSION?", strlen(">VERSION?"), mcu_res, &mcu_res_len, 5000) == 0){
				if(mcu_res[0] == 'V'){
					MCU_VERSION = mcu_res[1];
					log_debug("THIS MCU VERSION =>> %d\n", MCU_VERSION);

					if((MCU_VERSION >= 10) && (MCU_VERSION <= 99)){//STM32
						MCU_type = STM32_MCU;
					}else if((MCU_VERSION >= 100) && (MCU_VERSION <= 200)){
						MCU_type = GD32_MCU;
					}else{
						MCU_type = UNKNOWN_MCU;
						flg_mcu_update_fw = 0;
						flg_mcu_check_version = 0;
						return;
					}
				}else{
					if(!strncmp(mcu_res, "GDBOOT", 6)){
						log_debug("GD32 ALREADY IN BOOT MODE\n");
						//sprintf(MCU_Version, "BOOT");
						MCU_VERSION = 100;
						MCU_type = GD32_MCU;
					}else if(!strncmp(mcu_res, "BOOT", 4)){
						log_debug("STM32 ALREADY IN BOOT MODE\n");
						//sprintf(MCU_Version, "BOOT");
						MCU_VERSION = 10;
						MCU_type = STM32_MCU;
					}else{
						MCU_type = UNKNOWN_MCU;
						flg_mcu_update_fw = 0;
						flg_mcu_check_version = 0;
						return;
					}
				}
			}else{
				flg_mcu_check_version = 0;
			}
			return;
		}

		memset(link_download, 0x0, 320);
		if(MCU_type == STM32_MCU){
			sprintf(link_download, "%s/bk10/mcu/version.txt\0", SystemSetting.data.update_fw_url);
		}else if(MCU_type == GD32_MCU){
			sprintf(link_download, "%s/bk10/mcu/gd/version.txt\0", SystemSetting.data.update_fw_url);
		}

		res = file_download(link_download, "/tmp/firmware/mcu/", "version.txt");
		if(res == 0){//tai file version thanh cong
			//check version in file txt vs this version
			memset(buffer, 0x0, 256);
			res = read_text_file("/tmp/firmware/mcu/version.txt", buffer);
			if(res < 2){
				log_debug("READ FILE MCU version.txt ERROR\n");
				flg_mcu_check_version = 0;
			}else{
				log_debug("MCU Server Ver: %s, Current Ver: %d\n", buffer, MCU_VERSION);
				temp = ec21_atoi(buffer);

				if((MCU_VERSION != temp) && (temp >= 10) && (temp <= 200) && (MCU_VERSION >= 10) && (MCU_VERSION <= 200)){
					flg_mcu_update_fw = 1;
					log_debug("THERE ARE NEW UPDATE FOR MCU\n");
				}
				flg_mcu_check_version = 0;
			}
		}else if(res == -1){
			log_debug("DOWNLOAD version.txt ERROR\n");
			flg_mcu_check_version = 0;
			flg_mcu_update_fw = 0;
		}else if(res == -2){
			log_debug("wait internet and retry\n");
			//wait internet and retry
			sleep(1);
		}else if(res == -3){
			flg_mcu_update_fw = 0;
			flg_mcu_check_version = 0;
			log_debug("file MCU version.txt not found<-\n");
		}
	}else if(flg_mcu_update_fw == 1){
		if(flg_mcu_boot_mode == 1){
			if(firmware_lenght == 0){
				if(load_firmware_info() < 0){
					//load error
					flg_mcu_update_fw = 0;
					flg_mcu_check_version = 0;
				}
			}else{
				set_mcu_update_fw();
			}
		}else{
			if(flg_downloaded_mcu_fw == 0){
				memset(link_download, 0x0, 320);
				if(MCU_type == STM32_MCU){
					sprintf(link_download, "%s/bk10/mcu/mcufw.zip\0", SystemSetting.data.update_fw_url);
				}else if(MCU_type == GD32_MCU){
					sprintf(link_download, "%s/bk10/mcu/gd/mcufw.zip\0", SystemSetting.data.update_fw_url);
				}

				res = file_download(link_download, "/tmp/firmware/mcu/", "mcufw.zip");
				if(res == 0){
					log_debug("->DOWNLOAD MCU FIRMWARE SUCCESS!!!<-\n");

					res = unzip_file("/tmp/firmware/mcu/", "mcufw.zip", "/tmp/firmware/mcu/", "mcufw.hex");
					if(res == 0){
						u32 mcu_file_crc = FileCRC32Software("/tmp/firmware/mcu/mcufw.hex");
						log_debug("->MCU UNZIP SUCCESS!!<-\n");

						memset(buffer, 0x0, 256);
						res = read_text_file("/tmp/firmware/mcu/crc.txt", buffer);
						if(res < 2){
							log_debug("->MCU crc.txt not exist or file error!!! <-\n");
							flg_mcu_update_fw = 0;
						}else{
							u32 mcu_crc = ec21_atoi(buffer);

							if(mcu_file_crc == mcu_crc){
								log_debug("->MCU crc cal: %u, file crc: %u check success <-\n", mcu_crc, mcu_file_crc);
								flg_downloaded_mcu_fw = 1;
							}else{
								log_debug("->ERROR MCU crc cal: %u != file crc: %u !!! <-\n", mcu_crc, mcu_file_crc);
								flg_mcu_update_fw = 0;
							}
						}
					}else{
						log_debug("unzip error\n");
						flg_mcu_update_fw = 0;
					}

				}else{
					flg_mcu_update_fw = 0;
					log_debug("->DOWNLOAD MCU FIRMWARE ERROR!!!<-\n");
				}

			}else{
				/*>> enter bootmode */
				memset(mcu_res, 0x0, 160);
				if(MCU_type == STM32_MCU){
					log_debug(">> ST ENTER BOOT MODE\n");
					if(mcu_send_and_rcv(">GOTO_BOOT_MODE", strlen(">GOTO_BOOT_MODE"), mcu_res, &mcu_res_len, 5000) == 0){
						if(!strncmp(mcu_res, "BOOT", 4) || !strncmp(mcu_res, "OK", 2)){
							flg_mcu_boot_mode = 1;
						}
					}
				}else if(MCU_type == GD32_MCU){
					log_debug(">> GD ENTER BOOT MODE\n");
					if(mcu_send_and_rcv(">GD_GOTO_BOOT_MODE", strlen(">GD_GOTO_BOOT_MODE"), mcu_res, &mcu_res_len, 5000) == 0){
						if(!strncmp(mcu_res, "GDBOOT", 6) || !strncmp(mcu_res, "OK", 2)){
							flg_mcu_boot_mode = 1;
						}
					}
				}

				sleep(1);
			}
		}
	}
	check_mcu_update_timeout();
}

int load_firmware_info(void){
	u8 *finfo;
    size_t n = 0;
    int c;

	FILE *file = fopen("/tmp/firmware/mcu/mcufw.hex", "r");
	if (file == NULL){
		log_debug("FILE ERROR!!!!!");
		return -1;
	}
    fseek(file, 0, SEEK_END);

    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    finfo = malloc(f_size);

    //hex file convert to array
    while ((c = fgetc(file)) != EOF){
    	finfo[n++] = (u8)c;
    }

    firmware_crc = CRC32Software(finfo, f_size);
    free(finfo);

    firmware_lenght = f_size;

    firmware_nPKG = firmware_lenght / FW_PKG_SZ;
    if((firmware_lenght % FW_PKG_SZ) > 0){
    	firmware_nPKG += 1;
    }

    log_debug("fw nPKG = %d, fw length = %d, crc = %u\n", firmware_nPKG, firmware_lenght, firmware_crc);
    return 0;
}

u16 update_fw_tmout = 0;
int load_firmware_pkg(u16 pkg_index, u8* pkg_load, u16* pkg_len){
	if(firmware_nPKG == 0){
		if(load_firmware_info() < 0){
			//stop update
			return -1;
		}
	}

	FILE *file = fopen("/tmp/firmware/mcu/mcufw.hex", "r");
	if(file == NULL){
		return -1;
	}

	flg_mcu_update_fw = 0;
	flg_mcu_check_version = 0;

	fseek(file,(pkg_index * FW_PKG_SZ), SEEK_SET);

	if(pkg_index < firmware_nPKG - 1){
		fread(pkg_load, FW_PKG_SZ, 1, file);
		*pkg_len = FW_PKG_SZ;
	}else if(pkg_index == (firmware_nPKG - 1)){
		int epkg_len = firmware_lenght - (pkg_index * FW_PKG_SZ);

		if((epkg_len < 0) || (epkg_len > FW_PKG_SZ)){
			fclose(file);
			return -1;
		}
		fread(pkg_load, epkg_len, 1, file);

		*pkg_len = epkg_len;
	}
	update_fw_tmout = 0;
	fclose(file);
	return 0;
}

void check_mcu_update_timeout(void){
	if(firmware_nPKG > 0){
		u8 res[160];
		u16 res_len = 0;
		update_fw_tmout++;
		if(update_fw_tmout >= 15){
			update_fw_tmout = 0;
			if(mcu_send_and_rcv(">VERSION?", strlen(">VERSION?"), res, &res_len, 5000) == 0){
				if(!strncmp(res, "BOOT", 4)){
					log_debug("MCU IN BOOT MODE > 15s => REUPDATE\n");
					set_mcu_update_fw();
				}else if(res[0] == 'V'){
					firmware_nPKG = 0;
					log_debug(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>MCU UPDATE SUCCESS<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

					MCU_VERSION = 0;
					//send device config to SERVER
					GPS_Data.flg_send_cfg = 1;
					GPS_Data.time_to_send_evt = 118;
					//delete mcu firmware here
				}
			}
		}
	}
}

void set_flg_mcu_update_fw(void){
	flg_mcu_check_version = 1;
	flg_mcu_update_fw = 0;
	flg_mcu_boot_mode = 0;
	flg_downloaded_mcu_fw = 0;
	firmware_lenght = 0;
	firmware_crc = 0;
	firmware_nPKG = 0;
	MCU_VERSION = 0;
}

void set_flg_ec21_update_fw(void){
	flg_ec21_update_fw = 0;
	flg_ec21_check_version = 1;
}

