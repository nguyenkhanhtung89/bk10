#include <sys/reboot.h>
#include "utils.h"
#include "log.h"
#include "ql_powerdown.h"
#include <ql_oe.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include "driver.h"
#include "network.h"
#include <curl/curl.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

int check_url(char *url){
	int result = -1;
	CURL *curl;
	CURLcode response;

	curl = curl_easy_init();

	if(curl){
		char *ct;
		curl_easy_setopt(curl, CURLOPT_URL, url);

		/* don't write output to stdout */
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

		/* Perform the request */
		response = curl_easy_perform(curl);
		if(response == CURLE_OK){
			response = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);

			if((CURLE_OK == response) && ct){
				//log_debug("CURL RESPONSE -> %s\n", ct);
				if(!strncmp(ct, "text/plain", 10) || !strncmp(ct, "application/x-zip-compressed", 28) || !strncmp(ct, "audio/wav", 9) || !strncmp(ct, "application/octet-stream", 24)){
					result = 0;
				}
			}
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return result;
}

static void get_element_names(xmlNode * a_node, xmlDocPtr file, char * camsn){
   xmlNode *cur_node = NULL;
   xmlChar * key;
   int key_len = 0;
   for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
      if (cur_node->type == XML_ELEMENT_NODE) {
         //printf("node type: Element, name: %s\n", cur_node->name);
         if(!xmlStrcmp(cur_node->name, (const xmlChar *)"serialNumber")){
         	key = xmlNodeListGetString(file, cur_node->xmlChildrenNode, 1);
         	key_len = strlen(key);
         	if(key_len > 18){
         		memcpy(camsn, key + (key_len - 9), 9);
         		//printf("serialNumber: %s\n", camsn);
         	}
            xmlFree(key);
         }
      }
      get_element_names(cur_node->children, file, camsn);
   }
}

static int key_parse_xml(char *xml_fname, char *parse_node, char *out){
   xmlDoc *doc = NULL;
   xmlNode *root_element = NULL;

   LIBXML_TEST_VERSION
   /*parse the file and get the DOM */
   if ((doc = xmlReadFile(xml_fname, NULL, 0)) == NULL){
      return -1;
   }

   /*Get the root element node */
   root_element = xmlDocGetRootElement(doc);
   get_element_names(root_element, doc, out);
   xmlFreeDoc(doc);       // free document
   xmlCleanupParser();    // Free globals
   return 0;
}

int get_camera_serial_number(char* url, char* file_name, char * CamSN){
	CURL* easyhandle = curl_easy_init();

	curl_easy_setopt( easyhandle, CURLOPT_URL, url ) ;

	FILE* file = fopen( file_name, "w");

	curl_easy_setopt( easyhandle, CURLOPT_WRITEDATA, file) ;

	curl_easy_perform( easyhandle );

	curl_easy_cleanup( easyhandle );

	fclose(file);

	if(key_parse_xml(file_name, "serialNumber", CamSN) < 0){
		unlink(file_name);
		return -1;
	}
	unlink(file_name);

	if(strlen(CamSN) >= 9){
		return 0;
	}
	return -1;
}

int camera_verify(u8 *SN){
	//http://g3.etec.vn/api/CheckCam?DataCheck=F002121-10000212
}

int get_xml_file(char *url, char *fname){
	CURL* easyhandle = curl_easy_init();

	unlink(fname);

	curl_easy_setopt( easyhandle, CURLOPT_URL, url ) ;

	FILE* file = fopen( fname, "w");

	curl_easy_setopt( easyhandle, CURLOPT_WRITEDATA, file) ;

	curl_easy_perform( easyhandle );

	curl_easy_cleanup( easyhandle );

	fclose(file);

	if(access(fname, F_OK) < 0){
		return -1;
	}
	return 0;
}

const u32 Crc32Table[] = {
	0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
	0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD,
	0x4C11DB70,0x48D0C6C7,0x4593E01E,0x4152FDA9,0x5F15ADAC,0x5BD4B01B,0x569796C2,0x52568B75,
	0x6A1936C8,0x6ED82B7F,0x639B0DA6,0x675A1011,0x791D4014,0x7DDC5DA3,0x709F7B7A,0x745E66CD,
	0x9823B6E0,0x9CE2AB57,0x91A18D8E,0x95609039,0x8B27C03C,0x8FE6DD8B,0x82A5FB52,0x8664E6E5,
	0xBE2B5B58,0xBAEA46EF,0xB7A96036,0xB3687D81,0xAD2F2D84,0xA9EE3033,0xA4AD16EA,0xA06C0B5D,
	0xD4326D90,0xD0F37027,0xDDB056FE,0xD9714B49,0xC7361B4C,0xC3F706FB,0xCEB42022,0xCA753D95,
	0xF23A8028,0xF6FB9D9F,0xFBB8BB46,0xFF79A6F1,0xE13EF6F4,0xE5FFEB43,0xE8BCCD9A,0xEC7DD02D,
	0x34867077,0x30476DC0,0x3D044B19,0x39C556AE,0x278206AB,0x23431B1C,0x2E003DC5,0x2AC12072,
	0x128E9DCF,0x164F8078,0x1B0CA6A1,0x1FCDBB16,0x018AEB13,0x054BF6A4,0x0808D07D,0x0CC9CDCA,
	0x7897AB07,0x7C56B6B0,0x71159069,0x75D48DDE,0x6B93DDDB,0x6F52C06C,0x6211E6B5,0x66D0FB02,
	0x5E9F46BF,0x5A5E5B08,0x571D7DD1,0x53DC6066,0x4D9B3063,0x495A2DD4,0x44190B0D,0x40D816BA,
	0xACA5C697,0xA864DB20,0xA527FDF9,0xA1E6E04E,0xBFA1B04B,0xBB60ADFC,0xB6238B25,0xB2E29692,
	0x8AAD2B2F,0x8E6C3698,0x832F1041,0x87EE0DF6,0x99A95DF3,0x9D684044,0x902B669D,0x94EA7B2A,
	0xE0B41DE7,0xE4750050,0xE9362689,0xEDF73B3E,0xF3B06B3B,0xF771768C,0xFA325055,0xFEF34DE2,
	0xC6BCF05F,0xC27DEDE8,0xCF3ECB31,0xCBFFD686,0xD5B88683,0xD1799B34,0xDC3ABDED,0xD8FBA05A,
	0x690CE0EE,0x6DCDFD59,0x608EDB80,0x644FC637,0x7A089632,0x7EC98B85,0x738AAD5C,0x774BB0EB,
	0x4F040D56,0x4BC510E1,0x46863638,0x42472B8F,0x5C007B8A,0x58C1663D,0x558240E4,0x51435D53,
	0x251D3B9E,0x21DC2629,0x2C9F00F0,0x285E1D47,0x36194D42,0x32D850F5,0x3F9B762C,0x3B5A6B9B,
	0x0315D626,0x07D4CB91,0x0A97ED48,0x0E56F0FF,0x1011A0FA,0x14D0BD4D,0x19939B94,0x1D528623,
	0xF12F560E,0xF5EE4BB9,0xF8AD6D60,0xFC6C70D7,0xE22B20D2,0xE6EA3D65,0xEBA91BBC,0xEF68060B,
	0xD727BBB6,0xD3E6A601,0xDEA580D8,0xDA649D6F,0xC423CD6A,0xC0E2D0DD,0xCDA1F604,0xC960EBB3,
	0xBD3E8D7E,0xB9FF90C9,0xB4BCB610,0xB07DABA7,0xAE3AFBA2,0xAAFBE615,0xA7B8C0CC,0xA379DD7B,
	0x9B3660C6,0x9FF77D71,0x92B45BA8,0x9675461F,0x8832161A,0x8CF30BAD,0x81B02D74,0x857130C3,
	0x5D8A9099,0x594B8D2E,0x5408ABF7,0x50C9B640,0x4E8EE645,0x4A4FFBF2,0x470CDD2B,0x43CDC09C,
	0x7B827D21,0x7F436096,0x7200464F,0x76C15BF8,0x68860BFD,0x6C47164A,0x61043093,0x65C52D24,
	0x119B4BE9,0x155A565E,0x18197087,0x1CD86D30,0x029F3D35,0x065E2082,0x0B1D065B,0x0FDC1BEC,
	0x3793A651,0x3352BBE6,0x3E119D3F,0x3AD08088,0x2497D08D,0x2056CD3A,0x2D15EBE3,0x29D4F654,
	0xC5A92679,0xC1683BCE,0xCC2B1D17,0xC8EA00A0,0xD6AD50A5,0xD26C4D12,0xDF2F6BCB,0xDBEE767C,
	0xE3A1CBC1,0xE760D676,0xEA23F0AF,0xEEE2ED18,0xF0A5BD1D,0xF464A0AA,0xF9278673,0xFDE69BC4,
	0x89B8FD09,0x8D79E0BE,0x803AC667,0x84FBDBD0,0x9ABC8BD5,0x9E7D9662,0x933EB0BB,0x97FFAD0C,
	0xAFB010B1,0xAB710D06,0xA6322BDF,0xA2F33668,0xBCB4666D,0xB8757BDA,0xB5365D03,0xB1F740B4
};

u32 CRC32Software(u8* pData, long Length){
	u32 nReg;
	u32 nTemp = 0;
	long i, n;
	nReg = 0xFFFFFFFF;
	for (n = 0; n < Length; n++){
		nReg ^= (u32)pData[n];
		for (i = 0; i < 4; i++){
			nTemp = Crc32Table[(nReg >> 24) & 0xff];
			nReg <<= 8;
			nReg ^= nTemp;
		}
	}
	return nReg;
}

u32 FileCRC32Software(u8* FileName){
	u32 nReg;
    u8 *finfo;
    size_t n = 0;
    int c;

	FILE *file = fopen(FileName, "r");
	if (file == NULL) return 0;
    fseek(file, 0, SEEK_END);

    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    finfo = malloc(f_size);

    //hex file convert to array
    while ((c = fgetc(file)) != EOF){
    	finfo[n++] = (u8)c;
    }
    fclose(file);

    nReg = CRC32Software(finfo, f_size);
    free(finfo);

    return nReg;
}

const u8 CRC_Table[] = {0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126,
    32, 163, 253, 31, 65, 157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189,
    62, 96, 130, 220, 35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128,
    222, 60, 98, 190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67,
    161, 255, 70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187,
    89, 7, 219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196,
    154, 101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122,
    36, 248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139, 87,
    9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22, 233,
    183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168, 116,
    42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53};

u8 CRC_8BitsCompute(u8* data, u32 size){
  /* Reset CRC data register to avoid overlap when computing new data stream */
  u8 CRC_Data = 0;

  while (size--) {
    CRC_Data = CRC_Table[*(data++) ^ CRC_Data];
  }

  /* Return the CRC value */
  return CRC_Data;
}

u16 cal_sum16_buffer(u8 *src, u16 len){
	u32 sum = 0;
	u16 idx = 0;

	for(idx = 0; idx < len; idx++){
		sum += src[idx];
	}

	return (sum & 0xFFFF);
}

u8 cal_sum_str(char *src){
	u32 sum = 0;
	u16 idx = 0;
	u16 len = strlen(src);

	for(idx = 0; idx < len; idx++){
		sum += src[idx];
	}

	return (sum & 0xFF);
}

int str_find(char *src, char *dest){
	char * pch;
	int idx = -1;

	pch = strstr(src, dest);
	if(pch != NULL){
		idx = pch - src;
	}

	return idx;
}

void set_tz_gmt_hn(void){
	int    ret     = E_QL_OK;
	char stimezone[7] = {0};
	time_t rawtime;
	struct tm *info;
	char buffer[80];

	sprintf(stimezone, "GMT-7\0");
	ret = setenv("TZ", stimezone, 1);
	tzset();
	log_system("timezone=%d\n", timezone);
	log_system("tzname[0]=%s, tzname[1]=%s\n",tzname[0], tzname[1]);
	log_system("setenv %s ret = %d\n", stimezone, ret);

	time( &rawtime );
	info = localtime( &rawtime );
	log_system("Current time: %s\n", asctime(info));
}

u64 ec21_atoi(u8* s){
	char *eptr;
	return strtoll(s, &eptr, 10);
}

void ec21_reboot(void){
	sync_tibase();
	sync();
	sleep(1);
	Ql_Powerdown(1);
}

u8 check_phone_valid(char* phone){
	u8 idx = 0;
	u8 len = strlen(phone);
	for(idx = 0; idx < len; idx++){
		if((phone[idx] < 43) || (phone[idx] > 57)){
			return 0;
		}
	}
	return 1;
}

u8 byte_arr_cmp(u8 *src, u8 *dest, u8 size){
	u8 idx;

	for(idx = 0; idx < size; idx ++){
		if(src[idx] != dest[idx]) return 0;
	}

	return 1;
}

int char_index_of(char *src, char ch, u8 count){
	u16 idx	= 0;
	u16 clen	= strlen(src);
	u8 cnt   = 0;

	for(; idx < clen; idx++){
		if(src[idx] == ch){
			cnt ++;
			if(cnt == count){
			  return idx;
			}
		}
	}
	return -1;
}

uint32_t unix_time_in_seconds( uint8_t sec, uint8_t min, uint8_t hrs, uint8_t day, uint8_t mon, uint16_t year){
	uint32_t ts = 0;
	uint16_t y_k;
	//  Add up the seconds from all prev years, up until this year.
	uint8_t years = 0;
	uint8_t leap_years = 0;
	uint8_t mo_k = 0;
	for(y_k = EPOCH_YEAR; y_k < year; y_k++){
		if( IS_LEAP_YEAR( y_k )){
			leap_years++;
		}else{
			years++;
		}
	}
	ts += ((years*days_per_year[0]) + (leap_years*days_per_year[1])) * SEC_PER_DAY;

	//  Add up the seconds from all prev days this year, up until today.
	uint8_t year_index = (IS_LEAP_YEAR( year )) ? 1 : 0;
	for( mo_k = 0; mo_k < (mon-1); mo_k++ ){ //  days from previous months this year
		ts += days_per_month[ year_index ][ mo_k ] * SEC_PER_DAY;
	}
	ts += (day-1) * SEC_PER_DAY; // days from this month

	//  Calculate seconds elapsed just today.
	ts += hrs * SEC_PER_HOUR;
	ts += min * SEC_PER_MIN;
	ts += sec;

	return ts;
}

int file_download(char *link, char *dir ,char *name){
	char cmdline[256];
	char cmdline_1[256];
	if(check_internet()){//check internet available
		//step 1: check file and remove
		mk_dir__(dir);

		if(!check_url(link)){
			log_debug("the URL: %s AVAILABLE\n", link);
		}else{
			log_debug("the URL: %s is NOT AVAILABLE\n", link);
			return -3;
		}

		/*
		//step 2: download file
		memset(cmdline, 0x0, 256);
		sprintf(cmdline, "wget -NS %s %s", dir, link);
		system(cmdline); //download file to dir
		log_fatal(cmdline);
		*/

		memset(cmdline, 0x0, 256);
		sprintf(cmdline, "curl -o %s%s %s > %s", dir, name, link, name);
		system(cmdline); //download file to dir
		log_debug(cmdline);
		//curl -o /usrdata/firmware/bk88g.zip http://fw.dvbk.vn/bk88g/bk88.zip > bk88.zip

		//step 3: check exist dowloaded file
		memset(cmdline, 0x0, 256);
		sprintf(cmdline, "%s%s", dir, name);

		if(access(cmdline, F_OK) == 0){//check file in dir
			//step 3: check exist after download file
			log_debug("\nDOWNLOAD FILE %s SUCCESS\n", link);
			return 0;
		}
		log_debug("\nDOWNLOAD FILE %s ERROR\n", link);
		return -1;
	}
	log_debug("\nDOWNLOAD FILE %s ERROR => NO INTERNET\n", link);
	return -2;
}

int unzip_file(char *zip_dir, char *zip_name, char *unzip_dir, char *unzip_name){
	char cmdline[256];
	//step 1: check exist zip file
	memset(cmdline, 0x0, 256);
	sprintf(cmdline, "%s%s", zip_dir, zip_name);

	if(access(cmdline, F_OK) == 0){//check zip file in dir
		//step 3: unzip file
		memset(cmdline, 0x0, 256);
		sprintf(cmdline, "unzip -o %s%s -d %s", zip_dir, zip_name, unzip_dir);
		system(cmdline);
		log_fatal(cmdline);

		memset(cmdline, 0x0, 256);
		sprintf(cmdline, "%s%s", unzip_dir, unzip_name);

		if(access(cmdline, F_OK) == 0){//check file unzip
			log_fatal("EXTRACT FILE %s SUCCESS\n", cmdline);
			memset(cmdline, 0x0, 256);
			sprintf(cmdline, "chmod 100 %s%s", unzip_dir, unzip_name);
			system(cmdline);//add full access to new extract file
			log_fatal(cmdline);

			return 0;
		}
		log_fatal("EXTRACT FILE %s ERROR\n", cmdline);
		return -1;
	}else{
		return -2;
	}
}

int write_file(char* dir, char* name, u8* data, u32 len){
	u32 idx = 0;
	FILE* fp;
	char full_path[128];
	/* make directory for store device config infomation */
	if(!mk_dir__(dir)){
		return 0;
	}

	sprintf(full_path, "%s%s\0", dir, name);
	fp = fopen(full_path, "w");
	if(NULL == fp){
		return -1;
	}
	/* Write data */
	for(idx = 0; idx < len; idx ++){
		fwrite(data+idx, 1, 1, fp);
	}

	fclose(fp);

	return 0;
}

int mk_dir__(char* dir_name){
	int e;
	struct stat sb;

	e = stat(dir_name, &sb);
	if(e == 0){
		if (sb.st_mode & S_IFDIR)
			log_trace("%s is a directory.\n",dir_name);

		if (sb.st_mode & S_IFREG){
			log_trace("%s is a regular file.\n",dir_name);
			return 0;
		}
	}else{
		if(errno = ENOENT){
			char cmd[256];
			sprintf(cmd, "mkdir -p %s", dir_name);
			if(system(cmd) < 0){
				return 0;
			}
		}
	}
	return 1;
}

void strim(u8 *src){
	u16 len = 0;
	u16 idx = 0;
	u8 flg= 0;

	len = strlen((char*)src);
	if(len == 0){
		return;
	}
	/* Remove 0x0D and 0x0A end string */
	for(idx = 0; idx < len; idx++){
		if((src[idx] == 0x0D) || (src[idx] == 0x0A) || (src[idx] == 0x20)){
			src[idx] = 0;
			flg = 1;
		}else if(flg == 1){
			src[idx] = 0;
		}
	}
}

int read_text_file(char *FileName, char *des){
	struct stat st;
	FILE* fp;
	int size_;

	if(access(FileName, F_OK) != 0) return -2;

	fp = fopen(FileName, "r");
	if(NULL == fp){/* open file error */
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	memset(des, 0x0, sizeof(des));
	size_ = fread (des, 1 , fsize, fp);
	fclose(fp);
	return size_;
}

void latlong_to_DM(double input, u8* output){
	u16 deg = 0;
	u16 min = 0;
	u16 dec = 0, dec_t = 0;

	double dif_deg = 0, min_f = 0;
	deg = (input * 10000)/10000;
	dif_deg = (double)(input - (double)deg);
	min_f = dif_deg * (double)60.0;
	min = (min_f * 10000)/10000;
	dec_t = (((double)(min_f - (double)min)) * 1000);
	dec = (dec_t * 10000)/10000;

	output[0] = (u8)deg;
	output[1] = (u8)min;
	output[2] = (u8)(dec >> 8);
	output[3] = (u8)dec & 0xFF;
}

static double degToRad(double deg) {
	return deg * (double)0.017453292519943295;
}

float diff_dist_m(float x1, float y1, float x2, float y2){
	double p1X = degToRad((double)x1);
	double p1Y = degToRad((double)y1);
	double p2X = degToRad((double)x2);
	double p2Y = degToRad((double)y2);
	double distanceInMeter;
	double temp;
	temp = sin(p1X) * sin(p2X) + cos(p1X) * cos(p2X) * cos(p2Y - p1Y);

	if(temp > 1) temp = 1;
	if(temp < -1) temp = -1;

	distanceInMeter = acos(temp)* (double)6378137;
	return distanceInMeter;
}

int cmpfunc( const void* a, const void* b){
	return ( *(int*)a - *(int*)b );
}

void ArraySort(int vals[], u8 lenght){
	u8 l = 0, exchange = 1;
	int tmp = 0;

	/* Sort tab */
	while(exchange == 1){
		exchange = 0;
		for(l = 0; l < (lenght - 1); l++){
			if(vals[l] > vals[l+1]){
				 tmp = vals[l];
				 vals[l] = vals[l+1];
				 vals[l+1] = tmp;
				 exchange=1;
			}
		}
	}
}

int char_replace(char *src, char rep, char with){
	int i=0;
	int count=0;
  while (src[i]){
    if(src[i] == rep){
			src[i] = with;
			count++;
		}
    i++;
  }
	return count;
}

int get_week_of_year(struct tm *time) {
   // struct tm time_info = *localtime(&timestamp);

    // L?y thông tin v? ngày trong nam và th? trong tu?n
    int day_of_year = time->tm_yday; // Ngày trong nam (0-365)
    int weekday = time->tm_wday;    // Th? trong tu?n (0=Ch? nh?t, 1=Th? Hai, ..., 6=Th? B?y)

    // Tìm th? Hai d?u tiên trong nam
    int first_weekday = weekday - (day_of_year % 7);
    if (first_weekday < 0) first_weekday += 7; // Ði?u ch?nh d? không âm

    int days_to_first_monday = (8 - first_weekday) % 7; // Kho?ng cách d?n th? Hai d?u tiên
    int adjusted_day_of_year = day_of_year - days_to_first_monday;

    // Ch? tính tu?n n?u dã qua ngày d?u tiên c?a tu?n d?u tiên
    if (adjusted_day_of_year < 0) {
        return 0; // Tru?c tu?n d?u tiên
    }

    // Tính tu?n, b?t d?u t? tu?n 1
    int week_of_year = (adjusted_day_of_year / 7) + 1;

    return week_of_year;
}

