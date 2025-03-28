#include <ql_oe.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "log.h"
#include "mcu.h"
#include "serial.h"
#include "gps.h"

static u8 show_debug = 0;

void log_log(int level, const char *file, int line, const char *fmt, ...){
	if((level < LOG_SYSTEM) && (show_debug == 0)){
		return;
	}

	char buf_ct[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buf_ct, fmt, args);
	DEBUG_APP__(buf_ct, strlen(buf_ct));

	if(level == LOG_WRITEFILE ){
		FILE *pLog = fopen(DEBUG_FILENAME,"a");
		if(pLog == NULL)
			return;
		va_list args;
		va_start(args,fmt);
		vfprintf(pLog, fmt, args);
		fputc('\n',pLog);
		va_end(args);
		fclose(pLog);
	}
}

void LogDate(void)
{
#if logType == logfileConsole
	log_debug("\nLog File Debug Data %d/%d/%d, Time %d:%d:%d",GPS_Data.sys_time->tm_mday, GPS_Data.sys_time->tm_mon, GPS_Data.sys_time->tm_year+1900,GPS_Data.sys_time->tm_hour, GPS_Data.sys_time->tm_min,GPS_Data.sys_time->tm_sec);
#else
	log_file("\nLog File Debug Data %d/%d/%d, Time %d:%d:%d",GPS_Data.sys_time->tm_mday, GPS_Data.sys_time->tm_mon, GPS_Data.sys_time->tm_year+1900,GPS_Data.sys_time->tm_hour, GPS_Data.sys_time->tm_min,GPS_Data.sys_time->tm_sec);
#endif
}

void show_debug_app(void){
	if(show_debug == 0){
		show_debug = 1;
		DEBUG_APP__("ENABLE DEBUG\n", 13);
	}else{
		show_debug = 0;
		DEBUG_APP__("DISABLE DEBUG\n", 14);
	}
}

void disable_debug_app(void){
	show_debug = 0;
}

