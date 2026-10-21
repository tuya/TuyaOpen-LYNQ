#ifndef __OL_LOG_H__
#define __OL_LOG_H__

#include <stdbool.h>
#include <stdint.h>
#include "unilog.h"
#include "osasys.h"
#include "cmsis_compiler.h"
#include "debug_trace.h"
#include DEBUG_LOG_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif


//not recommended to print data too long, Over 200 bytes, log loss may occur
#define MLOG_BUFFER_LEN                  256


#define OL_UNILOG_CAT2(a, b)             a##b
#define OL_UNILOG_PASTE2(a, b)           OL_UNILOG_CAT2(a, b)
#define OL_UNILOG_PASTE3(a, b, c)        OL_UNILOG_PASTE2(OL_UNILOG_PASTE2(a, b), c)
#define OL_UNILOG_PASTE4(a, b, c, d)     OL_UNILOG_PASTE2(OL_UNILOG_PASTE3(a, b, c), d)
#define OL_UNILOG_UNIQUE_ID(ownerId, moduleId) \
        OL_UNILOG_PASTE4(ownerId##__##moduleId##__, __CURRENT_FILE_NAME__, _, __LINE__)

#define MLOG(moduleId, debugLevel, format, ...) \
do  \
{   \
    swLogPrintf(OL_UNILOG_UNIQUE_ID(UNILOG_CUSTOMER, moduleId), debugLevel, ##__VA_ARGS__); \
    {(void)format;} \
}while(0)

extern void mbtk_log_printf(char *buffer, int buffer_len, const char *func,const char *fmt,...);

//-------------------------------------------------------------------------------------
//------------------------------- MBTK Internal API log -------------------------------
//-------------------------------------------------------------------------------------
#define MBTK_LOG_INFO(fmt,...)    \
        {\
            char log_buffer[MLOG_BUFFER_LEN] = {0}; \
            mbtk_log_printf(log_buffer, MLOG_BUFFER_LEN, __func__, fmt, ##__VA_ARGS__); \
            MLOG(UNILOG_API,P_DEBUG, "%s", log_buffer); \
        }

#define MBTK_LOG_PRINTF(fmt,...)    \
        {\
            char log_buffer[MLOG_BUFFER_LEN] = {0}; \
            mbtk_log_printf(log_buffer, MLOG_BUFFER_LEN, __func__, fmt, ##__VA_ARGS__); \
            MLOG(UNILOG_API,P_VALUE, "%s", log_buffer); \
        }

#define MBTK_LOG_ERROR(fmt,...)    \
        {\
            char log_buffer[MLOG_BUFFER_LEN] = {0}; \
            mbtk_log_printf(log_buffer, MLOG_BUFFER_LEN, __func__, fmt, ##__VA_ARGS__); \
            MLOG(UNILOG_API,P_ERROR, "%s", log_buffer); \
        }


//-------------------------------------------------------------------------------------
//----------------------------------- Open APP log ------------------------------------
//-------------------------------------------------------------------------------------
#define OL_LOG_INFO(fmt,...)    \
        {\
            char log_buffer[MLOG_BUFFER_LEN] = {0}; \
            mbtk_log_printf(log_buffer, MLOG_BUFFER_LEN, __func__, fmt, ##__VA_ARGS__); \
            MLOG(UNILOG_APP,P_DEBUG, "%s", log_buffer); \
        }

#define OL_LOG_PRINTF(fmt,...)    \
        {\
            char log_buffer[MLOG_BUFFER_LEN] = {0}; \
            mbtk_log_printf(log_buffer, MLOG_BUFFER_LEN, __func__, fmt, ##__VA_ARGS__); \
            MLOG(UNILOG_APP,P_VALUE, "%s", log_buffer); \
        }

#define OL_LOG_ERROR(fmt,...)    \
        {\
            char log_buffer[MLOG_BUFFER_LEN] = {0}; \
            mbtk_log_printf(log_buffer, MLOG_BUFFER_LEN, __func__, fmt, ##__VA_ARGS__); \
            MLOG(UNILOG_APP,P_ERROR, "%s", log_buffer); \
        }


#ifdef __cplusplus
}
#endif
#endif/*__OL_LOG_H__*/
