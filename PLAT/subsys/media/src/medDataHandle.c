/****************************************************************************
*
* Copy right:   2024-, Copyrigths of EigenComm Ltd.
* File name:    openplayer.c
* Description:  EC718 media play source file
* History:      Rev1.0   2024-03-13
*
****************************************************************************/

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "exception_process.h"
#include DEBUG_LOG_HEADER_FILE
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
#include "volumeManager.h"
#endif

#include "medDataHandle.h"
#include "ccio_audio.h"

/*----------------------------------------------------------------------------*
 *					  MACROS												  *
 *----------------------------------------------------------------------------*/
#define MED_DATA_TRUNCK_SIZE_MAX			(1920)
#define MED_DATA_TRUNCK_BLK_CNT				(3)

#define MED_DATA_PRE_FILL_DMA_BUFF_CNT		(3840)
#define MED_DATA_HANDLE_CYCLE_TIME			(40) 	/*ms*/

#define MED_DATA_HANDLE_END		             (0x1)
#define MED_DATA_HANDLE_TRIGGER_FLAG          (SLAVE_TRANSFER_TX_FLAG)

/*----------------------------------------------------------------------------*
 *					 DATA TYPE DEFINITION									  *
 *----------------------------------------------------------------------------*/
typedef struct _medDataHdlState{
	uint8_t dataBitWidth;
	UINT16 medDataTrunckSize; 
	uint8_t medDataHandleFlag;
	osTimerId_t 	dataHdlTimer;
	rawDataHanderParam_T param;
}medDataHdlState_T;

/*----------------------------------------------------------------------------*
 * 					 GLOBAL VARIABLES									  	  *
 *----------------------------------------------------------------------------*/
ecRingT *   rawDataCache = NULL;
medDataHandleCtrlInfo   gDataHdlCtlInfo = {0};
medDataHdlState_T   gmedDataHdlStat = {0,0,MED_DATA_HDL_STA_IDLE,NULL,{{0,0}}};
static medDataCpltCallBack gDataCpltCallBack = NULL;
static void* gpDataCpltParam = NULL;
#if (PSRAM_EXIST == 1)
__attribute__((aligned(8))) PLAT_FPSRAM_ZI_CUST uint8_t dataTrunckBuf[MED_DATA_TRUNCK_SIZE_MAX*MED_DATA_TRUNCK_BLK_CNT*2] = {0};/*first byte is SamplteRate*/
#else
__attribute__((aligned(8))) uint8_t dataTrunckBuf[MED_DATA_TRUNCK_SIZE_MAX*MED_DATA_TRUNCK_BLK_CNT*2] = {0};/*first byte is SamplteRate*/
#endif
extern uint32_t sampleRateConvert(uint32_t rate, bool toEnum);
extern uint8_t getCurrDescAddr();
extern uint8_t isDmaRun();
/*----------------------------------------------------------------------------*
 * 					 PRIVATE FUNCTION DECLEARATION						      *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 * 				     PRIVATE FUNCTION					   	   	  			  *
 *----------------------------------------------------------------------------*/
static void medDataHdlTimerFunc(void *param)
{	
	int32_t readSize = 0;
	int32_t dataSize = 0;
	uint8_t currDmaAddr = 0;

	int32_t cacheFillCnt = xEcRingGetOption(rawDataCache,E_LRO_BUFF_FILL_SIZE);	
	if((gmedDataHdlStat.param.field.envType == MED_DATA_ENV_TYPE_LOCAL) && (gDataHdlCtlInfo.dmaBufFillCnt >= cacheFillCnt))
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medDataHdlTimerFunc_2, P_DEBUG, "stop play raw data,dmaBufFillCnt[%d],cacheFillCnt[%d]",gDataHdlCtlInfo.dmaBufFillCnt,cacheFillCnt);
		gmedDataHdlStat.medDataHandleFlag = MED_DATA_HDL_STA_SUSPEND;
		memset(dataTrunckBuf,0,MED_DATA_TRUNCK_SIZE_MAX*MED_DATA_TRUNCK_BLK_CNT*2);
		osTimerStop(gmedDataHdlStat.dataHdlTimer);
		return;
	}
	dataSize = xEcRingGetOption(rawDataCache,E_LRO_DATA_SIZE);
	if(dataSize < gmedDataHdlStat.medDataTrunckSize)
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medDataHdlTimerFunc_3, P_WARNING, "dataSize[%d] < trunkSize[%d]",dataSize,gmedDataHdlStat.medDataTrunckSize);
	if(dataSize)
	{
		if(gDataHdlCtlInfo.hdlFlag && isDmaRun())
		{
			currDmaAddr = getCurrDescAddr();
			if(currDmaAddr== gDataHdlCtlInfo.trunckIndex)
			{
				ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medDataHdlTimerFunc_4, P_WARNING, "currDmaAddr[%d] trunckIndex[%d]",currDmaAddr,gDataHdlCtlInfo.trunckIndex);
				gDataHdlCtlInfo.trunckIndex = ((gDataHdlCtlInfo.trunckIndex + 1) % 3);
			}
		}
		readSize = xEcRingRead(rawDataCache, dataTrunckBuf + gDataHdlCtlInfo.trunckIndex * gmedDataHdlStat.medDataTrunckSize, gmedDataHdlStat.medDataTrunckSize);
        if((gDataHdlCtlInfo.hdlFlag == 0) && (gDataHdlCtlInfo.trunckIndex == 1))
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medDataHdlTimerFunc_1, P_DEBUG, "start play raw data ");
			if(gmedDataHdlStat.param.field.envType == MED_DATA_ENV_TYPE_TONE)
			{
				audioStartPlaySound(PLAY_MULTI_MEDIA, dataTrunckBuf, MED_DATA_TRUNCK_BLK_CNT*gmedDataHdlStat.medDataTrunckSize, gmedDataHdlStat.param.field.samplerate, gmedDataHdlStat.dataBitWidth);
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
				volumeManagerActivate("ring");
#endif
			}
			else 
			{
				audioAppStartPlaySound(PLAY_MULTI_MEDIA, dataTrunckBuf, MED_DATA_TRUNCK_BLK_CNT*gmedDataHdlStat.medDataTrunckSize, gmedDataHdlStat.param.field.samplerate, gmedDataHdlStat.dataBitWidth);
#ifdef FEATURE_SUBSYS_VOLUME_ENABLE
				volumeManagerActivate("audio");
#endif
			}
			gDataHdlCtlInfo.hdlFlag = 1;
			
		}

		if(gDataHdlCtlInfo.hdlFlag && isDmaRun())
		{
			currDmaAddr = getCurrDescAddr();
			if(((currDmaAddr + 1 )%3)== gDataHdlCtlInfo.trunckIndex)
			{
				gDataHdlCtlInfo.trunckIndex = ((currDmaAddr + 2) % 3);
			}
			else
				gDataHdlCtlInfo.trunckIndex = ((gDataHdlCtlInfo.trunckIndex + 1) % 3);
			
		}
        else
		{
			gDataHdlCtlInfo.trunckIndex = ((gDataHdlCtlInfo.trunckIndex + 1) % 3);
		}
        if(gDataCpltCallBack)
		{
			gDataCpltCallBack(gpDataCpltParam, dataTrunckBuf + gDataHdlCtlInfo.trunckIndex * gmedDataHdlStat.medDataTrunckSize, gmedDataHdlStat.medDataTrunckSize * sizeof(uint16_t));
		}
		if(gmedDataHdlStat.param.field.envType != MED_DATA_ENV_TYPE_TONE)
			gDataHdlCtlInfo.dmaBufFillCnt = gDataHdlCtlInfo.dmaBufFillCnt + readSize;
	}
	
}

void medDataHandleInit(void)
{
	if(gmedDataHdlStat.dataHdlTimer == NULL)
		gmedDataHdlStat.dataHdlTimer = osTimerNew((osTimerFunc_t)medDataHdlTimerFunc, osTimerPeriodic, NULL, NULL);	
	memset(&gDataHdlCtlInfo,0x00,sizeof(medDataHandleCtrlInfo));
	memset(dataTrunckBuf,0,MED_DATA_TRUNCK_SIZE_MAX*MED_DATA_TRUNCK_BLK_CNT*2);
	memset(&gmedDataHdlStat.param,0x00,sizeof(rawDataHanderParam_T));
	gmedDataHdlStat.dataBitWidth = 0;
	gmedDataHdlStat.medDataTrunckSize = 0; 
	gmedDataHdlStat.medDataHandleFlag = MED_DATA_HDL_STA_IDLE;
}

/*----------------------------------------------------------------------------*
 * 				     GLOBAL FUNCTION					   	   	  			  *
 *----------------------------------------------------------------------------*/
int medSetDataCpltCallBack(medDataCpltCallBack callback, void* param)
{
	gDataCpltCallBack = callback;
	gpDataCpltParam = param;
	return 0;
}

uint8_t medDataHandleStart(rawDataHanderParam_T *handleParam)
{
	uint32_t samplerate = 0;
	
	medDataHandleInit();
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medDataHandleStart, P_INFO,"envtype: %e<medDataEnvType_E> ",handleParam->field.envType);
	gmedDataHdlStat.dataBitWidth = (handleParam->field.bitWidth == 0 ? 16 : 8);
	samplerate =  sampleRateConvert(handleParam->field.samplerate, false);	
	gmedDataHdlStat.medDataTrunckSize = samplerate * MED_DATA_HANDLE_CYCLE_TIME * gmedDataHdlStat.dataBitWidth / 8 / 1000;/* samplerate * 40ms * 16bit */

	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medDataHandleStart_1, P_INFO,
		"sound config: samplerate[%d],bitWidth[%d],TrunckSize[%d] ",
		samplerate, gmedDataHdlStat.dataBitWidth,gmedDataHdlStat.medDataTrunckSize);
	memcpy(&gmedDataHdlStat.param,handleParam,sizeof(rawDataHanderParam_T));
	
	osTimerStart(gmedDataHdlStat.dataHdlTimer,MED_DATA_HANDLE_CYCLE_TIME);	
	gmedDataHdlStat.medDataHandleFlag = MED_DATA_HDL_STA_START;
	return 0;
}

void medDataHandleStop(void)
{	
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medDataHandleStop, P_DEBUG, "hdlFlag:%e<medDataHdlStaT>,envType:%e<medDataEnvType_E>",gmedDataHdlStat.medDataHandleFlag,gmedDataHdlStat.param.field.envType);
	if(gmedDataHdlStat.medDataHandleFlag == MED_DATA_HDL_STA_START || 
		gmedDataHdlStat.medDataHandleFlag == MED_DATA_HDL_STA_SUSPEND)
	{
		if(gmedDataHdlStat.param.field.envType == MED_DATA_ENV_TYPE_LOCAL)
		{
			audioAppStopPlaySound(PLAY_MULTI_MEDIA);
		}
		memset(dataTrunckBuf,0,MED_DATA_TRUNCK_SIZE_MAX*MED_DATA_TRUNCK_BLK_CNT*2);
		if(gmedDataHdlStat.dataHdlTimer != NULL)
		{
			if(osTimerIsRunning(gmedDataHdlStat.dataHdlTimer))
				osTimerStop(gmedDataHdlStat.dataHdlTimer);
		}
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medDataHandleStop_1, P_DEBUG, "dmaBufFillCnt[%d]",gDataHdlCtlInfo.dmaBufFillCnt);
	}
	gmedDataHdlStat.medDataHandleFlag = MED_DATA_HDL_STA_STOP;
	if(rawDataCache)
	{
		xEcRingDelete(rawDataCache);
		rawDataCache = NULL;
	}

}


 ecRingT*	medInitDataCache(uint32_t cacheSize, uint32_t preFillSize)
{
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medInitDataCache, P_DEBUG, "cacheSize_%d ,preFillSize_%d ",cacheSize,preFillSize);
	if(rawDataCache == NULL)
	{
		rawDataCache = pxEcRingCreate(cacheSize);

		EC_ASSERT(rawDataCache,rawDataCache,0,0);
	}
	else
	{
		if(cacheSize != xEcRingGetOption(rawDataCache,E_LRO_BUFF_SIZE))
		{
			xEcRingDelete(rawDataCache);
			rawDataCache = pxEcRingCreate(cacheSize);

			EC_ASSERT(rawDataCache,rawDataCache,0,0);
		}
	}
	
	xEcRingClear(rawDataCache);
	rawDataCache->idataCnt = rawDataCache->iPosWrite = preFillSize;
	return rawDataCache;
}

ecRingT*	medGetDataCache(void)
{
	return rawDataCache;
}

int32_t   medGetDataCacheAvlbSize(void)
{
	int32_t avlbSize = 0;
	if(rawDataCache)
		avlbSize = xEcRingGetOption(rawDataCache, E_LRO_FREE_SIZE);
#if MED_DATA_HANDLE_DEBUG
	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, medGetDataCacheAvlbSize, P_DEBUG, "avlbSize_%d ",avlbSize);
#endif
   return avlbSize;
}


uint8_t medDataHandleStateGet(void)
{
	return gmedDataHdlStat.medDataHandleFlag;
}

void medDataHandleReset(void)
{
	uint32_t size = 0;
	//if(osTimerIsRunning(gmedDataHdlStat.dataHdlTimer))
	{
		//osTimerStop(gmedDataHdlStat.dataHdlTimer);
		if(rawDataCache)
		{
			xEcRingClear(rawDataCache);
			size = xEcRingGetOption(rawDataCache, E_LRO_BUFF_SIZE);
			rawDataCache->idataCnt = rawDataCache->iPosWrite = (size / 2);
		}
		memset(dataTrunckBuf,0,MED_DATA_TRUNCK_SIZE_MAX*MED_DATA_TRUNCK_BLK_CNT*2);
		gDataHdlCtlInfo.dmaBufFillCnt = 0;
		//osTimerStart(gmedDataHdlStat.dataHdlTimer,MED_DATA_HANDLE_CYCLE_TIME);	
	}
}

