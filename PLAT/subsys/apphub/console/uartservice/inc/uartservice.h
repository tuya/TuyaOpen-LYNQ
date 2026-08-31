/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __UARTSERVICE_H__
#define __UARTSERVICE_H__


#include <stdint.h>


void  uartServiceInit(void);
void  uartServiceSend(char *response, uint32_t timeout);
void *uartserviceHandleGet(void);


#endif
