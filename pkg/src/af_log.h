//
// log.h -- Afero logging definitions
//
// Clif Liu
//
// Copyright (c) 2015 Afero, Inc. All rights reserved.
//

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdint.h>
#include <syslog.h>

extern uint32_t g_debugLevel;


#define LOG_DEBUG_OFF 0
#define LOG_DEBUG1    1
#define LOG_DEBUG2    2
#define LOG_DEBUG3    3
#define LOG_DEBUG4    4

#define AFLOG_INFO(_fmt, ...) syslog(LOG_INFO,"I "_fmt,##__VA_ARGS__)
#define AFLOG_NOTICE(_fmt, ...) syslog(LOG_NOTICE,"N "_fmt, ##__VA_ARGS__)
#define AFLOG_WARNING(_fmt, ...) syslog(LOG_WARNING,"W "_fmt, ##__VA_ARGS__)
#define AFLOG_ERR(_fmt, ...) syslog(LOG_ERR,"E "_fmt,##__VA_ARGS__)
#define AFLOG_CRIT(_fmt, ...) syslog(LOG_CRIT,"C "_fmt,##__VA_ARGS__)
#define AFLOG_DEBUG1(_fmt, ...) if (g_debugLevel>=LOG_DEBUG1) syslog(LOG_DEBUG,"1 "_fmt,##__VA_ARGS__)
#ifdef BUILD_TARGET_RELEASE
#define AFLOG_DEBUG4(_fmt, ...)
#define AFLOG_DEBUG3(_fmt, ...)
#define AFLOG_DEBUG2(_fmt, ...)
#else
#define AFLOG_DEBUG4(_fmt, ...) if (g_debugLevel>=LOG_DEBUG4) syslog(LOG_DEBUG,"4 "_fmt,##__VA_ARGS__)
#define AFLOG_DEBUG3(_fmt, ...) if (g_debugLevel>=LOG_DEBUG3) syslog(LOG_DEBUG,"3 "_fmt,##__VA_ARGS__)
#define AFLOG_DEBUG2(_fmt, ...) if (g_debugLevel>=LOG_DEBUG2) syslog(LOG_DEBUG,"2 "_fmt,##__VA_ARGS__)
#endif


#define AFLOG_DEBUG_ENABLED()  (g_debugLevel >= LOG_DEBUG1)

void af_log_buffer(uint32_t level, char *name, uint8_t *buffer, int bufLen);

void af_util_convert_data_to_hex_with_name(char *name, uint8_t *data, int dataLen, char *buf, int bufLen);

#endif // __LOG_H__

