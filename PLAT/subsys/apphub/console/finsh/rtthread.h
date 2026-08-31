#ifndef __RT_THREAD_H__
#define __RT_THREAD_H__

#include <rtconfig.h>
#include <rtdef.h>
#include <rtservice.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifndef rt_malloc
#include <stdlib.h>
#define rt_malloc malloc
#endif
#ifndef rt_free
#define rt_free free
#endif


void rt_kputs(const char *str);
int rt_sprintf(char *buf, const char *format, ...);
int rt_snprintf(char *buf, rt_size_t size, const char *format, ...);
int rt_vsprintf(char *dest, const char *format, va_list arg_ptr);
extern int rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args);
extern int rt_kprintf(const char *fmt, ...);
extern int rt_kprintlnf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
