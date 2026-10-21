#ifndef _CTW_HTTP_H_
#define _CTW_HTTP_H_

#include <stdlib.h>
#include <string.h>
#include "HTTPClient.h"
#include "ctw_common.h"
#include "cJSON.h"

typedef enum
{
    CTW_HTTP_ENABLE_FLAG_ENABLE = 0x1,//bit 0
    CTW_HTTP_ENABLE_FLAG_TLS = 0x2,//bit 1
    CTW_HTTP_ENABLE_FLAG_PARAMS = 0x4,//bit 2
    CTW_HTTP_ENABLE_FLAG_TOKEN = 0x8,//bit 3
    CTW_HTTP_ENABLE_FLAG_DEREG = 0x10,//bit 4
    CTW_HTTP_ENABLE_FLAG_REGSTAT = 0x20,//bit 5
} ctwHttpEnableFlag_t;

    
typedef struct{
    char host[48];
    uint32_t timestamp;
    char signature[65];
    char iccid[CTW_CCID_LEN+1];
    char imsi[CTW_IMSI_LEN+1];
    char imei[CTW_IMEI_LEN+1];
    INT16 rsrp;
    INT8  snr;
    INT8  txPower;
    uint16_t phyCellId;
}ctwHttpRegParam_t;

void ctwHttpGetInvariPara(ctwHttpRegParam_t* pCtwHttpRegParam, MWNvmCfgCtwHttpParam* pCtwHttpParam);
void ctwHttpGetVariPara(ctwHttpRegParam_t* pCtwHttpRegParam, MWNvmCfgCtwHttpParam* pCtwHttpParam);
uint8_t ctwHttpAuthReg(HttpClientContext* pHttpClient, MWNvmCfgCtwHttpParam* pCtwHttpParam, ctwHttpRegParam_t* pRegParam);
uint8_t ctwHttpSendData(HttpClientContext* pHttpClient, MWNvmCfgCtwHttpParam* pCtwHttpParam, ctwHttpRegParam_t* pRegParam, char* topic, char* data);

#endif//_CTW_HTTP_H_

