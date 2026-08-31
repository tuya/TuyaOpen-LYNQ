#ifndef __DOUBAO_BOT_CONFIG_H__
#define __DOUBAO_BOT_CONFIG_H__

#define DOUBAO_DEFAULT_RTC_HOST 		"rtc.volcengineapi.com"
#define DOUBAO_DEFAULT_RTC_VERSION		"2024-12-01"

#define DOUBAO_REQ_ACTON_START			"StartVoiceChat"
#define DOUBAO_REQ_ACTON_STOP			"StopVoiceChat"
#define DOUBAO_REQ_ACTON_UPDATE			"UpdateVoiceChat"

#define ACCESS_KEY_ID					"***********"
#define SECRET_ACCESS_KEY				"***********"


#define ACCESS_APPID					"***********"
#define ACCESS_TOKEN					"***********"
#define DOUBAO_BOT_ID					"bot-***********"

/*
*参考：https://www.volcengine.com/docs/6348/1438400
*app_id:rtc应用ID，使用G771A codec，需要进行加白；
*room_id由应用客户端或者应用服务器提供，使用G711A编码，房间号规则为'G711A'+UUID
*user_id 由应用客户端（嵌入式硬件）提供；
*token有token服务器（应用服务器）依据一定的规则，并且与room id及user id绑定；
*应用客户端加入房间时需要携带token，此处使用火山引擎平台上生成的临时token，方便调试
*/
#define DOUBAO_TEST_APP_ID					"***********"
#define DOUBAO_TEST_ROOM_ID					"G711A******" 
#define DOUBAO_TEST_USER_ID					"user******"
#define DOUBAO_TEST_TEMP_TOKEN				"***********"

#define COZE_AUTH_TOKEN						"pat_***********"
#define COZE_BOT_ID							"***********"
#define COZE_AUDIO_TYPE						"G711A"
#define COZE_USER_ID						DOUBAO_TEST_USER_ID

int bot_generate_request_info(char *http_host,uint8_t mthod,char * action,char* req_body,char *AK,char *SK, char *header);
int hmac_sha256(const char *key, int key_len, const char *data, int data_len, char *output);

#endif
