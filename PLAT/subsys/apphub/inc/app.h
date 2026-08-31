/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    app.h
 * Description:  EC718 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_APP_H
#define  SUBSYS_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "subsys.h"

#define APP_MSG_QUEUE_SIZE	32

typedef enum
{
    APP_TP_MSG    		= 10,
    APP_KEY_MSG    		= 20,
	APP_STAT_MSG		= 30,
	APP_USER_MSG		= 40,
	APP_SYSTEM_MSG		= 50,
} AppMsgTypeT;

typedef struct __App_Point_t
{
	int32_t appActived;
	int32_t appStatus;
	int32_t initStatus;
} AppPointT;

typedef struct __App_Msg_t
{
	uint32_t* wndh;
	AppMsgTypeT msgType;
	int32_t param1;
	int32_t param2;
	uint32_t *param3;
	int32_t time;
	AppPointT pt;
} AppMsgT;

typedef struct
{
    AppMsgT msgs[APP_MSG_QUEUE_SIZE];
    uint32_t head;
    uint32_t tail;
} AppMsgQueueT;

typedef struct __App_Info_t
{
	int32_t id;
	int32_t appActived;
	int32_t appStatus;
	int32_t initStatus;
} AppInfoT;

typedef struct __App_t
{
	AppInfoT* info;
	uint32_t* wndh;
    int32_t (*init)(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
    int32_t (*preDraw)();
    int32_t (*msgProc)(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
    int32_t (*afterDraw)();
	int32_t (*destory)();
} AppT;

//typedef void (*AppCallback_t)(void *ctx);

int32_t appGetMsg(AppMsgT* msgPtr);
int32_t appSendMsg(AppMsgT* msgPtr);
int32_t mountApp(AppT* appPtr,int32_t slot);
int32_t setActiveApp(int32_t slot);
int32_t normalAppMsgProc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t poweroffAppMsgProc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);

extern AppT mainApp;

#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_APP_H */