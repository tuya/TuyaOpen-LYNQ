/****************************************************************************
 *
 * Copy right:   2023 -, Copyrigths of EigenComm Ltd.
 * File name:    opensdk api.c
 * Description:  ec7xx opensdk api entry source file
 * History:      Rev1.0   2023-11-02
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_OPENSDK_ENABLE
#include <stdint.h>
#include "cmsis_os2.h"
#include "opensdk.h"
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
#include "openplayer.h"
#endif
#include "app.h"
#include "pwrkey.h"
#include "slpman.h"
#include "bsp.h"


OPENSDK_FLAG_EXPORT(API0);


extern ARM_DRIVER_USART *uart_driver_get(uint8_t symbol,uint32_t calltable, uint32_t callback);
extern ARM_DRIVER_SPI *spi_driver_get(uint8_t symbol, uint32_t calltable, uint32_t callback);
extern ARM_DRIVER_I2C *i2c_driver_get(uint8_t symbol, uint32_t calltable, uint32_t callback);
/* BSP (800 - 899) */
OPENSDK_FUNC_EXPORT(uart_driver_get,        803);
OPENSDK_FUNC_EXPORT(spi_driver_get,         802);
OPENSDK_FUNC_EXPORT(i2c_driver_get,         801);
OPENSDK_FUNC_EXPORT(delay_us,               800);

OPENSDK_FUNC_RESERVED(100,                      700);

/* Global Map (690 - 699) */
OPENSDK_FUNC_RESERVED(7,                        693);
OPENSDK_FUNC_EXPORT(globalItemSet,              692);
OPENSDK_FUNC_EXPORT(globalItemGet,              691);
OPENSDK_FUNC_EXPORT(globalmapInit,              690);

/* APP Hub (680 - 689) */
OPENSDK_FUNC_RESERVED(8,                        682);
OPENSDK_FUNC_EXPORT(setActiveApp,               681);
OPENSDK_FUNC_EXPORT(mountApp,                   680);

/* Open Player (650 - 679) */
#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
OPENSDK_FUNC_RESERVED(11,                       669);
OPENSDK_FUNC_EXPORT(openPlayIsPause,            668);
OPENSDK_FUNC_EXPORT(openPlayIsMute,             667);
OPENSDK_FUNC_EXPORT(openPlayIsLoop,             666);
OPENSDK_FUNC_EXPORT(openPlayerGetStatus,        665);
OPENSDK_FUNC_EXPORT(openPlayerFastRewind,       664);
OPENSDK_FUNC_EXPORT(openPlayerFastForward,      663);
OPENSDK_FUNC_EXPORT(openPlayLoop,               662);
OPENSDK_FUNC_EXPORT(openPlayGetTime,            661);
OPENSDK_FUNC_EXPORT(openPlayMute,               660);
OPENSDK_FUNC_EXPORT(openPlayGetVolume,          659);
OPENSDK_FUNC_EXPORT(openPlaySetVolume,          658);
OPENSDK_FUNC_EXPORT(openPlayResume,             657);
OPENSDK_FUNC_EXPORT(openPlaySeek,               656);
OPENSDK_FUNC_EXPORT(openPlayStop,               655);
OPENSDK_FUNC_EXPORT(openPlayPause,              654);
OPENSDK_FUNC_EXPORT(openPlay,                   653);
OPENSDK_FUNC_EXPORT(openPlaySetCallback,        652);
OPENSDK_FUNC_EXPORT(openPlayDestory,            651);
OPENSDK_FUNC_EXPORT(openPlayCreate,             650);
#else
OPENSDK_FUNC_RESERVED(30,                       650);
#endif

/* Storage (600 - 649) */
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
OPENSDK_FUNC_RESERVED(7,                        643);
OPENSDK_FUNC_EXPORT(getFsFreeSize,              642);
OPENSDK_FUNC_EXPORT(getFsTotalSize,             641);
OPENSDK_FUNC_EXPORT(statvfs,                    640);
OPENSDK_FUNC_RESERVED(7,                        633);
OPENSDK_FUNC_EXPORT(stat,                       632);
OPENSDK_FUNC_EXPORT(rename,                     631);
OPENSDK_FUNC_EXPORT(remove ,                    630);
OPENSDK_FUNC_RESERVED(6,                        624);
OPENSDK_FUNC_EXPORT(mkdir,                      623);
OPENSDK_FUNC_EXPORT(readdir,                    622);
OPENSDK_FUNC_EXPORT(closedir,                   621);
OPENSDK_FUNC_EXPORT(opendir,                    620);
OPENSDK_FUNC_RESERVED(9,                        611);
OPENSDK_FUNC_EXPORT(file_truncate,              610);
OPENSDK_FUNC_EXPORT(file_fstat,                 609);
OPENSDK_FUNC_EXPORT(file_rewind,                608);
OPENSDK_FUNC_EXPORT(file_ftell,                 607);
OPENSDK_FUNC_EXPORT(file_fseek,                 606);
OPENSDK_FUNC_EXPORT(file_fputs,                 605);
OPENSDK_FUNC_EXPORT(file_fgets,                 604);
OPENSDK_FUNC_EXPORT(file_fwrite,                603);
OPENSDK_FUNC_EXPORT(file_fread,                 602);
OPENSDK_FUNC_EXPORT(file_fclose,                601);
OPENSDK_FUNC_EXPORT(file_fopen,                 600);
#else
OPENSDK_FUNC_RESERVED(50,                       600);
#endif

/* VOLTE TLS Functions (500 - 599) */
OPENSDK_FUNC_RESERVED(100, 500);

/* PS Mobile/TCP/UDP/Http/Mqtt/WebSocket/FTP/NTP/SMS Functions (400 - 499) */
OPENSDK_FUNC_RESERVED(100, 400);

/* Chip Driver Functions (300 - 399) */
OPENSDK_FUNC_RESERVED(90,                       310);
OPENSDK_FUNC_EXPORT(slpManPlatVoteDisableSleep, 309);
OPENSDK_FUNC_EXPORT(slpManPlatVoteEnableSleep,  308);
OPENSDK_FUNC_EXPORT(slpManApplyPlatVoteHandle,  307);
OPENSDK_FUNC_EXPORT(GPIO_pinRead,               306);
OPENSDK_FUNC_EXPORT(GPIO_pinWrite,              305);
OPENSDK_FUNC_EXPORT(GPIO_pinConfig,             304);
OPENSDK_FUNC_EXPORT(PAD_setPinPullConfig,       303);
OPENSDK_FUNC_EXPORT(PAD_setPinConfig,           302);
OPENSDK_FUNC_EXPORT(PAD_getDefaultConfig,       301);
OPENSDK_FUNC_EXPORT(pwrKeyStartPowerOff,        300);

/* CMSIS_OS2 Functions (200 - 299) */
OPENSDK_FUNC_RESERVED(24,                      276);
OPENSDK_FUNC_EXPORT(osKernelInitialize,        275);
OPENSDK_FUNC_EXPORT(osKernelGetInfo,           274);
OPENSDK_FUNC_EXPORT(osKernelGetState,          273);
OPENSDK_FUNC_EXPORT(osKernelStart,             272);
OPENSDK_FUNC_EXPORT(osKernelLock,              271);
OPENSDK_FUNC_EXPORT(osKernelUnlock,            270);
OPENSDK_FUNC_EXPORT(osKernelRestoreLock,       269);
OPENSDK_FUNC_EXPORT(osKernelGetTickCount,      268);
OPENSDK_FUNC_EXPORT(osTimerGetExpiryTime,      267);
OPENSDK_FUNC_EXPORT(osKernelGetTickFreq,       266);
OPENSDK_FUNC_EXPORT(osKernelGetSysTimerCount,  265);
OPENSDK_FUNC_EXPORT(osKernelGetSysTimerFreq,   264);
OPENSDK_FUNC_EXPORT(osThreadNew,               263);
OPENSDK_FUNC_EXPORT(osThreadGetNumber,         262);
OPENSDK_FUNC_EXPORT(osThreadGetName,           261);
OPENSDK_FUNC_EXPORT(osThreadGetId,             260);
OPENSDK_FUNC_EXPORT(osThreadGetState,          259);
OPENSDK_FUNC_EXPORT(osThreadGetStackSpace,     258);
OPENSDK_FUNC_EXPORT(osThreadSetPriority,       257);
OPENSDK_FUNC_EXPORT(osThreadGetPriority,       256);
OPENSDK_FUNC_EXPORT(osThreadYield,             255);
OPENSDK_FUNC_EXPORT(osThreadSuspend,           254);
OPENSDK_FUNC_EXPORT(osThreadIsSuspendAll,      253);
OPENSDK_FUNC_EXPORT(osThreadResume,            252);
OPENSDK_FUNC_EXPORT(osThreadSuspendAll,        251);
OPENSDK_FUNC_EXPORT(osThreadResumeAll,         250);
OPENSDK_FUNC_EXPORT(osThreadExit,              249);
OPENSDK_FUNC_EXPORT(osThreadTerminate,         248);
OPENSDK_FUNC_EXPORT(osThreadGetCount,          247);
OPENSDK_FUNC_EXPORT(osThreadEnumerate,         246);
OPENSDK_FUNC_EXPORT(osThreadFlagsSet,          245);
OPENSDK_FUNC_EXPORT(osThreadFlagsClear,        244);
OPENSDK_FUNC_EXPORT(osThreadFlagsGet,          243);
OPENSDK_FUNC_EXPORT(osThreadFlagsWait,         242);
OPENSDK_FUNC_EXPORT(osDelay,                   241);
OPENSDK_FUNC_EXPORT(osDelayUntil,              240);
OPENSDK_FUNC_EXPORT(osTimerNew,                239);
OPENSDK_FUNC_EXPORT(osTimerGetName,            238);
OPENSDK_FUNC_EXPORT(osTimerStart,              237);
OPENSDK_FUNC_EXPORT(osTimerStop,               236);
OPENSDK_FUNC_EXPORT(osTimerIsRunning,          235);
OPENSDK_FUNC_EXPORT(osTimerDelete,             234);
OPENSDK_FUNC_EXPORT(osEventFlagsNew,           233);
OPENSDK_FUNC_EXPORT(osEventFlagsSet,           232);
OPENSDK_FUNC_EXPORT(osEventFlagsClear,         231);
OPENSDK_FUNC_EXPORT(osEventFlagsGet,           230);
OPENSDK_FUNC_EXPORT(osEventFlagsWait,          229);
OPENSDK_FUNC_EXPORT(osEventFlagsDelete,        228);
OPENSDK_FUNC_EXPORT(osMutexNew,                227);
OPENSDK_FUNC_EXPORT(osMutexAcquire,            226);
OPENSDK_FUNC_EXPORT(osMutexRelease,            225);
OPENSDK_FUNC_EXPORT(osMutexGetOwner,           224);
OPENSDK_FUNC_EXPORT(osMutexDelete,             223);
OPENSDK_FUNC_EXPORT(osSemaphoreNew,            222);
OPENSDK_FUNC_EXPORT(osSemaphoreAcquire,        221);
OPENSDK_FUNC_EXPORT(osSemaphoreRelease,        220);
OPENSDK_FUNC_EXPORT(osSemaphoreGetCount,       219);
OPENSDK_FUNC_EXPORT(osSemaphoreDelete,         218);
OPENSDK_FUNC_EXPORT(osMessageQueueNew,         217);
OPENSDK_FUNC_EXPORT(osMessageQueuePut,         216);
OPENSDK_FUNC_EXPORT(osMessageQueuePutToFront,  215);
OPENSDK_FUNC_EXPORT(osMessageQueueGet,         214);
OPENSDK_FUNC_EXPORT(osMessageQueueGetCapacity, 213);
OPENSDK_FUNC_EXPORT(osMessageQueueGetMsgSize,  212);
OPENSDK_FUNC_EXPORT(osMessageQueueGetCount,    211);
OPENSDK_FUNC_EXPORT(osMessageQueueGetSpace,    210);
OPENSDK_FUNC_EXPORT(osMessageQueueReset,       209);
OPENSDK_FUNC_EXPORT(osMessageQueueDelete,      208);
OPENSDK_FUNC_EXPORT(ostaskENTER_CRITICAL,      207);
OPENSDK_FUNC_EXPORT(ostaskEXIT_CRITICAL,       206);
OPENSDK_FUNC_EXPORT(ostaskENTER_CRITICAL_ISR,  205);
OPENSDK_FUNC_EXPORT(ostaskEXIT_CRITICAL_ISR,   204);
OPENSDK_FUNC_EXPORT(osIsInISRContext,          203);
OPENSDK_FUNC_EXPORT(osTaskSetTimeOutState,     202);
OPENSDK_FUNC_EXPORT(osTaskCheckForTimeOut,     201);
OPENSDK_FUNC_EXPORT(osTaskGetFreeHeapSize,     200);

/* Standard Library Functions (1 - 199) */
OPENSDK_FUNC_RESERVED(63,       137);

#include <math.h>     //21[116:136]
OPENSDK_FUNC_RESERVED(21,       116);

#include <time.h>     //9[107:115]
OPENSDK_FUNC_RESERVED(9,       107);
// time_t time(time_t *timer)
// size_t strftime(char *str, size_t maxsize, const char *format, const struct tm *timeptr)
// time_t mktime(struct tm *timeptr)
// struct tm *localtime(const time_t *timer)
// struct tm *gmtime(const time_t *timer)
// double difftime(time_t time1, time_t time2)
// char *ctime(const time_t *timer)
// clock_t clock(void)
// char *asctime(const struct tm *timeptr)
// OPENSDK_FUNC_EXPORT(asctime,    115);
// OPENSDK_FUNC_EXPORT(clock,      114);
// OPENSDK_FUNC_EXPORT(ctime,      113);
// OPENSDK_FUNC_EXPORT(time,       112);
// OPENSDK_FUNC_EXPORT(difftime,   111);
// OPENSDK_FUNC_EXPORT(localtime,  110);
// OPENSDK_FUNC_EXPORT(strftime,   109);
// OPENSDK_FUNC_EXPORT(gmtime,     108);
// OPENSDK_FUNC_EXPORT(mktime,     107);

#include <stdarg.h>     //3[104:106]
OPENSDK_FUNC_RESERVED(3,       104);
// OPENSDK_FUNC_EXPORT(va_end,     106);
// OPENSDK_FUNC_EXPORT(va_arg,     105);
// OPENSDK_FUNC_EXPORT(va_start,   104);

#include <ctype.h> //11[93:103]
OPENSDK_FUNC_EXPORT(isxdigit,   103);
OPENSDK_FUNC_EXPORT(isupper,    102);
OPENSDK_FUNC_EXPORT(isspace,    101);
OPENSDK_FUNC_EXPORT(ispunct,    100);
OPENSDK_FUNC_EXPORT(isprint,    99);
OPENSDK_FUNC_EXPORT(islower,    98);
OPENSDK_FUNC_EXPORT(isgraph,    97);
OPENSDK_FUNC_EXPORT(isdigit,    96);
OPENSDK_FUNC_EXPORT(iscntrl,    95);
OPENSDK_FUNC_EXPORT(isalpha,    94);
OPENSDK_FUNC_EXPORT(isalnum,    93);

#include <string.h> //22[71:92]
// void *memchr(const void *str, int c, size_t n)
// int memcmp(const void *str1, const void *str2, size_t n)
// void *memcpy(void *dest, const void *src, size_t n)
// void *memmove(void *dest, const void *src, size_t n)
// void *memset(void *str, int c, size_t n)
// char *strcat(char *dest, const char *src)
// char *strncat(char *dest, const char *src, size_t n)
// char *strchr(const char *str, int c)
// int strcmp(const char *str1, const char *str2)
// int strncmp(const char *str1, const char *str2, size_t n)
// int strcoll(const char *str1, const char *str2)
// char *strcpy(char *dest, const char *src)
// char *strncpy(char *dest, const char *src, size_t n)
// size_t strcspn(const char *str1, const char *str2)
// char *strerror(int errnum)
// size_t strlen(const char *str)
// char *strpbrk(const char *str1, const char *str2)
// char *strrchr(const char *str, int c)
// size_t strspn(const char *str1, const char *str2)
// char *strstr(const char *haystack, const char *needle)
// char *strtok(char *str, const char *delim)
// size_t strxfrm(char *dest, const char *src, size_t n)
OPENSDK_FUNC_EXPORT(strxfrm,    92);
OPENSDK_FUNC_EXPORT(strtok,     91);
OPENSDK_FUNC_EXPORT(strstr,     90);
OPENSDK_FUNC_EXPORT(strspn,     89);
OPENSDK_FUNC_EXPORT(strrchr,    88);
OPENSDK_FUNC_EXPORT(strpbrk,    87);
OPENSDK_FUNC_EXPORT(strlen,     86);
OPENSDK_FUNC_EXPORT(strerror,   85);
OPENSDK_FUNC_EXPORT(strcspn,    84);
OPENSDK_FUNC_EXPORT(strncpy,    83);
OPENSDK_FUNC_EXPORT(strcpy,     82);
OPENSDK_FUNC_EXPORT(strcoll,    81);
OPENSDK_FUNC_EXPORT(strncmp,    80);
OPENSDK_FUNC_EXPORT(strcmp,     79);
OPENSDK_FUNC_EXPORT(strchr,     78);
OPENSDK_FUNC_EXPORT(strncat,    77);
OPENSDK_FUNC_EXPORT(strcat,     76);
OPENSDK_FUNC_EXPORT(memset,     75);
OPENSDK_FUNC_EXPORT(memmove,    74);
OPENSDK_FUNC_EXPORT(memcpy,     73);
OPENSDK_FUNC_EXPORT(memcmp,     72);
OPENSDK_FUNC_EXPORT(memchr,     71);

#include <stdlib.h>     //28[43:70]
OPENSDK_FUNC_RESERVED(15,       56);
// double atof(const char *str)
// int atoi(const char *str)
// long int atol(const char *str)
// double strtod(const char *str, char **endptr)
// long int strtol(const char *str, char **endptr, int base)
// unsigned long int strtoul(const char *str, char **endptr, int base)
// void *calloc(size_t nitems, size_t size)
// void free(void *ptr)
// void *malloc(size_t size)
// void *realloc(void *ptr, size_t size)
// void abort(void)
// int atexit(void (*func)(void))
// void exit(int status)
// char *getenv(const char *name)
// int system(const char *string)
// void *bsearch(const void *key, const void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *))
// void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void*))
// int abs(int x)
// div_t div(int numer, int denom)
// long int labs(long int x)
// ldiv_t ldiv(long int numer, long int denom)
// int rand(void)
// void srand(unsigned int seed)
// int mblen(const char *str, size_t n)
// size_t mbstowcs(schar_t *pwcs, const char *str, size_t n)
// int mbtowc(whcar_t *pwc, const char *str, size_t n)
// size_t wcstombs(char *str, const wchar_t *pwcs, size_t n)
// int wctomb(char *str, wchar_t wchar)
OPENSDK_FUNC_EXPORT(labs,       55);
OPENSDK_FUNC_EXPORT(atol,       54);
OPENSDK_FUNC_EXPORT(atoi,       53);
OPENSDK_FUNC_EXPORT(atof,       52);
OPENSDK_FUNC_EXPORT(strtoul,    51); 
OPENSDK_FUNC_EXPORT(strtod,     50);   
OPENSDK_FUNC_EXPORT(strtol,     49);
OPENSDK_FUNC_EXPORT(srand,      48);
OPENSDK_FUNC_EXPORT(rand,       47);
OPENSDK_FUNC_EXPORT(realloc,    46);
OPENSDK_FUNC_EXPORT(free,       45);
OPENSDK_FUNC_EXPORT(calloc,     44);
OPENSDK_FUNC_EXPORT(malloc,     43);

#include <stdio.h>  //42[1:42]
OPENSDK_FUNC_RESERVED(35,       8);
OPENSDK_FUNC_EXPORT(snprintf,   7);
OPENSDK_FUNC_EXPORT(vsprintf,   6);
OPENSDK_FUNC_EXPORT(vfprintf,   5);
OPENSDK_FUNC_EXPORT(vprintf,    4);
OPENSDK_FUNC_EXPORT(fprintf,    3);
OPENSDK_FUNC_EXPORT(sprintf,    2);
OPENSDK_FUNC_EXPORT(printf,     1);
OPENSDK_APP_EXPORT();


#endif
