#ifndef _CTIOT_FOTA_H
#define _CTIOT_FOTA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define START_MORE 0
#define BLOCK_SIZE 512

typedef enum
{
	FOTA_STATE_IDIL,
	FOTA_STATE_DOWNLOADING,
	FOTA_STATE_DOWNLOADED,
	FOTA_STATE_UPDATING,
}ctiotFotaState;
typedef struct
{
	char* package;
	char* packageUri;
	int packageLength;
	int packageUriLength;
}firmwareWritePara;

typedef enum
{
	FOTA_RESULT_INIT,                //0 init
	FOTA_RESULT_SUCCESS,             //1 success
	FOTA_RESULT_NOFREE,              //2 no space
	FOTA_RESULT_OVERFLOW,            //3 downloading overflow
	FOTA_RESULT_DISCONNECT,          //4 downloading disconnect
	FOTA_RESULT_CHECKFAIL,           //5 validate fail
	FOTA_RESULT_NOSUPPORT,           //6 unsupport package
	FOTA_RESULT_URIINVALID,          //7 invalid uri
	FOTA_RESULT_UPDATEFAIL,          //8 update fail
	FOTA_RESULT_PROTOCOLFAIL        //9 unsupport protocol
}ctiotFotaResult;

typedef struct
{
	ctiotFotaState fotaState;
	ctiotFotaResult fotaResult;
}ctiotFotaManage;

typedef struct
{
    char protocol[6];
    char address[51];
    int port;
    char uri[201];
}CTIOT_URI;
int fota_start(char* url);

void ctiot_fota_state_changed(void);

void ct_check_upgrade_result(void);
uint8_t ct_get_upgrade_result(void);
void ct_system_reboot(void);
//void notify_fotastate_con(lwm2m_transaction_t* transacP, void * message);

#endif

