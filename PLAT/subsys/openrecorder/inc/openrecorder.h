#ifndef __OPEN_RECORDER_API_H__
#define __OPEN_RECORDER_API_H__

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef void * openRecorder;
typedef void (*openRecorderCpltCallbackT)(void *userData,int32_t result);
typedef void (*openRecorderGetDataCallbackT)(void *,uint32_t);

typedef union _openRecorderConfigT{
	uint8_t config[5];
	struct  {
		uint32_t recordTime:	18;///<record time:second,0----record continuously
		uint32_t channel:		2; ///< recorder channel mono or dual
		uint32_t samplerate:	4; ///< samplerate
		uint32_t codec:	    	8; ///< the codec to be used,reserved 
		uint8_t resv1:        	4; ///reserved
		uint8_t resv2:        	4; ///reserved
		}recordParam;
}openRecorderConfigT;

typedef enum
{
	OPEN_REC_RET_OK,
	OPEN_REC_RET_ERR_NOMAL 			= -1,///< Unspecified RTOS error: run-time error but no other error message fits.
	OPEN_REC_RET_ERR_TIMEOUT 		= -2,///< Operation not completed within the timeout period.
	OPEN_REC_RET_ERR_RESOURCE 		= -3,///< Resource not available.
	OPEN_REC_RET_ERR_PARAM 			= -4,///< Parameter error.
	OPEN_REC_RET_ERR_PARAM_NOMEM 	= -5,///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
	OPEN_REC_RET_ERR_ISR 			= -6,///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
}openRecorderErrCodeT;

typedef enum {
	OPEN_RECORD_STA_IDLE,				///<< init or idle status
	OPEN_RECORD_STA_START,				///<< start record status
	OPEN_RECORD_STA_PAUSE,				///<< pause status
}openRecorderRecStatus;

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
/**
 \brief    create recorder handler
 \param[in] open recorder param,if NULL,use the default param
 \return	recorder handler or NULL
*/
 openRecorder openRecorderCreate(openRecorderConfigT *param);

/**
 \brief    destory recorder handler
 \param[in] recorder handler
 \return	void
*/
void openRecorderDestory(openRecorder recorder);

/**
 \brief    set recorder completed callback
 \param[in] recorder handler
 \param[in] recorder completed callback
 \param[in] userdate
 \return	void
*/
void openRecorderSetCallback(openRecorder recorder,openRecorderCpltCallbackT callback, void * userdata);

 /**
  \brief	set recorder get data callback.The callback will be called every 20ms,
  \			 the record data will be sent to the callback and not be saved to the file. 
  \param[in] recorder handler
  \param[in] recorder completed callback
  \param[in] userdate
  \return	 void
 */
 void openRecorderGetDataCallback(openRecorder recorder,openRecorderGetDataCallbackT callback);

 /**
  \brief   start record
  \param[in] recorder handler
  \param[in] dst,the destination to save the record source,it can be path or filename 
  \return    0-----success, < 0-----fail
 */
int8_t openRecorderStart(openRecorder recorder,void *dst);

/**
 \brief     pause recorder API
 \param[in] recorder handler
 \return	void
*/
void openRecorderPause(openRecorder recorder);

/**
 \brief    stop recorder API
 \param[in] recorder handler
 \return	void
*/
void openRecorderStop(openRecorder recorder);

/**
 \brief    recorder resume  API
 \param[in] recorder handler
 \return    0-----success, < 0-----fail
*/
int8_t openRecorderResume(openRecorder recorder);

/**
 \brief    get recoder time API
 \param[in] recorder handler
 \return:recorder time
*/
uint32_t openRecorderGetRecTime(openRecorder recorder);

/**
 \brief    recorder get pause status API
 \param[in] recorder handler
 \return true ---pause,false ---unpaused
*/
bool openRecorderIsPause(openRecorder recorder);

#endif/*__OPEN_RECORDER_API_H__*/
