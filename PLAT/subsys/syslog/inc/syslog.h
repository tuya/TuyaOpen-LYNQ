#ifndef __SYSLOG_H__
#define __SYSLOG_H__


#include <stdio.h>
#include <stdint.h>


typedef enum
{
    SC_UART0 = (1 << 0),
    SC_UART1 = (1 << 1),
    SC_UART2 = (1 << 2),
    SC_UART3 = (1 << 3),
    SC_USB   = (1 << 4),
    SC_LCD   = (1 << 5),
    SC_EPAT  = (1 << 6),
    SC_FILE  = (1 << 7),
} SyslogChannelT;

typedef enum
{
    SL_DEBUG = 0,   /* debug-level messages */
    SL_INFO,        /* informational */
    SL_NOTICE,      /* normal but significant condition */
    SL_WARNING,     /* warning conditions */
    SL_ERR,         /* error conditions */
    SL_CRIT,        /* critical conditions */
    SL_ALERT,       /* action must be taken immediately */
    SL_EMERG,       /* system is unusable */
    SL_NO,          /* no log */
} SyslogLevelT;


extern SyslogLevelT gSyslogLevel;


#define SYSLOG_DEBUG(fmt, ...)    if (SL_DEBUG   >= gSyslogLevel) {syslogPrintf("[%s][%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define SYSLOG_INFO(fmt, ...)     if (SL_INFO    >= gSyslogLevel) {syslogPrintf("[%s][%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define SYSLOG_NOTICE(fmt, ...)   if (SL_NOTICE  >= gSyslogLevel) {syslogPrintf("[%s][%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define SYSLOG_WARNING(fmt, ...)  if (SL_WARNING >= gSyslogLevel) {syslogPrintf("[%s][%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define SYSLOG_ERR(fmt, ...)      if (SL_ERR     >= gSyslogLevel) {syslogPrintf("[%s][%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define SYSLOG_CRIT(fmt, ...)     if (SL_CRIT    >= gSyslogLevel) {syslogPrintf("[%s][%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define SYSLOG_ALERT(fmt, ...)    if (SL_ALERT   >= gSyslogLevel) {syslogPrintf("[%s][%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define SYSLOG_EMERG(fmt, ...)    if (SL_EMERG   >= gSyslogLevel) {syslogPrintf("[%s][%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}


int32_t  syslogPrintf(char *fmt, ...);
void     syslogSetLevel(uint32_t level);
uint32_t syslogGetLevel(void);
void     syslogSetChannel(uint32_t channel);
uint32_t syslogGetChannel(void);


#endif
