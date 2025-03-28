#include "store.h"
#include "log.h"
#include "utils.h"

int write_data(char* full_path, u8* data, u16 len){
	u32 idx = 0;
	FILE* fp;

	fp = fopen(full_path, "w");
	if(NULL == fp){
		fclose(fp);
		return -1;
	}
	/* Write data */
	for(idx = 0; idx < len; idx ++){
		fwrite(data + idx, 1, 1, fp);
	}
	fclose(fp);
	return 0;
}

int read_data(char* full_path, u8* desc, u16* rlen){
	FILE *f = fopen(full_path, "r");
	if(NULL == f)return -1;
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	fread(desc, 1, fsize, f);
	fclose(f);

	desc[fsize] = 0;
	*rlen = fsize;
	return 0;
}

int write_string(char* full_path, char* data){
	u32 idx = 0;
	FILE* fp;
	u16 len = strlen(data);

	fp = fopen(full_path, "w");
	if(NULL == fp){
		fclose(fp);
		return -1;
	}
	/* Write data */
	for(idx = 0; idx < len; idx ++){
		fwrite(data + idx, 1, 1, fp);
	}
	fclose(fp);
	return 0;
}

int read_string(char* full_path, char * desc){
	FILE *f = fopen(full_path, "r");
	if(NULL == f)return -1;
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	fread(desc, 1, fsize, f);
	fclose(f);

	desc[fsize] = 0;
	if(strlen(desc) == 0) return -1;
	return 0;
}

void write_number(char* full_path, u64 num){
	u8 buff[64];
	sprintf(buff, "%llu\0", num);

	write_string(full_path, buff);
}

u64 read_number(char* full_path){
	u8 buff[64];

	if(read_string(full_path, buff) < 0) return 0;

	return ec21_atoi(buff);
}

int read_float(char* full_path, float* desc){
	u8 buff[64];

	if(read_string(full_path, buff) < 0) return -1;

	sscanf(buff, "%f", desc);

	return 0;
}

void write_float(char* full_path, float val){
	char buff[64];
	sprintf(buff, "%.2f\0", val);

	write_string(full_path, buff);
}

