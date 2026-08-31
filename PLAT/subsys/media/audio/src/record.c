/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    record.c
 * Description:  EC618 lwm2m demo entry source file
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <string.h>

#include "cmsis_os2.h"
#include "slpman.h"
#include "charge.h"
#include "osasys.h"

#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#include "iniparse.h"
#endif
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "record.h"
#endif

#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "servicemanager.h"
#include "audAmrRecord.h"
#include "media.h"
#include "ec_ring.h"

#define THREAD_STACK_SIZE_RECORD     		(20* 1024)
#define QUEUE_SIZE_RECORD            		100

#ifdef THREAD_STATIC
static StaticTask_t       gRecordThreadCbMem                                      = {0};
PLAT_FPSRAM_ZI_CUST static uint8_t gRecordThreadStackMem[THREAD_STACK_SIZE_RECORD] = {0};
#endif

osMessageQueueId_t gRecordQueue = NULL;

static int32_t recordProcess(QueueRecordT *queue)
{
	int32_t recResp = AV_RET_RECORD_ERROR;
    switch (queue->recType)
    {
#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
		case AUDIO_RECORD_CODEC_AMR:
	    case AUDIO_RECORD_CODEC_PCM:
		case AUDIO_RECORD_CODEC_PCM_3A:
		{
#ifdef MBTK_OPENCPU_SUPPORT
			int32_t ol_amrRecord(RecordCallbackT callback,RecordParamT *recordParam);

			if(NULL != queue->buffer)
			{
			    recResp = amrRecord(queue->buffer,&queue->recordParam);
			}
			else
			{
                recResp = ol_amrRecord(queue->callback,&queue->recordParam);
			}
#else
			recResp = amrRecord(queue->buffer,&queue->recordParam);
#endif
		}
		break;
#endif
        default:
            break;
    }
	return recResp;
}
osThreadId_t gRecordThread = NULL;
static void threadRecord(void *argument)
{
    QueueRecordT queue = {0};
    static uint8_t     vote  = 0xFF;

	int32_t recResp = AV_RET_RECORD_ERROR;
    gRecordQueue = osMessageQueueNew(QUEUE_SIZE_RECORD, sizeof(queue), NULL);
    if (gRecordQueue == NULL)
    {
        goto labelEnd;
    }
	
    slpManApplyPlatVoteHandle("record", &vote);
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadRecord, P_INFO, "threadRecord start ");
	gRecordThread = osThreadGetId();
    while (1)
    {
        memset(&queue, 0, sizeof(queue));
        if (osMessageQueueGet(gRecordQueue, &queue, 0, osWaitForever) == osOK)
        {
        	slpManPlatVoteDisableSleep(vote, SLP_SLP1_STATE);
        	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadRecord_1, P_INFO, "start process %e<AudioTypeT>", queue.recType);
            
            recResp = recordProcess(&queue);
 			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, threadRecord_2, P_INFO, "record recResp %d", recResp);
            
            if (queue.callback != NULL)
            {
                #ifndef MBTK_OPENCPU_SUPPORT
                queue.callback(recResp);
                #endif
            }
			if(queue.buffer != NULL)
			{
				free(queue.buffer);
				queue.buffer = NULL;
			}
			slpManPlatVoteEnableSleep(vote, SLP_SLP1_STATE);
        }
    }

labelEnd:
#if 0 // Service Manager
    osThreadExit();
#else
    Service_stop("service:/threadRecord");
#endif
}

int32_t EC_audioRecord(char *path, RecordParamT recordParam, RecordCallbackT callback)
{
    QueueRecordT queue = {0};
	uint32_t    length = 0;
	osStatus_t status = osError;
	
	if (gRecordQueue != NULL)
    {
    	if(path != NULL)
		{			
			length         = strlen(path);
			queue.buffer   = malloc(length + 1);

			if(queue.buffer != NULL)
			{
				memset(queue.buffer, 0,    length + 1);
           		memcpy(queue.buffer, path, length);
			}		 
		}

		queue.recType = recordParam.codec;
		queue.callback = callback;
		memcpy(&queue.recordParam, &recordParam, sizeof(RecordParamT));
		status = osMessageQueuePut(gRecordQueue, &queue, 0, 3);	
	}
	return status;
}

#ifdef MBTK_OPENCPU_SUPPORT
int32_t ol_audioRecord(RecordParamT recordParam, ol_RecordCallbackT callback)
{
    QueueRecordT queue = {0};
    osStatus_t status = osOK;

    if (gRecordQueue != NULL)
    {
        queue.recType = recordParam.codec;
        queue.buffer   = NULL;
        queue.callback = (RecordCallbackT)callback;
        memcpy(&queue.recordParam, &recordParam, sizeof(RecordParamT));
        status = osMessageQueuePut(gRecordQueue, &queue, 0, 0);
    }
    return status;
}

void ol_audioRecordStop(void)
{
    amrRecordStop(0);
}
#endif

int32_t recordInit(void)
{
    osThreadAttr_t threadAttr = {0};

    memset(&threadAttr, 0, sizeof(threadAttr));
    threadAttr.name       = "threadRecord";
    threadAttr.stack_size = THREAD_STACK_SIZE_RECORD;
    threadAttr.priority   = osPriorityBelowNormal7;
#ifdef THREAD_STATIC
	threadAttr.stack_mem  = gRecordThreadStackMem;
	threadAttr.cb_mem     = &gRecordThreadCbMem;
	threadAttr.cb_size    = sizeof(StaticTask_t);
#endif
#if 0
    if (osThreadNew(threadRecord, NULL, &threadAttr) == NULL)
#else
    char serviceName[32] = {0};
    snprintf(serviceName, sizeof(serviceName), "service:/%s", threadAttr.name);
    Service_reg(serviceName, threadRecord, NULL, threadAttr.cb_mem, threadAttr.cb_size, threadAttr.stack_mem, threadAttr.stack_size, threadAttr.priority);
    if (Service_start(serviceName) == 0)
#endif
    {
        //SYSLOG_EMERG("Failed to create thread for record.\r\n");
    }

    return 0;
}


