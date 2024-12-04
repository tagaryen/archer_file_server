#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>


#include "List.h"


#define LOG_FILE_PATH_LEN             (MAX_PATH_LEN + DATE_TIME_LEN)
#define MAX_LEVEL_LEN                 7
#define DATE_TIME_LEN                 11
#define DATE_SEC_TIME_LEN             20
#define FILE_NAME_AND_LINE_LEN        512
#define MAX_PATH_LEN                  1024

enum { LOG_LEVEL_NONE, LOG_LEVEL_TRACE, LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR, LOG_LEVEL_FATAL };

typedef struct Logger_st {
    char            log_root_path[MAX_PATH_LEN];
    int             log_root_path_len;
    int             log_default_level;
    int             log_running;
    pthread_mutex_t log_mutex;
    pthread_cond_t  log_cond;
    pthread_t       log_thread;
    List           *log_event_list;
} Logger;

#ifdef __cplusplus
extern "C" {
#endif

Logger * logger_new();
Logger * logger_new_with_path_level(const char *log_path, const int lv);
void logger_set_path_level(Logger *logger, const char *log_path, const int lv);
void logger_free(Logger *logger);
void logger_log(Logger *logger, int lv, const char *filename, const int line, const char *fmt, ...);
void logger_console(int lv, const char *fmt, ...);

#define console_out(...) logger_console(LOG_LEVEL_INFO, __VA_ARGS__)
#define console_warn(...) logger_console(LOG_LEVEL_WARN, __VA_ARGS__)
#define console_err(...) logger_console(LOG_LEVEL_ERROR, __VA_ARGS__)


#ifdef __cplusplus
}
#endif


