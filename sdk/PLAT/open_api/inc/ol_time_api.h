#ifndef __OL_TIME_API__
#define __OL_TIME_API__

#include "commontypedef.h"


typedef struct
{
    unsigned int year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    char timezone;
}ol_ntp_time_t;


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to set the information about the server for time
 *          synchronization and start time synchronization
 * PARAMETERS
 *      serverAddr:     server address
 *      serverPort:     server port
 * RETURN VALUES
 *      0    : successfully
 *      -1   : error
 *
 *****************************************************************************/
extern int ol_ntp(char* serverAddr, unsigned short serverPort);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to get system UTC time
 * PARAMETERS
 *      time:  point to time stauct define as ol_ntp_time_t
 * RETURN VALUES
 *      0    : successfully
 *      -1   : error
 *
 *****************************************************************************/
extern int ol_get_time(ol_ntp_time_t* time);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to set system UTC time
 * PARAMETERS
 *      time:  point to time stauct define as ol_ntp_time_t
 *             year : 2000 ~ 2099 year
 *             hour : is UTC value ex. beijing(12:00:00)/UTC(4:00:00+32)
 * RETURN VALUES
 *      0    : successfully
 *      -1   : error
 *
 *****************************************************************************/
extern int ol_set_time(ol_ntp_time_t* time);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to get the number of seconds since from 1970.01.01 00:00:00
 * PARAMETERS
 *      seconds:    point to the struct time_t and is used to store the value of seconds.
 * RETURN VALUES
 *      return the current time seconds from 1970.01.01 00:00:00
 *
 *****************************************************************************/
extern time_t ol_time(time_t *seconds);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to parse the value returned by the ol_time() function
 *          into the current date(UTC time)
 * PARAMETERS
 *      timep:              point to the time_t value
 * RETURN VALUES
 *      return the point of the struct tm
 *
 *****************************************************************************/
extern struct tm* ol_gmtime(const time_t*timep);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to parse the value returned by the ol_time() function
 *          into the current date(local time)
 * PARAMETERS
 *      timep:              point to the time_t value
 * RETURN VALUES
 *      return the point of the struct tm
 *
 *****************************************************************************/
extern struct tm* ol_localtime(const time_t *timep);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to convert the struct tm(UTC time) to the number of seconds
 *          1970.01.01 00:00:00
 * PARAMETERS
 *      timeptr:    point to the tm value, converted time with time zone(ol_localtime)
 * RETURN VALUES
 *      return the seconds from 1970.01.01 00:00:00
 *
 *****************************************************************************/
extern time_t ol_mktime(struct tm* timeptr);


#endif //__OL_TIME_API__
