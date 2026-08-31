/**
  * @file doubao_engine.c
  * @brief doubao engine function
  * @data 	2024-3-3
  * @version Rev1.0
  * @copyright  2017-, Copyrigths of EigenComm Ltd.
  */

/*----------------------------------------------------------------------------*
 *                   	HEADER INC                                    		  *
 *----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include DEBUG_LOG_HEADER_FILE

#include "cmsis_os2.h"
#include "slpman.h"
#include "osasys.h"
#include "ps_event_callback.h"
#include "networkmgr.h"
#ifdef FEATURE_SUBSYS_AI_DOUBAO_ENABLE
#include "doubao_engine.h"
#include "volcEngineRtcLite.h"
#include "cJSON.h"
#endif
#include "open_httpc.h"
#include "media.h"
#include "record.h"
#include "openRecorder.h"
#include "openplayer.h"
#include "g711.h"
#include "ec_ring.h"
#include "doubao_bot_util.h"
#include "doubao_bot_config.h"
#include "volc_time.h"

#include "mw_nvm_config.h"
#include "ccio_atcmd.h"
#include "ccio_message.h"
#include "ccio_audio.h"

#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
#include "lcdDrv.h"
#include "main_lcd.h"
#endif
#include "mw_nvm_ai.h"
/*----------------------------------------------------------------------------*
 *                   	MACRO                                     			  *
 *----------------------------------------------------------------------------*/
#define THREAD_STACK_SIZE_DOUBAO    		(30*1024)
#define THREAD_STACK_SIZE_FUNCTION_CALL   	(10*1024)

#define AI_AUDIO_STREAM_BUFF_SIZE			(64*1024)
#define QUEUE_SIZE_DOUBAO					(100)
#define QUEUE_SIZE_FC						(100)

#define DEFAULT_FPS_VALUE                   (25)
#define NUMBER_OF_OPUS_FRAME_FILES          (618)
#define SAMPLE_AUDIO_FRAME_DURATION         (20)

#define MAX_APPID_LEN       				(64)
#define MAX_ROOMID_LEN      				(64)
#define MAX_USERID_LEN      				(64)
#define MAX_video_file_LEN   				(256)
#define MAX_audio_file_LEN   				(256)
#define MAX_TOKEN_LEN       				(256)
#define MAX_PATH_LEN        				(1024)


/*----------------------------------------------------------------------------*
 *                      DATA TYPE DEFINITION                                  *
 *----------------------------------------------------------------------------*/
typedef struct {
	char app_id[MAX_APPID_LEN];
	char room_id[MAX_ROOMID_LEN];
	char user_id[MAX_USERID_LEN];
	char video_file[MAX_video_file_LEN];
	char audio_file[MAX_audio_file_LEN];
	char token[MAX_TOKEN_LEN];
	bool voiceChartStarted;
	bool channel_joined;
	bool is_send_video;
	bool is_send_audio;
	osThreadId_t  send_video_thread_id;
	osThreadId_t  send_audio_thread_id;
	int audio_codec;
	int fps;
	byte_rtc_engine_t engine;
	bool fini_notifyed;
} byte_rtc_data;



/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

static osMessageQueueId_t 	gDoubaoQueue = NULL;
static osMessageQueueId_t 	gRtcFcQueue = NULL;

static osEventFlagsId_t 	gAiPlayProcFlag = NULL;
static osEventFlagsId_t 	gAiNotiFlag = NULL;
static bool 				muteFlag = false;
static int					gInterruptMode = 0;
#ifdef THREAD_STATIC
static uint8_t            gDoubaoThreadStackMem[THREAD_STACK_SIZE_DOUBAO] = {0};
static StaticTask_t       gDoubaoThreadCbMem                              = {0};
static uint8_t            grtcFcThreadStackMem[THREAD_STACK_SIZE_FUNCTION_CALL] = {0};
static StaticTask_t       grtcFcThreadCbMem                              = {0};
#endif

extern openPlayer gOpenPlayHandler;
extern openRecorder gOpenRecordHandler;
extern ecRingT *gAudStreamRingBuf;
ecRingT *gAiAudCache = NULL;

byte_rtc_data g_byte_rtc_data;

char rtcPostHeader[1500] = {0};
fcFunc gFcFuncList[] = {doubao_fc_set_volume, \
						doubao_fc_set_lightness};

#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
extern osMessageQueueId_t 	gSubtitleQueue;
extern lcdDrvFunc_t* lcdDev;
#endif

ecAiCfgStore_t 	gAiNvmCfg = {{0},{0}};

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/
static void doubao_audio_player_fill(uint8_t *data,uint32_t size);
static void aiRecordDataCallback(uint8_t *data,uint32_t size);
static void doubao_audio_proc_start(void);

static void byte_rtc_on_join_channel_success(byte_rtc_engine_t engine,const char *channel, int elapsed_ms, bool rejoin) {
	g_byte_rtc_data.channel_joined = true;
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_join_channel_success, P_DEBUG,"join channel success %s elapsed %d ms\n", channel, elapsed_ms);
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
	if(gSubtitleQueue)
	{
		QueueRtcSubtitleT queue = {RTC_SHOW_RTC_START,NULL};
		osMessageQueuePut(gSubtitleQueue,&queue,0,0);
	}
#endif
};

static void byte_rtc_on_user_joined(byte_rtc_engine_t engine,const char *channel, const char *user_name,int elapsed_ms){
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_user_joined, P_DEBUG,"remote user joined  %s:%s\n",channel,user_name);
};

static void byte_rtc_on_user_offline(byte_rtc_engine_t engine,const char *channel, const char *user_name , int reason){
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_user_offline, P_DEBUG,"remote user offline  %s:%s\n",channel,user_name);
};

static void byte_rtc_on_user_mute_audio(byte_rtc_engine_t engine,const char *channel, const char *user_name ,int muted){
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_user_mute_audio, P_DEBUG,"remote user mute audio  %s:%s %d\n",channel,user_name,muted);

};

static void byte_rtc_on_user_mute_video(byte_rtc_engine_t engine,const char *channel, const char *user_name ,int muted){
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_user_mute_video, P_DEBUG,"remote user mute video  %s:%s %d\n",channel,user_name,muted);
};

static void __attribute__((used)) byte_rtc_on_connection_lost(byte_rtc_engine_t engine,const char *channel){
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_connection_lost, P_DEBUG,"connection Lost  %s\n",channel);
};
/*下行tts音频*/
static void byte_rtc_on_audio_data(byte_rtc_engine_t engine,const char *channel, const char *user_name ,uint16_t sent_ts,
					  audio_codec_type_e codec, const void *data_ptr, size_t data_len){
#ifdef DOUBAO_RTC_AUDIO_DUMP
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_audio_data, P_DEBUG,"codec(%d),channel(%s)",codec,channel);
	ECPLAT_DUMP(UNILOG_PLAT_DOUBAO_AI, audio_data_dump, P_DEBUG,"",data_len,data_ptr);
#endif
	if(!muteFlag)
		doubao_audio_player_fill((uint8_t *)data_ptr,data_len);						
};

static void byte_rtc_on_video_data(byte_rtc_engine_t engine,const char *channel, const char *user_name ,uint16_t sent_ts,
					  video_data_type_e codec,	int is_key_frame,
					  const void *data_ptr, size_t data_len) 
{
	if (is_key_frame) {
		ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_video_data, P_DEBUG,"received key frame of user: %s\n", user_name);
	}
};

static void byte_rtc_on_channel_error(byte_rtc_engine_t engine,const char *channel, int code, const char *msg){
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_channel_error, P_DEBUG,"error occur %s %d %s\n",channel,code,msg?msg:"");
};

static void byte_rtc_on_gloable_error(byte_rtc_engine_t engine,int code, const char *msg){
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_gloable_error, P_DEBUG,"global error occur %d %s\n",code,msg?msg:"");
};

static void byte_rtc_on_keyframe_gen_req(byte_rtc_engine_t engine,const char *channel, const char *user_name){
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_keyframe_gen_req, P_DEBUG,"remote req key frame %s:%s\n",channel,user_name);
};

static void byte_rtc_on_target_bitrate_changed(byte_rtc_engine_t engine,const char *channel, uint32_t target_bps){
};

static void byte_rtc_on_token_privilege_will_expire(byte_rtc_engine_t engine,const char *token) {
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_token_privilege_will_expire, P_ERROR,"token privilege will expire %s",token);
};
static void byte_rtc_on_fini_notify(byte_rtc_engine_t engine) {
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, byte_rtc_on_fini_notify, P_ERROR,"byte_rtc_on_fini_notify");
	g_byte_rtc_data.fini_notifyed = true;
	if(gAiNotiFlag)
		osEventFlagsSet(gAiNotiFlag,0x01);
}

// function calling 消息 参考 https://www.volcengine.com/docs/6348/1359441
static void on_function_calling_message_received(const cJSON* root,char *chat_id) {
  
    // 收到function calling 消息，需要根据具体情况要在服务端处理还是客户端处理   
    // 在客户端处理,通过byte_rtc_rts_send_message接口通知智能体
    
    cJSON* tool_obj_arr = cJSON_GetObjectItem(root, "tool_calls");
    cJSON* obji = NULL;
	cJSON* paramValue_obj = NULL;
	uint8_t value = 0;
    cJSON_ArrayForEach(obji, tool_obj_arr) {
        cJSON* id_obj = cJSON_GetObjectItem(obji, "id");
        cJSON* function_obj = cJSON_GetObjectItem(obji, "function");
        if (id_obj && function_obj) {
            cJSON* arguments_obj = cJSON_GetObjectItem(function_obj, "arguments");
            cJSON* name_obj = cJSON_GetObjectItem(function_obj, "name");
			const char* func_name = cJSON_GetStringValue(name_obj);
			cJSON *praram = NULL;
			cJSON *arg_json = cJSON_Parse(arguments_obj->valuestring);
			if (arg_json != NULL)
			{
				praram = cJSON_GetObjectItemCaseSensitive(arg_json, func_name);
				if (cJSON_IsNumber(praram)) 
				{
				    value = praram->valueint;
				}
          	}

			ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, on_function_calling_message_received, P_DEBUG,"%s:%d",func_name,value);
			const char* func_id = cJSON_GetStringValue(id_obj);
			if( gRtcFcQueue && func_id)
			{				
				QueueRtcFcFlagT queue = {0};
				memset(&queue,0,sizeof(QueueRtcFcFlagT));
				queue.funcID = (char *)calloc(strlen(func_id) + 1,sizeof(char));
				if(chat_id)
				{
					queue.chatId =  (char *)calloc(strlen(chat_id) + 1,sizeof(char));
					EC_ASSERT(queue.chatId,queue.chatId,0,0);
					memcpy(queue.chatId,chat_id,strlen(chat_id));
				}
				EC_ASSERT(queue.funcID,queue.funcID,0,0);
				memcpy(queue.funcID,func_id,strlen(func_id));
				if(!memcmp(func_name,"set_volume",10))
				{
					queue.fcDesc = RTC_FC_SET_VOL;
					queue.paramDesc.paramType = RTC_FUNCTION_CALL_PARAM_NUM;
					queue.paramDesc.paramValue.numValue = value;
					
				}
				else if(!memcmp(func_name,"brightness",10))
				{
					queue.fcDesc = RTC_FC_SET_BRIGHTLESS;
					queue.paramDesc.paramType = RTC_FUNCTION_CALL_PARAM_NUM;
					queue.paramDesc.paramValue.numValue = value;
				}
				osMessageQueuePut(gRtcFcQueue,&queue,0,3);
			}
			
        }
    }
}


static void on_subtitle_message_received(byte_rtc_engine_t engine, const cJSON* root) {
    /*
        {
            "data" : 
            [
                {
                    "definite" : false,
                    "language" : "zh",
                    "mode" : 1,
                    "paragraph" : false,
                    "sequence" : 0,
                    "text" : "\\u4f60\\u597d",
                    "userId" : "voiceChat_xxxxx"
                }
            ],
            "type" : "subtitle"
        }
    */
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
    QueueRtcSubtitleT queue = {RTC_SHOW_IDLE,NULL};
#endif
	uint32_t textLen = 0;
    cJSON * type_obj = cJSON_GetObjectItem(root, "type");
    if (type_obj != NULL && strcmp("subtitle", cJSON_GetStringValue(type_obj)) == 0) {
        cJSON* data_obj_arr = cJSON_GetObjectItem(root, "data");
        cJSON* obji = NULL;
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE	
		char *subPtr = NULL;
#endif
        cJSON_ArrayForEach(obji, data_obj_arr) {
            cJSON* user_id_obj = cJSON_GetObjectItem(obji, "userId");
            cJSON* text_obj = cJSON_GetObjectItem(obji, "text");
			  cJSON* definite_obj = cJSON_GetObjectItem(obji, "definite");
#ifndef BSP_AIKIT			
 			if (user_id_obj && text_obj && (cJSON_IsBool(definite_obj) && cJSON_IsTrue(definite_obj))) 
#else
            if (user_id_obj && text_obj && (cJSON_IsBool(definite_obj))) 
#endif
			{
                ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,on_subtitle_message_received,P_DEBUG,"subtitle:%s:%s", cJSON_GetStringValue(user_id_obj), cJSON_GetStringValue(text_obj));
#ifndef FEATURE_SUBSYS_GUI_LVGL_ENABLE				
				printf("%s: %s\r\n",cJSON_GetStringValue(user_id_obj),cJSON_GetStringValue(text_obj));
#else
				if(!memcmp(cJSON_GetStringValue(user_id_obj),DOUBAO_TEST_USER_ID,strlen(DOUBAO_TEST_USER_ID)))
				{
					muteFlag = false;
					queue.showFlag = RTC_SHOW_SUBTITLE_SELF;
				}
				else
				{
					queue.showFlag = RTC_SHOW_SUBTITLE_ROBOT;
				}
				subPtr = cJSON_GetStringValue(text_obj);
				textLen = strlen(subPtr) + 1;
				if(textLen && gSubtitleQueue)
				{
					queue.subtitle = (char *)malloc(textLen);
					EC_ASSERT(queue.subtitle,queue.subtitle,0,0);
					memset(queue.subtitle,0x00,textLen);
					memcpy(queue.subtitle, subPtr,textLen - 1);
#ifdef BSP_AIKIT
					queue.definite = cJSON_IsTrue(definite_obj);
#endif
					osMessageQueuePut(gSubtitleQueue,&queue,0,0);
				}
#endif				
            }
        }
    }
}

/*获取字幕回调函数*/
void byte_rtc_on_message_received(byte_rtc_engine_t engine, const char*  room, const char* uid, const uint8_t* message, int size, bool binary) {
    // 字幕消息，参考https://www.volcengine.com/docs/6348/1337284
    // subv|length(4)|json str
    //
    // function calling 消息，参考https://www.volcengine.com/docs/6348/1359441
    // tool|length(4)|json str
	//ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,byte_rtc_on_message_received,P_DEBUG,"%s(%d): %s",uid,binary,message);
    static char message_buffer[4096];
#ifndef COZE_RTC_FEATURE
    if (size > 8) {
        memcpy(message_buffer, message, size);
        message_buffer[size] = 0;
        message_buffer[size + 1] = 0;

        cJSON *root = cJSON_Parse(message_buffer + 8);
        if (root != NULL) {
            if (message[0] == 's' && message[1] == 'u' && message[2] == 'b' && message[3] == 'v') {
                // 字幕消息
                on_subtitle_message_received(engine, root);
            } else if (message[0] == 't' && message[1] == 'o' && message[2] == 'o' && message[3] == 'l') {
            ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,byte_rtc_on_message_received_0,P_DEBUG,"function calling message: %s",message_buffer + 8);
                // function calling 消息
                on_function_calling_message_received(root,NULL);
            } else {
            ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,byte_rtc_on_message_received_1,P_DEBUG,"unknown json message: %s", message_buffer + 8);
            }
            cJSON_Delete(root);
        } else {
			ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,byte_rtc_on_message_received_2,P_DEBUG,"unknown message.");
        }
    } else {
        ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,byte_rtc_on_message_received_3,P_DEBUG,"unknown message.");
    }
#else
	 if (size > 0) {
        memcpy(message_buffer, message, size);
        message_buffer[size] = 0;
        message_buffer[size + 1] = 0;
		QueueRtcSubtitleT queue = {RTC_SHOW_IDLE,NULL};
		cJSON *root = cJSON_Parse(message_buffer);
		if(root != NULL)
		{
			cJSON *event_type = cJSON_GetObjectItemCaseSensitive(root, "event_type");
			cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");
			if (cJSON_IsObject(data) && cJSON_IsString(event_type))
			{
		        char *strEventType = cJSON_GetStringValue(event_type);
				if(!strcmp(strEventType,"conversation.message.completed"))
				{

					cJSON *role = cJSON_GetObjectItemCaseSensitive(data, "role");
					cJSON *type = cJSON_GetObjectItemCaseSensitive(data, "type");
					cJSON *content = cJSON_GetObjectItemCaseSensitive(data, "content");
					if(cJSON_IsString(role) && role->valuestring != NULL && 
						cJSON_IsString(content) && content->valuestring != NULL &&
						cJSON_IsString(type) && type->valuestring != NULL)
					{
#ifndef FEATURE_SUBSYS_GUI_LVGL_ENABLE				
					printf("%s: %s\r\n",cJSON_GetStringValue(role),cJSON_GetStringValue(content));
#else
					char *subPtr = NULL;
					uint32_t textLen = 0;

					if(!memcmp(cJSON_GetStringValue(role),"user",4))
					{
						queue.showFlag = RTC_SHOW_SUBTITLE_SELF;
					}
					else
						queue.showFlag = RTC_SHOW_SUBTITLE_ROBOT;
					if(!memcmp(cJSON_GetStringValue(type),"answer",6) || !memcmp(cJSON_GetStringValue(type),"question",8))
					{
						subPtr = cJSON_GetStringValue(content);
						textLen = strlen(subPtr) + 1;
					}					
					if(textLen && gSubtitleQueue)
					{
						queue.subtitle = (char *)malloc(textLen);
						EC_ASSERT(queue.subtitle,queue.subtitle,0,0);
						memset(queue.subtitle,0x00,textLen);
						memcpy(queue.subtitle, subPtr,textLen - 1);
						osMessageQueuePut(gSubtitleQueue,&queue,0,0);
					}
#endif	
						
					}
					else
						ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,byte_rtc_on_message_received_valid_0,P_ERROR,"content field not found or result fail");
				}
				else if(!strcmp(strEventType,"conversation.created"))
				{
					cJSON *prologue = cJSON_GetObjectItemCaseSensitive(data, "prologue");
					if(cJSON_IsString(prologue) && prologue->valuestring != NULL)
					{
#ifndef FEATURE_SUBSYS_GUI_LVGL_ENABLE				
						printf("%s: %s\r\n","assistant",cJSON_GetStringValue(prologue));
#else
						char *subPtr = NULL;
						uint32_t textLen = 0;

						queue.showFlag = RTC_SHOW_SUBTITLE_ROBOT;

						subPtr = cJSON_GetStringValue(prologue);
						textLen = strlen(subPtr) + 1;
				
						if(textLen && gSubtitleQueue)
						{
							queue.subtitle = (char *)malloc(textLen);
							EC_ASSERT(queue.subtitle,queue.subtitle,0,0);
							memset(queue.subtitle,0x00,textLen);
							memcpy(queue.subtitle, subPtr,textLen - 1);
							osMessageQueuePut(gSubtitleQueue,&queue,0,0);
						}
#endif
					}
				}
				else if(!strcmp(strEventType,"conversation.chat.requires_action"))/*function call*/
				{
					cJSON *required_action = cJSON_GetObjectItemCaseSensitive(data, "required_action");
					cJSON *chat_id = cJSON_GetObjectItemCaseSensitive(data, "id");
					if(cJSON_IsObject(required_action))
					{
						cJSON *submit_tool_outputs = cJSON_GetObjectItemCaseSensitive(required_action,"submit_tool_outputs");
						if(cJSON_IsObject(submit_tool_outputs) && cJSON_IsString(chat_id))
						{
							on_function_calling_message_received(submit_tool_outputs,chat_id->valuestring);
						}
					}
				}
				else 
				{				
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,byte_rtc_on_message_received_valid_1,P_ERROR,"other message");
		        }
		    }
			cJSON_Delete(root);
		}
 	}	
#endif
}


static int create_start_voice_chat_data(uint8_t procType,char **str)
{
	cJSON *body = cJSON_CreateObject();
	cJSON_AddStringToObject(body, "AppId", g_byte_rtc_data.app_id);
	cJSON_AddStringToObject(body, "RoomId", g_byte_rtc_data.room_id);
	cJSON_AddStringToObject(body, "TaskId", "ecTask02");

	switch(procType)
	{
		case AI_ENG_START_VOICE_CHART_FLAG:
		{			
			cJSON *config = cJSON_CreateObject();
			cJSON *asrConfig = cJSON_CreateObject();
			cJSON_AddStringToObject(asrConfig, "Provider", "volcano");

			cJSON *asrProviderParams = cJSON_CreateObject();
			cJSON_AddStringToObject(asrProviderParams, "Mode", "bigmodel");
			cJSON_AddStringToObject(asrProviderParams, "AppId", ACCESS_APPID);
			cJSON_AddStringToObject(asrProviderParams, "AccessToken", ACCESS_TOKEN);
			cJSON_AddItemToObject(asrConfig, "ProviderParams", asrProviderParams);
			cJSON_AddItemToObject(config, "ASRConfig", asrConfig);

			cJSON *ttsConfig = cJSON_CreateObject();
			cJSON_AddStringToObject(ttsConfig, "Provider", "volcano");//volcano_bidirection

			cJSON *ttsProviderParams = cJSON_CreateObject();
			cJSON *ttsApp = cJSON_CreateObject();
			cJSON_AddStringToObject(ttsApp, "token", ACCESS_TOKEN);
			cJSON_AddStringToObject(ttsApp, "AppId", ACCESS_APPID);
			cJSON_AddItemToObject(ttsProviderParams, "app", ttsApp);
			
			//cJSON_AddStringToObject(ttsProviderParams, "ResourceId", "volc.service_type.10029");
			cJSON *ttsAudioParams = cJSON_CreateObject();			

			cJSON_AddStringToObject(ttsAudioParams, "voice_type", "BV005_streaming");
			cJSON_AddItemToObject(ttsProviderParams, "audio", ttsAudioParams);
			cJSON_AddItemToObject(ttsConfig, "ProviderParams", ttsProviderParams);

			
			//cJSON_AddItemToObject(ttsConfig, "audio", ttsAudioParams);
			
			cJSON_AddItemToObject(config, "TTSConfig", ttsConfig);

			cJSON *llmConfig = cJSON_CreateObject();
			cJSON_AddStringToObject(llmConfig, "Mode", "ArkV3");

#if 1
			//function call begin
			cJSON *tools_array = cJSON_CreateArray();

		    cJSON *function_obj = NULL;
		    cJSON *function_info = NULL;
		    cJSON *parameters_obj = NULL;
		    cJSON *properties_obj = NULL;
		    cJSON *paramValue_obj = NULL;
			cJSON *required_array = NULL;
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
			// 创建function对象
		    function_obj = cJSON_CreateObject();
		    function_info = cJSON_CreateObject();
			// 创建parameters对象
		    parameters_obj = cJSON_CreateObject();
		    properties_obj = cJSON_CreateObject();
		    paramValue_obj = cJSON_CreateObject();
			
		    cJSON_AddStringToObject(function_obj, "type", "function");
		    cJSON_AddItemToObject(function_obj, "function", function_info);

		    // 设置function的name和description
		    cJSON_AddStringToObject(function_info, "name", "brightness");
		    cJSON_AddStringToObject(function_info, "description", "调节屏幕亮度");
		    
		    cJSON_AddStringToObject(parameters_obj, "type", "object");
		    cJSON_AddItemToObject(parameters_obj, "properties", properties_obj);
		    cJSON_AddItemToObject(properties_obj, "brightness", paramValue_obj);
		    cJSON_AddStringToObject(paramValue_obj, "type", "integer");
		    cJSON_AddStringToObject(paramValue_obj, "description", "亮度调节方式,0代表最暗,1是调暗,2是调亮,3最亮");

		    // 创建required数组
		    required_array = cJSON_CreateArray();
		    cJSON_AddItemToArray(required_array, cJSON_CreateString("brightness"));
		    cJSON_AddItemToObject(parameters_obj, "required", required_array);

		    // 将parameters对象添加到function_info中
		    cJSON_AddItemToObject(function_info, "parameters", parameters_obj);			
		    // 将function对象添加到Tools数组中
		    cJSON_AddItemToArray(tools_array, function_obj);
#endif/*FEATURE_SUBSYS_GUI_LVGL_ENABLE*/
			//set volume start
			function_obj = cJSON_CreateObject();
		    function_info = cJSON_CreateObject();
			parameters_obj = cJSON_CreateObject();
		    properties_obj = cJSON_CreateObject();
		    paramValue_obj = cJSON_CreateObject();
			
		    cJSON_AddStringToObject(function_obj, "type", "function");
		    cJSON_AddItemToObject(function_obj, "function", function_info);

		    // 设置function的name和description
		    cJSON_AddStringToObject(function_info, "name", "set_volume");
		    cJSON_AddStringToObject(function_info, "description", "调节音量");

		    // 创建parameters对象
		    cJSON_AddStringToObject(parameters_obj, "type", "object");
		    cJSON_AddItemToObject(parameters_obj, "properties", properties_obj);
		    cJSON_AddItemToObject(properties_obj, "set_volume", paramValue_obj);
		    cJSON_AddStringToObject(paramValue_obj, "type", "integer");
			cJSON_AddStringToObject(paramValue_obj, "description", "音量调节方式,0代表最小或者静音,1是调低,2是调高,3是调到最高");
		    // 创建required数组
		    required_array = cJSON_CreateArray();
		    cJSON_AddItemToArray(required_array, cJSON_CreateString("set_volume"));
		    cJSON_AddItemToObject(parameters_obj, "required", required_array);

		    // 将parameters对象添加到function_info中
		    cJSON_AddItemToObject(function_info, "parameters", parameters_obj);			
		    // 将function对象添加到Tools数组中
		    cJSON_AddItemToArray(tools_array, function_obj);
			//set voluem end
			
			cJSON_AddItemToObject(llmConfig, "Tools", tools_array);
			//function call end
#endif			
			cJSON_AddStringToObject(llmConfig, "botId", DOUBAO_BOT_ID);
			cJSON_AddItemToObject(config, "LLMConfig", llmConfig);

			//cJSON_AddItemToObject(body, "Config", config);

			cJSON *SubtitleConfig = cJSON_CreateObject();
			cJSON_AddBoolToObject(SubtitleConfig, "DisableRTSSubtitle", false);
#ifndef BSP_AIKIT
			cJSON_AddNumberToObject(SubtitleConfig, "SubtitleMode", 1);
#else
			cJSON_AddNumberToObject(SubtitleConfig, "SubtitleMode", 0);
#endif
			cJSON_AddItemToObject(config, "SubtitleConfig", SubtitleConfig);
			cJSON_AddNumberToObject(config,"InterruptMode",gInterruptMode);
			cJSON_AddItemToObject(body, "Config", config);

			cJSON *agentConfig = cJSON_CreateObject();

			cJSON *targetUserId = cJSON_CreateArray();
			cJSON_AddItemToArray(targetUserId, cJSON_CreateString(g_byte_rtc_data.user_id));
			cJSON_AddItemToObject(agentConfig, "TargetUserId", targetUserId);
			cJSON_AddStringToObject(agentConfig, "WelcomeMessage", "你好移芯");
			cJSON_AddStringToObject(agentConfig, "UserId", "volc01");
			cJSON_AddItemToObject(body, "AgentConfig", agentConfig);
		}
			break;
		case AI_ENG_STOP_VOICE_CHART_FLAG:
			//cJSON_AddStringToObject(body, "UserId", g_byte_rtc_data.user_id);
			break;
		case AI_ENG_UPDATE_VOICE_CHART_FLAG:
			//cJSON_AddStringToObject(body, "UserId", g_byte_rtc_data.user_id);
			cJSON_AddStringToObject(body, "Command", "interrupt");
			break;
		default:
			break;
	}
	char *tmpstr = cJSON_Print(body);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,create_start_voice_chat_data,P_DEBUG,"%s",tmpstr);
	tmpstr = cJSON_PrintUnformatted(body);
	if (tmpstr != NULL) {
		*str = (char *)malloc(strlen(tmpstr) + 1);
		memset(*str,0x00,strlen(tmpstr) + 1);
		memcpy(*str,tmpstr,strlen(tmpstr));	
	}
	cJSON_Delete(body);

	return 0;
}

static void aiHttpRteCb(int8_t result, int8_t dataType,const void *extra, int32_t len)
{
	switch(dataType)
	{
		case HTTPC_RTEDATA_IDLE:
			ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,create_start_voice_chat_data_1,P_DEBUG,"HTTP CONNECT result : %d",result);
		break;
		case HTTPC_RTEDATA_HEADER:
			ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,create_start_voice_chat_data_2,P_DEBUG,"HTTP RSP HEAD : %s",(int8_t *)extra);
		break;
		case HTTPC_RTEDATA_CONTEXT:
		{
			ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,create_start_voice_chat_data_3,P_DEBUG,"HTTP RSP DATA : %s",(int8_t *)extra);
			cJSON *root = cJSON_Parse(extra);
			if (root == NULL) 
			{
				const char *error_ptr = cJSON_GetErrorPtr();
				if (error_ptr != NULL) 
				{
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,create_start_voice_chat_data_4,P_DEBUG,"Error before: %s",error_ptr);
				}
				return;
			}
			cJSON *result = cJSON_GetObjectItemCaseSensitive(root, "Result");
			cJSON *ResponseMetadata = cJSON_GetObjectItemCaseSensitive(root, "ResponseMetadata");
		    if (cJSON_IsObject(ResponseMetadata) && cJSON_IsString(result))
			{
		        cJSON *action = cJSON_GetObjectItemCaseSensitive(ResponseMetadata, "Action");
		        if (cJSON_IsString(action) && (action->valuestring != NULL)  && (result->valuestring != NULL))
				{
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,create_start_voice_chat_data_5,P_DEBUG,"%s result: %s",action->valuestring,result->valuestring);			

					if(/*!strncasecmp(result->valuestring,"ok",strlen("ok")) && */
						!strncasecmp(action->valuestring,DOUBAO_REQ_ACTON_START,strlen(DOUBAO_REQ_ACTON_START)) && gDoubaoQueue)
					{	
						QueueDoubaoEngT queue = {.procFlag = AI_ENG_START_RTC_FLAG};
						osMessageQueuePut(gDoubaoQueue,&queue,0,0);
					}
		        } 
				else 
				{
					if( gDoubaoQueue)
					{	
						QueueDoubaoEngT queue = {.procFlag = AI_ENG_STOP_VOICE_CHART_FLAG};
						osMessageQueuePut(gDoubaoQueue,&queue,0,0);
					}
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,create_start_voice_chat_data_6,P_ERROR,"Action field not found or result fail");
		        }
		    } 
			else 
		    {
		    	if( gDoubaoQueue)
				{	
					QueueDoubaoEngT queue = {.procFlag = AI_ENG_STOP_VOICE_CHART_FLAG};
					osMessageQueuePut(gDoubaoQueue,&queue,0,0);
				}
		       ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,create_start_voice_chat_data_7,P_ERROR,"ResponseMetadata field not found or result field not found");
		    }
						
			cJSON_Delete(root);
		}
		break;
		default:
		break;
	}
	return;
}

static void cozeHttpRteCb(int8_t result, int8_t dataType,const void *extra)
{
	switch(dataType)
	{
		case HTTPC_RTEDATA_IDLE:
			ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_1,P_DEBUG,"HTTP CONNECT result : %d",result);
		break;
		case HTTPC_RTEDATA_HEADER:
			ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_2,P_DEBUG,"HTTP RSP HEAD : %s",(int8_t *)extra);
		break;
		case HTTPC_RTEDATA_CONTEXT:
		{
			ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_3,P_DEBUG,"HTTP RSP DATA : %s",(int8_t *)extra);
			cJSON *root = cJSON_Parse(extra);
			if (root == NULL) 
			{
				const char *error_ptr = cJSON_GetErrorPtr();
				if (error_ptr != NULL) 
				{
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_4,P_DEBUG,"Error before: %s",error_ptr);
				}
				return;
			}
			cJSON *result = cJSON_GetObjectItemCaseSensitive(root, "code");
			if(cJSON_IsNumber(result))
			{
				uint32_t code = cJSON_GetNumberValue(result);
				if(code != 0)
				{
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_valid_0,P_ERROR,"code: %d",code);
				}
			}
			cJSON *ResponseMetadata = cJSON_GetObjectItemCaseSensitive(root, "data");
		    if (cJSON_IsObject(ResponseMetadata))
			{
		        cJSON *token = cJSON_GetObjectItemCaseSensitive(ResponseMetadata, "token");
				cJSON *roomId = cJSON_GetObjectItemCaseSensitive(ResponseMetadata, "room_id");
				cJSON *appId = cJSON_GetObjectItemCaseSensitive(ResponseMetadata, "app_id");
		        if ((cJSON_IsString(token) && token->valuestring != NULL) && 
					(cJSON_IsString(roomId) && roomId->valuestring != NULL) && 
					(cJSON_IsString(appId) && appId->valuestring != NULL))
				{
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_5,P_DEBUG,"token[%s]",cJSON_GetStringValue(token));
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_6,P_DEBUG,"roomId[%s]",cJSON_GetStringValue(roomId));
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_7,P_DEBUG,"appId[%s]",cJSON_GetStringValue(appId));
					
					memset(g_byte_rtc_data.app_id,0,MAX_APPID_LEN);
					memset(g_byte_rtc_data.room_id,0,MAX_ROOMID_LEN);
					memset(g_byte_rtc_data.token,0,MAX_TOKEN_LEN);
										
					memcpy(g_byte_rtc_data.app_id,cJSON_GetStringValue(appId),strlen(cJSON_GetStringValue(appId)));
					memcpy(g_byte_rtc_data.room_id,cJSON_GetStringValue(roomId),strlen(cJSON_GetStringValue(roomId)));
					memcpy(g_byte_rtc_data.token,cJSON_GetStringValue(token),strlen(cJSON_GetStringValue(token)));
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_8,P_DEBUG,"%s",g_byte_rtc_data.token);
					if( gDoubaoQueue)
					{	
						QueueDoubaoEngT queue = {.procFlag = AI_ENG_START_RTC_FLAG};
						osMessageQueuePut(gDoubaoQueue,&queue,0,0);
					}

		        } 
				else 
				{
					if( gDoubaoQueue)
					{	
						QueueDoubaoEngT queue = {.procFlag = AI_ENG_STOP_VOICE_CHART_FLAG};
						osMessageQueuePut(gDoubaoQueue,&queue,0,0);
					}
					ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_valid_1,P_ERROR,"Action field not found or result fail");
		        }
		    } 
			else 
		    {
		    	if( gDoubaoQueue)
				{	
					QueueDoubaoEngT queue = {.procFlag = AI_ENG_STOP_VOICE_CHART_FLAG};
					osMessageQueuePut(gDoubaoQueue,&queue,0,0);
				}
		       ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,cozeHttpRteCb_valid_2,P_ERROR,"ResponseMetadata field not found or result field not found");
		    }
						
			cJSON_Delete(root);
		}
		break;
		default:
		break;
	}
	return;
}

#ifdef COZE_RTC_FEATURE
static void coze_conversation_chat_ul(uint8_t eventType,cJSON *data)
{
	if(eventType < COZE_UL_EVENT_TYPE_SUBMIT_TOOL_OUTPUTS || COZE_UL_EVENT_TYPE_CANCEL < eventType || data == NULL)
	{
		return;
	}
	int64_t ret = 0;	
	static uint32_t id = 0;
	char strId[11] = {0};
	memset(strId,0,11);
	snprintf(strId,11,"%d",id++);

	cJSON *ctrl_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(ctrl_obj, "id", strId);
	switch(eventType)
	{
		case COZE_UL_EVENT_TYPE_CANCEL:
			cJSON_AddStringToObject(ctrl_obj, "event_type", "conversation.chat.cancel");
			break;
		case COZE_UL_EVENT_TYPE_SUBMIT_TOOL_OUTPUTS:
			cJSON_AddStringToObject(ctrl_obj, "event_type", "conversation.chat.submit_tool_outputs");
			break;
		default:
			break;
	}
	
	cJSON_AddItemToObject(ctrl_obj, "data", data);
    char *json_string = cJSON_Print(ctrl_obj);
    char ctrl_message_buffer[1024] = {0};
	memset(ctrl_message_buffer,0,1024);
    int json_str_len = strlen(json_string);

    memcpy(ctrl_message_buffer, json_string, json_str_len);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,coze_interrupt_chat,P_DEBUG,"send message: %s", json_string);
    cJSON_Delete(ctrl_obj);

    ret = byte_rtc_rts_send_message(g_byte_rtc_data.engine, g_byte_rtc_data.room_id, gAiNvmCfg.czInfo.bid, ctrl_message_buffer, json_str_len, 0, RTS_MESSAGE_RELIABLE);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,coze_interrupt_chat_1,P_DEBUG,"send message id : %d",ret);
}
#endif

static void coze_create_room(void)
{	
	char *strbody = NULL;
	char postUrl[128] = {0};

	memset(&gAiNvmCfg,0,sizeof(ecAiCfgStore_t));
    if(!mwNvmAiCfgRead(&gAiNvmCfg))
    {
        return;
    }
	snprintf(postUrl,128,"https://api.coze.cn/v1/audio/rooms");
	memset(rtcPostHeader,0x00,1500);
	sprintf(rtcPostHeader,"Authorization: Bearer %s\r\nContent-Type: application/json",gAiNvmCfg.czInfo.token);

	cJSON *body = cJSON_CreateObject();
	cJSON_AddStringToObject(body, "bot_id", gAiNvmCfg.czInfo.bid);

	cJSON *config = cJSON_CreateObject();
	cJSON *audioConfig = cJSON_CreateObject();
	cJSON_AddStringToObject(audioConfig,"codec",COZE_AUDIO_TYPE);
	cJSON_AddItemToObject(config,"audio_config",audioConfig);
	cJSON_AddItemToObject(body,"config",config);	
	cJSON_AddStringToObject(body, "uid", gAiNvmCfg.czInfo.uid);
	memcpy(g_byte_rtc_data.user_id,gAiNvmCfg.czInfo.uid,strlen(gAiNvmCfg.czInfo.uid));
	strbody = cJSON_Print(body);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,coze_create_room,P_DEBUG,"%s",strbody);
	openHttpcPost(postUrl,strbody,strlen(strbody),cozeHttpRteCb);
	
	cJSON_Delete(body);
}

static void rtc_post_request(uint8_t reqType)
{
	char *body = NULL;
	char *action = NULL;
	char postUrl[128] = {0};
	switch(reqType)
	{
		case AI_ENG_START_VOICE_CHART_FLAG:
			action = DOUBAO_REQ_ACTON_START;
			break;
		case AI_ENG_UPDATE_VOICE_CHART_FLAG:
			action = DOUBAO_REQ_ACTON_UPDATE;
			break;
		case AI_ENG_STOP_VOICE_CHART_FLAG:
			action = DOUBAO_REQ_ACTON_STOP;
			break;
		default:
		{
			ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, rtc_post_request_invalid, P_ERROR, "unsupport req(%d)",reqType);
		}
		break;
	}
	if(!action)
		return;
	create_start_voice_chat_data(reqType,&body);	
	memset(postUrl,0x00,128*sizeof(char));
	snprintf(postUrl,128,"http://%s/?Action=%s&Version=%s",DOUBAO_DEFAULT_RTC_HOST,action,DOUBAO_DEFAULT_RTC_VERSION);
	memset(rtcPostHeader,0x00,1500);
	bot_generate_request_info(DOUBAO_DEFAULT_RTC_HOST,HTTP_POST,action,body,ACCESS_KEY_ID,SECRET_ACCESS_KEY,rtcPostHeader);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, rtc_post_request_3, P_DEBUG, "%s",rtcPostHeader);					
	openHttpcPost(postUrl,body,strlen(body),aiHttpRteCb);
	if(body)
		free(body);
	body = NULL;
	return;
}

static void doubao_audio_proc_start(void)
{
	openPlayerConfigT playParam = {0};
	openRecorderConfigT recordParam = {0};
	
	if(gAudStreamRingBuf == NULL)
	{
		gAudStreamRingBuf = pxEcRingCreate(AI_AUDIO_STREAM_BUFF_SIZE);
		EC_ASSERT(gAudStreamRingBuf,gAudStreamRingBuf,0,0);	
	}
	else
	{
		if(xEcRingGetOption(gAudStreamRingBuf, E_LRO_BUFF_SIZE) != AI_AUDIO_STREAM_BUFF_SIZE)
		{
			xEcRingDelete(gAudStreamRingBuf);
			gAudStreamRingBuf = pxEcRingCreate(AI_AUDIO_STREAM_BUFF_SIZE);
			EC_ASSERT(gAudStreamRingBuf,gAudStreamRingBuf,0,0);	
		}
	}
	
	xEcRingClear(gAudStreamRingBuf);
	//gAudStreamRingBuf->idataCnt = gAudStreamRingBuf->iPosWrite = AI_AUDIO_STREAM_BUFF_SIZE / 2;
	memset(&recordParam,0x00,sizeof(openRecorderConfigT));
	recordParam.recordParam.codec = AUDIO_RECORD_CODEC_PCM; 
	gOpenRecordHandler = openRecorderCreate(&recordParam);
	openRecorderGetDataCallback(gOpenRecordHandler,(openRecorderGetDataCallbackT)aiRecordDataCallback);
	openRecorderStart(gOpenRecordHandler,NULL);	

	memset(&playParam,0x00,sizeof(openPlayerConfigT));
	playParam.playParam.store = AUDIO_PLAY_STREAM;
	playParam.playParam.codec = AUDIO_PLAY_CODEC_TYPE_G711;
	playParam.playParam.sampleRate = SAMPLERATE_8K;
	gOpenPlayHandler = openPlayCreate(&playParam);
	openPlay(gOpenPlayHandler,NULL);
}


static void doubao_audio_player_fill(uint8_t *data,uint32_t size)
{
	uint32_t freeSize = 0;
	uint32_t __attribute__((unused)) fillSize = 0;
	freeSize = xEcRingGetOption(gAudStreamRingBuf, E_LRO_FREE_SIZE);
	//EC_ASSERT(freeSize > size,freeSize,size,0);
	if(gAudStreamRingBuf && freeSize)
	{
		fillSize = xEcRingWriteEx(gAudStreamRingBuf,data,size);	
	}
#ifdef DOUBAO_RTC_AUDIO_DUMP
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,doubao_audio_player_fill,P_DEBUG,"freeSize(%d),tryFillSize(%d),fillSize(%d)",freeSize,size,fillSize);
#endif
}

void doubao_audio_proc_end(void)
{
	openPlayStop(gOpenPlayHandler);
	openRecorderStop(gOpenRecordHandler);
	if(gAudStreamRingBuf)
	{
		xEcRingDelete(gAudStreamRingBuf);
		gAudStreamRingBuf = NULL;
	}
}


static void init_default_byte_rtc_data(void){
	g_byte_rtc_data.channel_joined = false;
	g_byte_rtc_data.voiceChartStarted = false;
	memcpy(g_byte_rtc_data.app_id,DOUBAO_TEST_APP_ID,strlen(DOUBAO_TEST_APP_ID));
	memcpy(g_byte_rtc_data.room_id,DOUBAO_TEST_ROOM_ID,strlen(DOUBAO_TEST_ROOM_ID));
	memcpy(g_byte_rtc_data.user_id,DOUBAO_TEST_USER_ID,strlen(DOUBAO_TEST_USER_ID));
	memcpy(g_byte_rtc_data.token,DOUBAO_TEST_TEMP_TOKEN,strlen(DOUBAO_TEST_TEMP_TOKEN));
	
	g_byte_rtc_data.is_send_video =   false;
	g_byte_rtc_data.is_send_audio =   false;
	g_byte_rtc_data.send_video_thread_id = NULL;
	g_byte_rtc_data.send_audio_thread_id = NULL;

	g_byte_rtc_data.audio_codec = AUDIO_CODEC_TYPE_G711A;

	g_byte_rtc_data.fps = DEFAULT_FPS_VALUE;
	g_byte_rtc_data.engine	=	NULL;
	g_byte_rtc_data.fini_notifyed = false;
}

static void aiRecordDataCallback(uint8_t *data,uint32_t size)
{
	uint32_t __attribute__((unused)) ret =0;
	audio_frame_info_t audInfo = {.data_type = AUDIO_DATA_TYPE_PCMA};
	uint32_t encodeSize = size / 2;

	uint8_t *dstPtr = (uint8_t *)malloc(encodeSize*(sizeof(uint8_t)));
	EC_ASSERT(dstPtr,dstPtr,encodeSize,0);
	memset(dstPtr,0x00,encodeSize *sizeof(uint8_t));
	extern int g711_encode(unsigned char* pCodecBits, const char* pBuffer, int nBufferSize);
	encodeSize = g711_encode(dstPtr,(char *)data,size);
#ifdef DOUBAO_RTC_AUDIO_DUMP
	ECPLAT_DUMP(UNILOG_PLAT_DOUBAO_AI,GA711_DUMP,P_INFO,"",encodeSize,dstPtr);
#endif
#ifdef AUD_LOOP_TEST
	doubao_audio_player_fill(dstPtr,encodeSize);
#else
	if(g_byte_rtc_data.channel_joined == true)
		ret = byte_rtc_send_audio_data(g_byte_rtc_data.engine,g_byte_rtc_data.room_id,dstPtr,encodeSize,&audInfo);
#endif
#ifdef DOUBAO_RTC_AUDIO_DUMP
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,aiRecordDataCallback,P_INFO,"encodeSize(%d),send ret(%d)",encodeSize,ret);
#endif
	free(dstPtr);
	dstPtr = NULL;
}


void DoubaoTestPadInit(void)
{
    APmuWakeupPadSettings_t wakeupPadSetting;
    wakeupPadSetting.negEdgeEn = true;
    wakeupPadSetting.posEdgeEn = false;
    wakeupPadSetting.pullDownEn = false;
    wakeupPadSetting.pullUpEn = true;

    slpManSetWakeupPadCfg(WAKEUP_PAD_3, true, &wakeupPadSetting); // stop Rtc
    NVIC_EnableIRQ(PadWakeup3_IRQn);

	slpManSetWakeupPadCfg(WAKEUP_PAD_4, true, &wakeupPadSetting); // stop Rtc
    NVIC_EnableIRQ(PadWakeup4_IRQn);
	
	slpManSetWakeupPadCfg(WAKEUP_PAD_5, true, &wakeupPadSetting);//start rtc
    NVIC_EnableIRQ(PadWakeup5_IRQn);

}
/*WAKEUP0,启动/打断智能体*/
void Pad0_WakeupIntHook(void)
{
	if(gDoubaoQueue)
	{
		QueueDoubaoEngT queue = {0};
		if(g_byte_rtc_data.voiceChartStarted || g_byte_rtc_data.channel_joined)
			queue.procFlag = AI_ENG_UPDATE_VOICE_CHART_FLAG;
		else
			queue.procFlag = AI_ENG_START_VOICE_CHART_FLAG;
		osMessageQueuePut(gDoubaoQueue,&queue, 0, 0);//ISR,do not set timeout
	}
}
/*boot,启动/打断智能体*/
void boot_ISR(void)
{
    uint16_t portIrqMask = GPIO_saveAndSetIrqMask(RTC_BOOT_GPIO_INSTANCE);
    if (GPIO_getInterruptFlags(RTC_BOOT_GPIO_INSTANCE) & (1 << RTC_BOOT_GPIO_INDEX))
    {
        if(gDoubaoQueue)
		{
			QueueDoubaoEngT queue = {0};
			if(g_byte_rtc_data.voiceChartStarted || g_byte_rtc_data.channel_joined)
			{
				
				muteFlag = true;
				xEcRingClear(gAudStreamRingBuf);
				extern void medDataHandleReset(void);
				medDataHandleReset();
				queue.procFlag = AI_ENG_UPDATE_VOICE_CHART_FLAG;
			}
			else
				queue.procFlag = AI_ENG_START_VOICE_CHART_FLAG;
			osMessageQueuePut(gDoubaoQueue,&queue, 0, 0);//ISR,do not set timeout
		}
        GPIO_clearInterruptFlags(RTC_BOOT_GPIO_INSTANCE, 1 << RTC_BOOT_GPIO_INDEX);
    }
    GPIO_restoreIrqMask(RTC_BOOT_GPIO_INSTANCE, portIrqMask);
	
}

#ifndef BSP_AIKIT
/*WAKEUP5,降低音量*/
void Pad5_WakeupIntHook(void)
{
#if 0
	if(gRtcFcQueue)
	{
		QueueRtcFcFlagT queue = {0};
		memset(&queue,0,sizeof(QueueRtcFcFlagT));
		queue.fcDesc = RTC_FC_SET_VOL;
		queue.paramDesc.paramType = RTC_FUNCTION_CALL_PARAM_NUM;
		queue.paramDesc.paramValue.numValue = RTC_FC_SET_VOL_LOW;
		osMessageQueuePut(gRtcFcQueue,&queue,0,0);
	}
#endif
	return;
}

/*WAKEUP3,增加音量*/
void Pad3_WakeupIntHook(void)
{
	if(gRtcFcQueue)
	{
		QueueRtcFcFlagT queue = {0};
		memset(&queue,0,sizeof(QueueRtcFcFlagT));
		queue.fcDesc = RTC_FC_SET_VOL;
		queue.paramDesc.paramType = RTC_FUNCTION_CALL_PARAM_NUM;
		queue.paramDesc.paramValue.numValue = RTC_FC_SET_VOL_UP;
		osMessageQueuePut(gRtcFcQueue,&queue,0,0);
	}
	return;
}
#else
/*WAKEUP3,降低音量*/
void Pad3_WakeupIntHook(void)
{

	if(gRtcFcQueue)
	{
		QueueRtcFcFlagT queue = {0};
		memset(&queue,0,sizeof(QueueRtcFcFlagT));
		queue.fcDesc = RTC_FC_SET_VOL;
		queue.paramDesc.paramType = RTC_FUNCTION_CALL_PARAM_NUM;
		queue.paramDesc.paramValue.numValue = RTC_LOCAL_SET_VOL_LOW;
		osMessageQueuePut(gRtcFcQueue,&queue,0,0);
	}
	return;
}

/*WAKEUP4,增加音量*/
void Pad4_WakeupIntHook(void)
{
	if(gRtcFcQueue)
	{
		QueueRtcFcFlagT queue = {0};
		memset(&queue,0,sizeof(QueueRtcFcFlagT));
		queue.fcDesc = RTC_FC_SET_VOL;
		queue.paramDesc.paramType = RTC_FUNCTION_CALL_PARAM_NUM;
		queue.paramDesc.paramValue.numValue = RTC_LOCAL_SET_VOL_UP;
		osMessageQueuePut(gRtcFcQueue,&queue,0,0);
	}
	return;
}

#endif
void pwrKey_wakeupHook(void)
{
	osStatus_t ret = 0;
	if(gDoubaoQueue)
	{
		QueueDoubaoEngT queue = {0};
		queue.procFlag = AI_ENG_STOP_VOICE_CHART_FLAG;
		ret = osMessageQueuePut(gDoubaoQueue,&queue, 0, 0);//ISR,do not set timeout
	}
}

#ifdef BSP_AIKIT
void pwrKey_doubleCkickHook(void)
{
	gInterruptMode = (gInterruptMode == 0 ? 1 :0);
	
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
		if(gSubtitleQueue)
		{
			QueueRtcSubtitleT queue = {0};
			memset(&queue,0,sizeof(QueueRtcSubtitleT));
			queue.showFlag = RTC_SHOW_MODE;
			queue.interruptMode = gInterruptMode;
			osMessageQueuePutToFront(gSubtitleQueue,&queue,0,0);
		}
#endif			
}
#endif

extern CcioDevice_t         gI2sDevice;
void doubao_fc_set_volume(char *funcId,char *chatId,void *param)
{
	uint8_t volume[] = {0, 45, 50, 55, 60, 65, 70, 75};
	CcioDevice_t    *chdev = &gI2sDevice;
    CcioDsaVolteHandleCodec_t handleCodecSpeaker;
	MWNvmCfgUsrSetCodecVolumn usrCodecVolumn;
	uint32_t volumeSet = *((uint32_t *)param);
	uint32_t setVal = 0;
	uint32_t currVal = 0;
	uint32_t deviceType = 0;
	static int8_t index = -1;
	if(volumeSet < 0 || volumeSet > 3)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,doubao_fc_set_volume,P_ERROR,"bad value[%d]",volumeSet);
	}
	//get current volume	
	mwNvmCfgGetUsrCodecVolumn(0, &usrCodecVolumn);
	currVal = usrCodecVolumn.rxDigUsrSet;
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,doubao_fc_set_volume_0,P_DEBUG,"current value[%d]",currVal);
	if(index < 0)
	{
		for(index = 0;index < sizeof(volume)/sizeof(uint8_t);index++)
		{
			if(currVal <= volume[index + 1] && currVal >= volume[index])
				break;			
		}
	}
	
	switch(volumeSet)
	{
		case RTC_FC_SET_VOL_MIN:
		{
			setVal = 0;
			index = 0;
		}
			break;
		case RTC_FC_SET_VOL_LOW:
		case RTC_LOCAL_SET_VOL_LOW:
		{
			setVal = index > 0 ? volume[--index] : 0;
		}
			break;
		case RTC_FC_SET_VOL_UP:
		case RTC_LOCAL_SET_VOL_UP:
		{
			setVal = index < (sizeof(volume)/sizeof(uint8_t) - 1) ? volume[++index] : volume[sizeof(volume)/sizeof(uint8_t) - 1];
		}
			break;
		case RTC_FC_SET_VOL_MAX:
			setVal = volume[sizeof(volume)/sizeof(uint8_t) - 1];
			index = sizeof(volume)/sizeof(uint8_t) - 1;
			break;
		default:
			break;
	}
	int audioRet = audioFastInit(); 
	if(audioRet)
	{
		handleCodecSpeaker.a.codecParam.deviceType = deviceType;
	    handleCodecSpeaker.a.codecParam.speakerVolVal = setVal;
	    handleCodecSpeaker.a.codecParam.volumeLevelHandle = CODEC_SPEAKER_VAL_SET;
	    handleCodecSpeaker.atHandle = NULL;
	    handleCodecSpeaker.chdev = chdev;	    
	    ccioSendMsgToRxTask(CCIO_CHAN_HANDLE_CODEC, &handleCodecSpeaker, (chanMsgBuildFunc)ccioBuildMsgHandleCodec);
	}
	//set volue to the nv,
	usrCodecVolumn.rxDigUsrSet &= ~0xff;
	usrCodecVolumn.rxDigUsrSet |= setVal;
	mwNvmCfgSetAndSaveUsrCodecVolumn(deviceType, usrCodecVolumn.rxDigUsrSet, usrCodecVolumn.rxAnaUsrSet);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,doubao_fc_set_volume_1,P_ERROR,"setVal[%d],index[%d]",setVal,index);
	
#if defined(FEATURE_SUBSYS_GUI_LVGL_ENABLE) && defined(BSP_AIKIT)
	if(gSubtitleQueue)
	{
		QueueRtcSubtitleT queue = {RTC_SHOW_VOLUME,NULL};
		queue.volumeIndex = index + 1;
		osMessageQueuePutToFront(gSubtitleQueue,&queue,0,0);
	}
#endif			
	if(volumeSet == RTC_LOCAL_SET_VOL_LOW|| volumeSet == RTC_LOCAL_SET_VOL_UP)
	{
		return;
	}
#ifndef COZE_RTC_FEATURE
	cJSON *fc_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(fc_obj, "ToolCallID", funcId);
	char content[256] = {0};
	memset(content,0,256);
	if(index >= (sizeof(volume)/sizeof(uint8_t) - 1))
		sprintf(content,"音量调到了最大");
	else if(index == 0)
		sprintf(content,"音量调到了最小");
	else
		sprintf(content,"音量调节完成");
	cJSON_AddStringToObject(fc_obj, "Content", content);
	char *json_string = cJSON_Print(fc_obj);

	char fc_message_buffer[256] = {'f', 'u', 'n', 'c'};
	int json_str_len = strlen(json_string);
	fc_message_buffer[4] = (json_str_len >> 24) & 0xff;
	fc_message_buffer[5] = (json_str_len >> 16) & 0xff;
	fc_message_buffer[6] = (json_str_len >> 8) & 0xff;
	fc_message_buffer[7] = (json_str_len >> 0) & 0xff;
	memcpy(fc_message_buffer + 8, json_string, json_str_len);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,doubao_fc_set_volume_2,P_DEBUG,"send message: %s", json_string);
	cJSON_Delete(fc_obj);
	byte_rtc_rts_send_message(g_byte_rtc_data.engine, g_byte_rtc_data.room_id, "volc01", fc_message_buffer, json_str_len + 8, 1, RTS_MESSAGE_RELIABLE);
#else
	cJSON *data = cJSON_CreateObject();
	cJSON_AddStringToObject(data,"chat_id",chatId);
	cJSON *tool_outputs = cJSON_CreateArray();
	cJSON_AddItemToObject(data,"tool_outputs",tool_outputs);

	cJSON *ot_item1 = cJSON_CreateObject();
	cJSON_AddStringToObject(ot_item1,"tool_call_id",funcId);
	cJSON *output = cJSON_CreateObject();
	cJSON_AddStringToObject(output,"content","success");
	char *outStr = cJSON_PrintUnformatted(output);
	cJSON_AddStringToObject(ot_item1,"output",outStr);
	cJSON_AddItemToArray(tool_outputs,ot_item1);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,doubao_fc_set_volume_3,P_DEBUG,"send data: %s", cJSON_Print(data));
	coze_conversation_chat_ul(COZE_UL_EVENT_TYPE_SUBMIT_TOOL_OUTPUTS,data);
#endif
	return;
}

void doubao_fc_set_lightness(char *funcId,char *chatId, void *param)
{
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
	const uint8_t backlightList[] = {1, 10, 30, 45, 60, 75, 100};
	uint32_t lightSet = *((uint32_t *)param);
	uint8_t lightValue = 0;
	static uint8_t index = 5;
	switch(lightSet)
	{
		case RTC_FC_SET_LIGHT_MIN:
			index = 0;
			break;
		case RTC_FC_SET_LIGHT_MAX:
			index = sizeof(backlightList)/sizeof(uint8_t) - 1;			
			break;
		case RTC_FC_SET_LIGHT_UP:
			index = index < (sizeof(backlightList)/sizeof(uint8_t) - 1) ? ++index : index;
			break;
		case RTC_FC_SET_LIGHT_LOW:
			index = index > 0 ? --index : 0;
			break;
		default:
			break;
	}
	lightValue = backlightList[index];
	lcdBackLight(lcdDev, lightValue);
#ifndef COZE_RTC_FEATURE
	cJSON *fc_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(fc_obj, "ToolCallID", funcId);
	char content[256] = {0};
	memset(content,0,256);
	if(index >= (sizeof(backlightList)/sizeof(uint8_t) - 1))
		sprintf(content,"屏幕已经调到了最亮");
	else if(index == 0)
		sprintf(content,"屏幕已经调到了最暗");
	else
		sprintf(content,"屏幕亮度已调节");

	cJSON_AddStringToObject(fc_obj, "Content", content);
	char *json_string = cJSON_Print(fc_obj);
	static char fc_message_buffer[256] = {'f', 'u', 'n', 'c'};
	int json_str_len = strlen(json_string);
	fc_message_buffer[4] = (json_str_len >> 24) & 0xff;
	fc_message_buffer[5] = (json_str_len >> 16) & 0xff;
	fc_message_buffer[6] = (json_str_len >> 8) & 0xff;
	fc_message_buffer[7] = (json_str_len >> 0) & 0xff;
	memcpy(fc_message_buffer + 8, json_string, json_str_len);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,doubao_fc_set_lightness,P_DEBUG,"send message: %s", json_string);
	cJSON_Delete(fc_obj);
	byte_rtc_rts_send_message(g_byte_rtc_data.engine, g_byte_rtc_data.room_id, "volc01", fc_message_buffer, json_str_len + 8, 1, RTS_MESSAGE_RELIABLE);
#else
	cJSON *data = cJSON_CreateObject();
	cJSON_AddStringToObject(data,"chat_id",chatId);
	cJSON *tool_outputs = cJSON_CreateArray();
	cJSON_AddItemToObject(data,"tool_outputs",tool_outputs);

	cJSON *ot_item1 = cJSON_CreateObject();
	cJSON_AddStringToObject(ot_item1,"tool_call_id",funcId);
	cJSON *output = cJSON_CreateObject();
	cJSON_AddStringToObject(output,"content","success");
	char *outStr = cJSON_PrintUnformatted(output);
	cJSON_AddStringToObject(ot_item1,"output",outStr);
	cJSON_AddItemToArray(tool_outputs,ot_item1);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,doubao_fc_set_lightness_1,P_DEBUG,"send data: %s", cJSON_Print(data));
	coze_conversation_chat_ul(COZE_UL_EVENT_TYPE_SUBMIT_TOOL_OUTPUTS,data);
#endif

	return;
#endif
}

static void threadFunctionCall(void *argument)
{
	QueueRtcFcFlagT queue = {0};
	osStatus_t ret = osOK;
	uint32_t fcIndex = 0;
	uint8_t paramType = 0;
	uint32_t paramLen = 0;
	int32_t paramValue = 0;
	gRtcFcQueue = osMessageQueueNew(QUEUE_SIZE_FC, sizeof(queue), NULL);
    if (gRtcFcQueue == NULL)
    {
    	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, threadFunctionCall_invalid_0, P_ERROR, "queue create error");
        goto labelEnd;
    }
	while(1)
	{
		ret = osMessageQueueGet(gRtcFcQueue,&queue,NULL,osWaitForever);
		ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, threadFunctionCall, P_ERROR, "%e<rtcLocalFcE>,funcId:%s",queue.fcDesc,queue.funcID);
		if(ret != osOK)
			break;
		paramType = queue.paramDesc.paramType;
		if(queue.fcDesc <= RTC_FC_SET_BRIGHTLESS &&  queue.fcDesc >= RTC_FC_SET_VOL)
		{
			if(paramType == RTC_FUNCTION_CALL_PARAM_NUM)
			{
				paramValue = queue.paramDesc.paramValue.numValue;
				gFcFuncList[queue.fcDesc](queue.funcID,queue.chatId, &paramValue);
			}
			else
			{
				//TODO
			}
		}		
		if(queue.funcID)
			free(queue.funcID);
		if(queue.chatId)
			free(queue.chatId);
		queue.funcID = NULL;
		queue.chatId = NULL;
	}
labelEnd:
	if(gRtcFcQueue)
	{
		osMessageQueueDelete(gRtcFcQueue);
		gRtcFcQueue = NULL;
	}
	osThreadExit();
}

void aiProcTimerFunc(void *param)
{
	if(gDoubaoQueue)
	{
		QueueDoubaoEngT queue = {0};
		queue.procFlag = AI_ENG_STOP_VOICE_CHART_FLAG;
		osMessageQueuePut(gDoubaoQueue,&queue, 0, 0);//ISR,do not set timeout
	}
}

static void threadDoubao(void *argument)
{
	int ret = 0;
	QueueDoubaoEngT queue = {0};
	
    gDoubaoQueue = osMessageQueueNew(QUEUE_SIZE_DOUBAO, sizeof(queue), NULL);
    if (gDoubaoQueue == NULL)
    {
    	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, threadDoubao_invalid_0, P_ERROR, "queue create error");
        goto labelEnd;
    }

	gAiNotiFlag = osEventFlagsNew(NULL);
	if (!gAiNotiFlag) {
       ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, threadDoubao_invalid_1, P_ERROR, "flag create error");
        goto labelEnd;
    }
	DoubaoTestPadInit();
	byte_rtc_event_handler_t eventHandle = {
        .on_global_error            =   byte_rtc_on_gloable_error,
        .on_join_room_success       =   byte_rtc_on_join_channel_success,
        .on_room_error              =   byte_rtc_on_channel_error,
        .on_user_joined             =   byte_rtc_on_user_joined,
        .on_user_offline            =   byte_rtc_on_user_offline,
        .on_user_mute_audio         =   byte_rtc_on_user_mute_audio,
        .on_user_mute_video         =   byte_rtc_on_user_mute_video,

        .on_audio_data              =   byte_rtc_on_audio_data,
        .on_video_data              =   byte_rtc_on_video_data,

        .on_key_frame_gen_req       =   byte_rtc_on_keyframe_gen_req,
        .on_target_bitrate_changed  =   byte_rtc_on_target_bitrate_changed,
        .on_token_privilege_will_expire =   byte_rtc_on_token_privilege_will_expire,
        .on_message_received			= byte_rtc_on_message_received,
        .on_fini_notify       			=   byte_rtc_on_fini_notify,
    };
	memset(&g_byte_rtc_data,0x00,sizeof(byte_rtc_data));
	init_default_byte_rtc_data();
	
	while(1)
	{
		memset(&queue, 0, sizeof(queue));
        if (osMessageQueueGet(gDoubaoQueue, &queue, 0, osWaitForever) == osOK)
    	{
    		ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, threadDoubao_0, P_ERROR, "proc :%e<aiEngPorcFlag>",queue.procFlag);
    		switch(queue.procFlag)
			{

				case AI_ENG_CREATE_ROOM_FLAG:
				case AI_ENG_START_VOICE_CHART_FLAG:
				{
					if(g_byte_rtc_data.voiceChartStarted || g_byte_rtc_data.channel_joined)
					{
						ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI, threadDoubao_invalid_2, P_ERROR, "channel_joined(%d)",g_byte_rtc_data.channel_joined);
					}
					else
					{	
						muteFlag = false;
						g_byte_rtc_data.voiceChartStarted = true;
						doubao_audio_proc_start();
#ifdef COZE_RTC_FEATURE 
						coze_create_room();
#else
						rtc_post_request(queue.procFlag);
#endif
					}
				}
				break;
				case AI_ENG_START_RTC_FLAG:
				{
#ifndef AUD_LOOP_TEST
					g_byte_rtc_data.engine = byte_rtc_create(g_byte_rtc_data.app_id,&eventHandle);
					byte_rtc_set_log_level(g_byte_rtc_data.engine,BYTE_RTC_LOG_LEVEL_FATAL);
					byte_rtc_set_params(g_byte_rtc_data.engine, "{\"debug\":{\"log_to_console\":1}}"); 
					byte_rtc_room_options_t option = { true,false,true,false};
					ret = byte_rtc_init(g_byte_rtc_data.engine);
					
					if(ret < 0)
					{
						ECPLAT_PRINTF(UNILOG_PLAT_AI, threadDoubao_invalid_3, P_ERROR, "byte_rtc_init ret(%d)",ret);
						//stop voice chart
					}
					else
					{
						ret = byte_rtc_set_audio_codec(g_byte_rtc_data.engine,g_byte_rtc_data.audio_codec);
						ECPLAT_PRINTF(UNILOG_PLAT_AI, threadDoubao_1, P_DEBUG, "byte_rtc_set_audio_codec ret(%d)",ret);
						ret = byte_rtc_join_room(g_byte_rtc_data.engine,g_byte_rtc_data.room_id,g_byte_rtc_data.user_id,g_byte_rtc_data.token,&option);
						ECPLAT_PRINTF(UNILOG_PLAT_AI, threadDoubao_2, P_DEBUG, "byte_rtc_join_room ret(%d)",ret);						
					}
#endif
				}
					break;
				case AI_ENG_UPDATE_VOICE_CHART_FLAG:
				{
					if(!g_byte_rtc_data.voiceChartStarted || !g_byte_rtc_data.channel_joined)
					{
						break;
					}
					
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
					if(gSubtitleQueue)
						osMessageQueueReset(gSubtitleQueue);
#endif
#ifdef COZE_RTC_FEATURE
					cJSON *data = cJSON_CreateObject();
					coze_conversation_chat_ul(COZE_UL_EVENT_TYPE_CANCEL,data);
#else
					rtc_post_request(queue.procFlag);
#endif
				}
					break;
				case AI_ENG_STOP_VOICE_CHART_FLAG:
				{
					if(!g_byte_rtc_data.voiceChartStarted)
					{
						break;
					}
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
					if(gSubtitleQueue)
						osMessageQueueReset(gSubtitleQueue);
#endif					
					g_byte_rtc_data.voiceChartStarted = false;
					doubao_audio_proc_end();
#ifdef AUD_LOOP_TEST
					rtc_post_request(queue.procFlag);
#else
					if(g_byte_rtc_data.channel_joined)
					{					
						byte_rtc_leave_room(g_byte_rtc_data.engine, g_byte_rtc_data.room_id);
					    osDelay(20);
						g_byte_rtc_data.channel_joined = false;
					    byte_rtc_fini(g_byte_rtc_data.engine);
						if(gAiNotiFlag)
						{	
							ret = osEventFlagsWait(gAiNotiFlag,0x01,osFlagsWaitAny, 1000*1000);//timeout 1000ms	
							EC_ASSERT((ret & 0x01) != 0 && g_byte_rtc_data.fini_notifyed, ret,g_byte_rtc_data.fini_notifyed,0);
							osEventFlagsClear(gAiNotiFlag, 0x01);
						}
						else							
					    	osDelay(20);
				    	byte_rtc_destroy(g_byte_rtc_data.engine);
						g_byte_rtc_data.fini_notifyed = false;
 
				    	//关闭智能体，如果不主动调用， 智能体会在远端离开后3分钟离开	
#ifndef COZE_RTC_FEATURE		    	
					rtc_post_request(queue.procFlag);
#endif/*COZE_RTC_FEATURE*/
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
					if(gSubtitleQueue)
					{
						QueueRtcSubtitleT queue = {RTC_SHOW_RTC_STOP,NULL};
						queue.subtitle = (char *)malloc(32*sizeof(char));
						EC_ASSERT(queue.subtitle,queue.subtitle,0,0);
						memset(queue.subtitle,0,32*sizeof(char));
						strcpy(queue.subtitle,"RTC已关闭");

						osMessageQueuePut(gSubtitleQueue,&queue,0,0);
					}
#endif
						}
#endif
				}
					break;
				default:
					break;
			}					

    	}
	}
labelEnd:
	if(gDoubaoQueue)
		osMessageQueueDelete(gDoubaoQueue);
	gDoubaoQueue = NULL;

	if(gAiNotiFlag)
		osEventFlagsDelete(gAiNotiFlag);
	gAiNotiFlag = NULL;
	osThreadExit();

}

INT32 rtcNetWorkStatusCallback(PsEventID eventID, void *param, UINT32 paramLen)
{
    NmAtiNetInfoInd *netif = NULL;
	ECPLAT_PRINTF(UNILOG_PLAT_AI, rtcNetWorkStatusCallback, P_DEBUG, "eventID(%x)",eventID);
    switch(eventID)
    {
        case PS_URC_ID_PS_NETINFO:
        {
            netif = (NmAtiNetInfoInd *)param;
            if (netif->netifInfo.netStatus == NM_NETIF_ACTIVATED)
            {
#ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
            	if(gSubtitleQueue)
        		{
            		QueueRtcSubtitleT queue = {RTC_SHOW_NETWORK,NULL};
					queue.subtitle = (char *)malloc(32*sizeof(char));
					EC_ASSERT(queue.subtitle,queue.subtitle,0,0);
					memset(queue.subtitle,0,32*sizeof(char));
					strcpy(queue.subtitle,"网络已连接");

					osMessageQueuePut(gSubtitleQueue,&queue,0,0);
        		}
#endif			
            }
            break;
        }

        default:
            break;
    }
    return 0;
}
#ifdef BSP_AIKIT
void rtc_bootIoInit(void)
{

    PadConfig_t padConfig;
    PAD_getDefaultConfig(&padConfig);

    padConfig.mux = RTC_BOOT_PAD_ALT_FUNC;
    PAD_setPinConfig(RTC_BOOT_PAD_INDEX, &padConfig);
	PAD_setPinPullConfig(RTC_BOOT_PAD_INDEX, PAD_INTERNAL_PULL_DOWN);

    GpioPinConfig_t config;
    config.pinDirection = GPIO_DIRECTION_INPUT;
    config.misc.interruptConfig = GPIO_INTERRUPT_RISING_EDGE; 

    GPIO_pinConfig(RTC_BOOT_GPIO_INSTANCE, RTC_BOOT_GPIO_INDEX, &config);
	XIC_SetVector(PXIC1_GPIO_IRQn, boot_ISR);
    XIC_EnableIRQ(PXIC1_GPIO_IRQn);
}
#endif

/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/
int32_t doubao_rtc_proc(uint8_t procFlag)
{
	if(gDoubaoQueue)
	{
		QueueDoubaoEngT queue = {0};
		queue.procFlag = procFlag;
		osMessageQueuePut(gDoubaoQueue,&queue, 0, 3);
	}
	return 0;
}


int32_t doubao_task_init(void)
{
	osThreadAttr_t threadAttr = {0};

	memset(&threadAttr, 0, sizeof(threadAttr));
	threadAttr.name 	  = "threadDoubao";
	threadAttr.stack_size = THREAD_STACK_SIZE_DOUBAO;
	threadAttr.priority   = osPriorityBelowNormal7;
#ifdef THREAD_STATIC
	threadAttr.stack_mem  = gDoubaoThreadStackMem;
	threadAttr.cb_mem	  = &gDoubaoThreadCbMem;
	threadAttr.cb_size	  = sizeof(StaticTask_t);
#endif
	if (osThreadNew(threadDoubao, NULL, &threadAttr) == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, doubao_task_init, P_ERROR, "Failed to create thread for doubao ");
	}

	memset(&threadAttr, 0, sizeof(threadAttr));
	threadAttr.name 	  = "threadFunctionCall";
	threadAttr.stack_size = THREAD_STACK_SIZE_FUNCTION_CALL;
	threadAttr.priority   = osPriorityBelowNormal7;
#ifdef THREAD_STATIC
		threadAttr.stack_mem  = grtcFcThreadStackMem;
		threadAttr.cb_mem	  = &grtcFcThreadCbMem;
		threadAttr.cb_size	  = sizeof(StaticTask_t);
#endif

	if (osThreadNew(threadFunctionCall, NULL, &threadAttr) == NULL)
	{
		ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, doubao_task_init_1, P_ERROR, "Failed to create thread for function call ");
	}
	registerPSEventCallback(PS_GROUP_PS_MASK, rtcNetWorkStatusCallback);
#ifdef BSP_AIKIT
	rtc_bootIoInit();
#endif
	return 0;
}
	
