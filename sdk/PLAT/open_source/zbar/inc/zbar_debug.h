#ifndef __ZBAR_DEBUG_H__
#define __ZBAR_DEBUG_H__

#include "ol_log.h"
 
#define zbar_log                        OL_LOG_PRINTF

//#define zprintf(...)

//#define dprintf(...)

//#define zassert(...)

#define zprintf(level, fmt, ...)        do { \
        if(level == 0) {\
            OL_LOG_PRINTF(fmt, ##__VA_ARGS__); \
        }\
    } while(0)

#define dprintf(level, fmt, ...)        do { \
        if(level == 0) {\
            OL_LOG_PRINTF(fmt, ##__VA_ARGS__); \
        }\
    } while(0)

#define zassert(condition, retval, format, ...) do { \
        EC_ASSERT(condition,retval,0,0); \
    } while(0)

#endif //__ZBAR_DEBUG_H__