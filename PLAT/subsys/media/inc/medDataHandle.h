#ifndef __MED_DATA_HANDLE_H__
#define __MED_DATA_HANDLE_H__
#include "ec_ring.h"

#define MED_DATA_HANDLE_DEBUG				 		(0) ///< for debug control

typedef enum _EPAT_medDataEnvType_e{
	MED_DATA_ENV_TYPE_LOCAL,
	MED_DATA_ENV_TYPE_TONE
}medDataEnvType_E;

typedef struct medDataHandlCtrlInfo{
	uint8_t hdlFlag;
	uint8_t trunckIndex;		
	uint32_t dmaBufFillCnt;
}medDataHandleCtrlInfo;

typedef enum
{
	MED_DATA_HDL_OK,
	MED_DATA_HDL_ERR_NOMAL 			= -1,///< Unspecified RTOS error: run-time error but no other error message fits.
	MED_DATA_HDL_ERR_TIMEOUT 		= -2,///< Operation not completed within the timeout period.
}medDataHdlErrCodeT;

typedef enum _EPAT_medDataHdlSta
{
	MED_DATA_HDL_STA_IDLE,
	MED_DATA_HDL_STA_START,
	MED_DATA_HDL_STA_SUSPEND,
	MED_DATA_HDL_STA_STOP,
}medDataHdlStaT;

typedef union{
	char byte[2];
	struct{
	uint16_t bitWidth:		2;
	uint16_t samplerate:	4;
	uint16_t envType:		2;
	uint16_t resved:		10;
		}field;
}rawDataHanderParam_T;

typedef void (*medDataCpltCallBack)(void *param, uint8_t* buffer, uint32_t size);

uint8_t 	medDataHandleStart(rawDataHanderParam_T *handleParam);
void 		medDataHandleStop(void);
uint8_t 	medDataHandleStateGet(void);

ecRingT*	medInitDataCache(uint32_t cacheSize, uint32_t preFillSize);
ecRingT*	medGetDataCache(void);
int32_t   	medGetDataCacheAvlbSize(void);
int medSetDataCpltCallBack(medDataCpltCallBack callback, void* param);

#endif
