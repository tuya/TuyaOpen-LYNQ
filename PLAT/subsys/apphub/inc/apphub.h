/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    apphub.h
 * Description:  EC718 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_APPHUB_H
#define  SUBSYS_APPHUB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "subsys.h"

#define APPHUB_MAX_SLOT	16

typedef enum
{
    SLOT_EMPTY    	= 0,	//no mount
    SLOT_SILENT    	= 1,	//no run proc
	SLOT_BACKGOURD  = 2,	//backgournd run proc
	SLOT_ACTIVED    = 3,	//with ui run proc
} SlotStatusTypeT;

typedef struct
{
	int32_t curActiveApp;
	int32_t freeSlotNumber;
	int32_t slotStat;
} AppHubInfoT;

void subApphubInit();
int32_t getApphubSlotStat();

#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_APPHUB_H */