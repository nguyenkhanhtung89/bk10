#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL, LOG_NW, LOG_GPS, LOG_WRITEFILE, LOG_SYSTEM};

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#define log_nw(...) log_log(LOG_NW, __FILE__, __LINE__, __VA_ARGS__)
#define log_gps(...) log_log(LOG_GPS, __FILE__, __LINE__, __VA_ARGS__)
#define log_system(...) log_log(LOG_SYSTEM, __FILE__, __LINE__, __VA_ARGS__)
#define log_file(...) log_log(LOG_WRITEFILE, __FILE__, __LINE__, __VA_ARGS__)

#define DEBUG_FILENAME			"/usrdata/debug.log"

void log_log(int level, const char *file, int line, const char *fmt, ...);

void show_debug_app(void);

void disable_debug_app(void);

void LogDate(void);

#endif
