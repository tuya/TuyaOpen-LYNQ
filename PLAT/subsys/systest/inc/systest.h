/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __SYSTEST_H__
#define __SYSTEST_H__


#include "cmsis_os2.h"


#define SYSTEST_BOOT_START                                  "bootStart"
#define SYSTEST_HANDLE_CHECK(handle)                        \
do {                                                        \
    if (handle == NULL)                                     \
    {                                                       \
        printf("Failed to create %s.\r\n", #handle);        \
        return;                                             \
    }                                                       \
} while (0)


typedef struct
{
    int32_t param1;
    int32_t param2;
    char    *param3;
} SystestParamT;


void    systestInit(void);
void    systestCaseDelete(osMessageQueueId_t *queue, osThreadId_t *thread);
int32_t systestProcess(char *name, SystestParamT *param);
void    systestParamDelete(SystestParamT *param);


#endif
