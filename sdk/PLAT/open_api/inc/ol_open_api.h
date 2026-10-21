/****************************************************************************
 *
 ****************************************************************************/
#ifndef __OL_OPEN_API__
#define __OL_OPEN_API__

#include "ol_log.h"

typedef struct
{
    unsigned char     tsYear;
    unsigned char     tsMonth;
    unsigned char     tsDay;
    unsigned char     tsHour;
    unsigned char     tsMinute;
    unsigned char     tsSecond;
    unsigned char     tsTimezone;
    unsigned char     tsZoneSign;
} ol_timestamp;

typedef struct
{
    unsigned char type;
    char alphaid[32];
    char dialnum[24];
} ol_num_list;

typedef struct
{
    unsigned char totalnum;
    ol_num_list numlist[4];
} ol_cnum_info;

typedef struct
{
    unsigned long long sentbytes;
    unsigned long long recvbytes;
} ol_flow_info;

typedef enum
{
    SLEEP_DISABLE = 0, //pmu disable
    SLEEP_ENABLE       //pmu enable
}ol_sleep_state;

typedef enum
{
    SLEEP_ACTIVE = 0,   //active
    SLEEP_IDLE = 1,     //Idle
    SLEEP_SLEEP1 = 2,   //sleep1
    SLEEP_SLEEP2 = 3,   //sleep2
    SLEEP_HIBERNATE = 4,//hibernate
    SLEEP_ERROR
}ol_sleep_mode;

typedef enum
{
    OL_DUMP_FLASH_EPAT_LOOP,      /*0 -- dump full exception info to flash and EPAT tool then trapped in endless loop(while(1))*/
    OL_DUMP_PRINT_RESET,          /*print necessary exception info, and then reset*/
    OL_DUMP_FLASH_RESET,          /*dump full exception info to flash, and then reset*/
    OL_DUMP_FLASH_EPAT_RESET,     /*dump full exception info to flash and EPAT tool, and then reset*/
    OL_DUMP_SILENT_RESET,         /*reset directly*/
    OL_DUMP_MAX
}ol_dump_mode;



/*****************************************************************************
 *
 * FUNCTIO
 *      ol_get_cnum
 * DESCRIPTION
 *      This API is used to get cnum
 * PARAMETERS
 *      pointer to the struct ol_cnum_info
 * RETURN VALUES
 *      0 :success
 *      <0:fail
 * RETURN MESSAGE
 *      NONE
 *
 *****************************************************************************/
extern int ol_get_cnum(ol_cnum_info* cnum);


/*****************************************************************************
 *
 * FUNCTIO
 *      ol_flow_set_switch
 * DESCRIPTION
 *      This API is used to set whether to enable data counter
 * PARAMETERS
 *      mode:1:enable 0:disable
 * RETURN VALUES
 *      0 :success
 *      <0:fail
 * RETURN MESSAGE
 *      NONE
 *
 *****************************************************************************/
extern int ol_flow_set_switch(BOOL mode);


/*****************************************************************************
 *
 * FUNCTIO
 *      ol_flow_check_switch
 * DESCRIPTION
 *      This API is used to check the switch of data counter
 * PARAMETERS
 *      mode:the pointer to the switch mode
 * RETURN VALUES
 *      0 :success
 *      <0:fail
 * RETURN MESSAGE
 *      NONE
 *
 *****************************************************************************/
extern int ol_flow_check_switch(BOOL* mode);


/*****************************************************************************
 *
 * FUNCTIO
 *      ol_flow_get_count
 * DESCRIPTION
 *      This API is used to get the data counter
 * PARAMETERS
 *      flow:the pointer to the struct ol_flow_info
 * RETURN VALUES
 *      0 :success
 *      <0:fail
 * RETURN MESSAGE
 *      NONE
 *
 *****************************************************************************/
extern int ol_flow_get_count(ol_flow_info* flow);


/*****************************************************************************
 *
 * FUNCTIO
 *      ol_flow_clean_count
 * DESCRIPTION
 *      This API is used to clean the data counter
 * PARAMETERS
 *      NULL
 * RETURN VALUES
 *      0 :success
 *      <0:fail
 * RETURN MESSAGE
 *      NONE
 *
 *****************************************************************************/
extern int ol_flow_clean_count(void);



#endif /*__OL_OPEN_API__*/
