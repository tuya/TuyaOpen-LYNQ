/******************************************************************************
 * (C) Copyright 2018 EIGENCOMM International Ltd.
 * All Rights Reserved
*******************************************************************************
 *  Filename: sntp.c
 *
 *  Description: sntp
 *
 *  History:creat by xwang
 *
 *  Notes:
 *
******************************************************************************/

#include "commontypedef.h"


#define SNTP_RCV_TIMEOUT 15  //seconds
#define SNTP_RETRY_TIMES 3
#define SNTP_RESEND_REQUEST_DELAY 5000 //ms

#define SNTP_DEFAULT_PORT  123




BOOL SntpInit(CHAR* serverAddr, UINT16 serverPort, UINT16 reqHandler, BOOL autoSync, UINT8 cid, UINT8 retryTimes, UINT16 timeout);

void SntpTerminat(void);

//check sntp whether running,TRUE->running, FALSE->not run
BOOL SntpChkStatus(CHAR *serverAddr, UINT16 *serverPort, UINT8 *cid);



