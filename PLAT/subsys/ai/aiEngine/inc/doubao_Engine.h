/****************************************************************************
 *
 * @Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * @File name:    doubao_Engine.h
 * @Description:  EC718 doubao rtc function
 * @History:      Rev1.0   2025-04-27
 *
 ****************************************************************************/
#ifndef __DOUBAO_ENGINE_H__
#define __DOUBAO_ENGINE_H__
/*----------------------------------------------------------------------------*
 *                   MACRO DEFINITION                                  		  *
 *----------------------------------------------------------------------------*/
//#define AUD_LOOP_TEST					/*  rtc audio loop test switch     */
//#define DOUBAO_RTC_AUDIO_DUMP			/*  rtc audio data dump switch     */
//#define COZE_RTC_FEATURE				/*  rtc connect to czoe switch     */

#ifdef COZE_RTC_FEATURE
#define COZE_UL_EVENT_TYPE_SUBMIT_TOOL_OUTPUTS	1
#define COZE_UL_EVENT_TYPE_CANCEL				2
#endif


#define RTC_FUNCTION_CALL_PARAM_NUM		0
#define RTC_FUNCTION_CALL_PARAM_STR		1


#define RTC_BOOT_PAD_INDEX       15      // GPIO0
#define RTC_BOOT_PAD_ALT_FUNC    PAD_MUX_ALT0
#define RTC_BOOT_GPIO_INSTANCE   0       // GPIO0
#define RTC_BOOT_GPIO_INDEX      0       // GPIO0

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef enum _EPAT_aiEngPorcFlag{
	AI_ENG_START_VOICE_CHART_FLAG 		= 0x01,	  /*flag for start voice chat */
	AI_ENG_STOP_VOICE_CHART_FLAG,				  /*flag for stop voice chat  */
	AI_ENG_UPDATE_VOICE_CHART_FLAG,				  /*flag for update voice chat*/
	AI_ENG_CREATE_ROOM_FLAG,					  /*flag for create room(coze)*/
	AI_ENG_START_RTC_FLAG		  		= 0x10	  /*flag for start rtc engine */
}aiEngProcFlag;

typedef struct {
	uint8_t procFlag;
}QueueDoubaoEngT;

typedef struct {
	uint8_t paramType;
	uint32_t paramLen;
	union {
		int32_t numValue;
		char *strValue;
		}paramValue;
}fcParamDescT;

typedef enum __EPAT_rtcLocalFcE{
	RTC_FC_SET_VOL,
	RTC_FC_SET_BRIGHTLESS,
}rtcLocalFcE;
typedef void (* fcFunc)(char *funcId,char *chatId,void *param);

typedef enum{
	RTC_FC_SET_VOL_MIN,
	RTC_FC_SET_VOL_LOW,	
	RTC_FC_SET_VOL_UP,
	RTC_FC_SET_VOL_MAX,
	RTC_LOCAL_SET_VOL_LOW,
	RTC_LOCAL_SET_VOL_UP,
}rtcFcSetVolValueE;

typedef enum{
	RTC_FC_SET_LIGHT_MIN,
	RTC_FC_SET_LIGHT_LOW,
	RTC_FC_SET_LIGHT_UP,
	RTC_FC_SET_LIGHT_MAX,
}rtcFcSetLightValueE;


typedef struct {
	rtcLocalFcE fcDesc;
	fcParamDescT paramDesc;
	char *funcID;
	char *chatId;
}QueueRtcFcFlagT;

/*----------------------------------------------------------------------------*
 *                    		 FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/*测试用，MINDKB上使用两个按键触发rtc启动和关闭*/
void Pad3_WakeupIntHook(void);			/*WAKEUP3,set volume (aikit low,miniDKB up)  */
#ifdef BSP_AIKIT
void Pad4_WakeupIntHook(void);			/*WAKEUP4,volume up				    		*/
void pwrKey_doubleCkickHook(void);	
#else
void Pad5_WakeupIntHook(void);			/*WAKEUP5, volume up		                */
#endif
void Pad0_WakeupIntHook(void);			/*WAKEUP0,start/interrupt intelligent agent */
void pwrKey_wakeupHook(void);			/*POWEY KEY，  close rtc engine		        */


/**
 @brief     rtc operation process
 @param[in] process flag refer to <aiEngProcFlag>
 @return	progress result
*/
int32_t doubao_rtc_proc(uint8_t procFlag);

/**
 @brief     doubao rtc task init
 @return	void
*/
int32_t doubao_task_init(void);

void doubao_fc_set_volume(char *funcId,char *chatId,void *param);
void doubao_fc_set_lightness(char *funcId,char *chatId,void *param);

#endif/*__DOUBAO_ENGINE_H__*/
