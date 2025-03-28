#ifndef __UTILS_H
#define __UTILS_H

#include <ql_oe.h>

#define SEC_PER_MIN         60
#define SEC_PER_HOUR        3600
#define SEC_PER_DAY         86400
#define MOS_PER_YEAR        12
#define EPOCH_YEAR          2020    //1970

#define PIC_DIR					"/usrdata/images/"

#define IS_LEAP_YEAR(year)  ((((year%4) == 0) && ((year%100) != 0)) || ((year%400) == 0))

static int days_per_month[2][MOS_PER_YEAR] = {
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static int days_per_year[2] = {
  365, 366
};

int check_url(char *url);

void set_tz_gmt_hn(void);

u64 ec21_atoi(u8* s);

void ec21_reboot(void);

int get_camera_serial_number(char* url, char* file_name, char * CamSN);

u8 CRC_8BitsCompute(u8* data, u32 size);

u8 byte_arr_cmp(u8 *src, u8 *dest, u8 size);

int char_index_of(char *src, char ch, u8 count);

u16 cal_sum16_buffer(u8 *src, u16 len);

u8 cal_sum_str(char *src);

u8 check_phone_valid(char* phone);

int str_find(char *src, char *dest);

int fw_file_download(char *link, char file_name);

int mk_dir__(char* dir_name);

void strim(u8 *src);

int read_text_file(char *FileName, char *des);

void latlong_to_DM(double input, u8* output);

uint32_t unix_time_in_seconds( uint8_t sec, uint8_t min, uint8_t hrs, uint8_t day, uint8_t mon, uint16_t year);

int write_file(char* dir, char* name, u8* data, u32 len);

int file_download(char *link, char *dir ,char *name);

int unzip_file(char *zip_dir, char *zip_name, char *unzip_dir, char *unzip_name);

u32 CRC32Software(u8* pData, long Length);

u32 FileCRC32Software(u8* FileName);

float diff_dist_m(float x1, float y1, float x2, float y2);

int get_xml_file(char *url, char *fname);

int camera_verify(u8 *SN);

int cmpfunc( const void* a, const void* b);

void ArraySort(int vals[], u8 lenght);

int char_replace(char *src, char rep, char with);

int get_week_of_year(struct tm *time);

#endif /* __UTILS_H */
