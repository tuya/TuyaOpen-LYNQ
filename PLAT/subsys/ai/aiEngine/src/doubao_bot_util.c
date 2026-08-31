/**
  * @file doubao_bot_util.c
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
#include <time.h>
#include DEBUG_LOG_HEADER_FILE

#include "cmsis_os2.h"
#include "slpman.h"
#include "osasys.h"
#include "ps_lib_api.h"
#ifdef FEATURE_SUBSYS_AI_DOUBAO_ENABLE
#include "doubao_bot_util.h"
#endif
#include "cJSON.h"
#include "md.h"
#include "hmac_drbg.h"
#include "HTTPClient.h"
#include "doubao_bot_config.h"

int hmac_sha256(const char *key, int key_len, const char *data, int data_len, char *output) {
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    mbedtls_md_init(&ctx);

    if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1) != 0) {
        mbedtls_md_free(&ctx);
        return -1;
    }

    if (mbedtls_md_hmac_starts(&ctx, (unsigned char *)key, key_len) != 0) {
        mbedtls_md_free(&ctx);
        return -2;
    }

    if (mbedtls_md_hmac_update(&ctx, (unsigned char *)data, data_len) != 0) {
        mbedtls_md_free(&ctx);
        return -3;
    }

    if (mbedtls_md_hmac_finish(&ctx, (unsigned char *)output) != 0) {
        mbedtls_md_free(&ctx);
        return -4;
    }

    mbedtls_md_free(&ctx);

    return 0;
}

static int hash_sha256(const char *input, int input_len, char *output) {
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *md_info;

    mbedtls_md_init(&ctx);
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md_info == NULL) {
        mbedtls_md_free(&ctx);
        return -1;  // 获取算法信息失败
    }

    if (mbedtls_md_setup(&ctx, md_info, 0) != 0) {
        mbedtls_md_free(&ctx);
        return -2;  // 设置算法失败
    }

    // 开始哈希计算
    if (mbedtls_md_starts(&ctx) != 0) {
        mbedtls_md_free(&ctx);
        return -3;  // 开始计算失败
    }

    if (mbedtls_md_update(&ctx, (unsigned char *)input, input_len) != 0) {
        mbedtls_md_free(&ctx);
        return -4;  // 更新数据失败
    }

    // 完成哈希计算并获取结果
    if (mbedtls_md_finish(&ctx, (unsigned char *)output) != 0) {
        mbedtls_md_free(&ctx);
        return -5;  // 完成计算失败
    }
	
    mbedtls_md_free(&ctx);

    return 0;
}

void byteToHexChars(char byte, char *hexChars) {
    const char hexDigits[] = "0123456789abcdef";
    hexChars[0] = hexDigits[(byte >> 4) & 0x0F];
    hexChars[1] = hexDigits[byte & 0x0F];
}

void hexArrayToText(const char *hexArray, int arraySize, char *text) {
    for (int i = 0; i < arraySize; i++) {
        byteToHexChars(hexArray[i], &text[i * 2]);
    }
    text[arraySize * 2] = '\0';
}

void reverse(unsigned char *str) {
    int len = strlen((char *)str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int ulltoa(uint64_t num, unsigned char *str) {
    int i = 0;
    do {
        str[i++] = num % 10 + '0';
        num /= 10;
    } while (num != 0);
    str[i] = '\0';

    reverse(str);

    return i;
}


int bot_generate_request_info(char *http_host,uint8_t mthod,char * action,char* req_body,char *AK,char *SK, char *header)
{
	
	char *canonical_uri = "/";
	char *content_type = "application/json";
	char time_str[16+1] = {0};
	char content_sha256[32] = {0};
	char bodySha256[64+1] = {0};
	char x_content_sha256[64+1] = {0};
	char tmp_string[128] = {0};
	char *signed_headers = "host;x-content-sha256;x-date";
	char *region = "cn-beijing";
	char *service = "rtc";
	char *canonical_header = NULL;
	char *canonical_request = NULL;
	char *tmpPtr = NULL;
	char *strMethod = NULL;
	char *string_to_sign = NULL;
	char *Authorization = NULL;
	
	canonical_header = (char *)malloc(256*sizeof(char));
	EC_ASSERT(canonical_header,canonical_header,0,0);
	memset(canonical_header,0x00,256*sizeof(char));

	canonical_request = (char *)malloc(1024*sizeof(char));
	EC_ASSERT(canonical_request,canonical_request,0,0);
	memset(canonical_request,0x00,1024*sizeof(char));

	memset(tmp_string,0x00,128 *sizeof(char));

	sprintf(tmp_string,"Action=%s&Version=%s",action,DOUBAO_DEFAULT_RTC_VERSION);
	
	memset(content_sha256,0x00,32*sizeof(char));
	memset(x_content_sha256,0x00,65*sizeof(char));
	memset(bodySha256,0x00,65*sizeof(char));

	//步骤一：创建规范请求
	utc_timer_value_t timeUtc   = {0};
	appGetSystemTimeUtcSync(&timeUtc);
	memset(time_str,0x00,16*sizeof(char) +1 );
	sprintf(time_str, "%d%02d%02dT%02d%02d%02dZ", timeUtc.UTCtimer1>>16, (timeUtc.UTCtimer1&0xFF00)>>8, (timeUtc.UTCtimer1&0xFF), (timeUtc.UTCtimer2>>24), (timeUtc.UTCtimer2&0xFF0000)>>16, (timeUtc.UTCtimer2&0xFF00)>>8);

	hash_sha256(req_body,strlen(req_body),content_sha256);
	hexArrayToText(content_sha256,32,x_content_sha256);
	memcpy(bodySha256,x_content_sha256,64);
	tmpPtr = canonical_header;
		
	sprintf(tmpPtr,"host:%s\n",http_host);
	tmpPtr = canonical_header + strlen(canonical_header);

	strcat(tmpPtr,"x-content-sha256:");
	tmpPtr=canonical_header + strlen(canonical_header);
	
	memcpy(tmpPtr,x_content_sha256,64);
	tmpPtr = canonical_header + 64;
	
	strcat(tmpPtr,"\n");
	tmpPtr=canonical_header + strlen(canonical_header);
	sprintf(tmpPtr,"x-date:%s\n",time_str);

	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,bot_generate_request_info,P_INFO,"%s",canonical_header);
	
	tmpPtr = canonical_request;
	if(mthod == HTTP_POST)
	{
		strMethod = "POST";
	}
	else
	{
		strMethod = "GET";
	}
	
	sprintf(tmpPtr,"%s\n%s\n%s\n%s\n%s\n%s",strMethod,canonical_uri,tmp_string,canonical_header,signed_headers,x_content_sha256);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,bot_generate_request_info_1,P_INFO,"%s",canonical_request);

	//步骤二：创建待签字符串
	memset(tmp_string,0x00,sizeof(tmp_string)/sizeof(char));
	snprintf(tmp_string,9,"%s",time_str);
	strcat(tmp_string,"/cn-beijing/rtc/request");

	string_to_sign = (char *)malloc(512*sizeof(char));
	EC_ASSERT(string_to_sign,string_to_sign,0,0);
	memset(string_to_sign,0x00,512*sizeof(char));

	memset(content_sha256,0x00,32*sizeof(char));
	memset(x_content_sha256,0x00,65*sizeof(char));

	hash_sha256(canonical_request,strlen(canonical_request),content_sha256);
	hexArrayToText(content_sha256,32,x_content_sha256);
	
	sprintf(string_to_sign,"HMAC-SHA256\n%s\n%s\n%s",time_str,tmp_string,x_content_sha256);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,bot_generate_request_info_3,P_INFO,"string_to_sign:%s",string_to_sign);

	//计算签名秘钥
	uint32_t key_len = (strlen(SK) > 32 ? strlen(SK) : 32);
	char *kSecret = malloc(key_len + 1);
	EC_ASSERT(kSecret,kSecret,0,0);

	memset(kSecret,0,key_len + 1);
	memcpy(kSecret,SK,strlen(SK));
	memset(content_sha256,0x00,32*sizeof(char));
	memset(x_content_sha256,0x00,65*sizeof(char));
	hmac_sha256(kSecret,strlen(kSecret),time_str,8,content_sha256);
	memcpy(kSecret,content_sha256,32);

	memset(content_sha256,0x00,32*sizeof(char));
	hmac_sha256(kSecret,32,region,strlen(region),content_sha256);
	memcpy(kSecret,content_sha256,32);

	memset(content_sha256,0x00,32*sizeof(char));
	hmac_sha256(kSecret,32,service,strlen(service),content_sha256);
	memcpy(kSecret,content_sha256,32);

	memset(content_sha256,0x00,32*sizeof(char));
	hmac_sha256(kSecret,32,"request",7,content_sha256);
	memcpy(kSecret,content_sha256,32);

	
	memset(content_sha256,0x00,32*sizeof(char));
	hmac_sha256(kSecret,32,string_to_sign,strlen(string_to_sign),content_sha256);
	hexArrayToText(content_sha256,32,x_content_sha256);
	
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,bot_generate_request_info_4,P_INFO,"signature:%s",x_content_sha256);

	Authorization = malloc(512*sizeof(char));
	EC_ASSERT(Authorization,Authorization,0,0);
	memset(Authorization,0x00,512);

	sprintf(Authorization,"HMAC-SHA256 Credential=%s/%s, SignedHeaders=%s, Signature=%s", AK, tmp_string, signed_headers, x_content_sha256);
	ECPLAT_PRINTF(UNILOG_PLAT_DOUBAO_AI,bot_generate_request_info_9,P_INFO,"Authorization:%s",Authorization);

	if(header)
		sprintf(header,"Content-Type: %s\r\nX-Content-Sha256: %s\r\nX-Date: %s\r\nAuthorization: %s",content_type,bodySha256,time_str,Authorization);
	if(Authorization != NULL)
	{
		free(Authorization);
	}
	Authorization = NULL;

	if(kSecret != NULL)
	{
		free(kSecret);
	}
	kSecret = NULL;

	if(string_to_sign != NULL)
	{
		free(string_to_sign);
	}
	string_to_sign = NULL;

	if(Authorization != NULL)
	{
		free(Authorization);
	}
	Authorization = NULL;

	if(canonical_request != NULL)
	{
		free(canonical_request);
		canonical_request = NULL;
	}

	if(canonical_header != NULL)
	{
		free(canonical_header);
		canonical_header = NULL;
	}
	return 0;
}


