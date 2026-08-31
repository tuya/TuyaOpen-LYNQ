#include <string.h>
#include "FreeRTOS.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "charge.h"

#include "bsp_custom.h"
#include DEBUG_LOG_HEADER_FILE
#include "plat_config.h"

#ifdef FEATURE_DRIVER_KEYPAD_ENABLE
#include "keypad.h"
#include "kpc.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_MISC_ENABLE
#include "misc.h"
#endif
#ifdef FEATURE_SUBSYS_APPHUB_ENABLE
#include "app.h"
#include "apphub.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "ui.h"
#endif

#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
#include "openrecorder.h"
#endif

#include "cJSON.h"
#include "phoneMenu.h"
#include "featureRecorder.h"


extern fpui_recorder_t *ui_recorder_get(void);
extern void *ui_recorder_set(fpui_recorder_t *data);
int32_t submenu_Proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
/**
  \fn
  \brief
  \return
*/
static char recorder_btn[3][8] = {"文件","录音","返回"};
void showRecorder(void)
{
    SYSLOG_DEBUG("showRecorder\r\n");
    fpui_recorder_t *recorder_ptr = ui_recorder_get();
    memset(recorder_ptr->title,0,strlen(recorder_ptr->title));
    strcpy(recorder_ptr->title,"test.wav");
    memcpy(recorder_ptr->bottomBtn,recorder_btn,sizeof(recorder_btn));
    recorder_ptr->color = 0x0012;
    ui_recorder_set(recorder_ptr);
}

#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
static openRecorder gOpenRecordHandler = NULL;
#endif

void run_recorder_start(char* name)
{
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
    int time = 0;
	openRecorderConfigT param = {0};
	memset(&param,0x00,sizeof(openRecorderConfigT));
	param.recordParam.recordTime = time;
	gOpenRecordHandler = openRecorderCreate(&param);
	openRecorderStart(gOpenRecordHandler,name);
#endif
}

void run_recorder_stop()
{
#ifdef FEATURE_SUBSYS_OPENRECORDER_API_ENABLE
	openRecorderStop(gOpenRecordHandler);
#endif
}
/**
  \fn
  \brief
  \return
*/
int32_t recorder_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("recorder_init\r\n");
    showRecorder();
    return 0;
}

/**
  \fn
  \brief
  \return
*/
int32_t recorder_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("recorder_proc\r\n");
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_DEBUG("msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
            switch(msg->param1)
            {
                case FUNC_CODE_NO:
                    mainApp.msgProc = normalAppMsgProc;
                    show_home();
                    break;

                case FUNC_CODE_YES:
                    run_recorder_start("d:/test.amr");
                    break;
                case FUNC_CODE_MENU:
                    break;
                case FUNC_CODE_DIR_L:
                case FUNC_CODE_DIR_R:
                    break;
                case FUNC_CODE_BACK:
                    mainApp.msgProc = submenu_Proc;
                    show_list();
                    run_recorder_stop();
                    break;
                case FUNC_CODE_DIR_U:
                    SYSLOG_DEBUG("FUNC_CODE_DIR_U %d\r\n",msg->msgType);
                    break;
                case FUNC_CODE_DIR_D:
                    SYSLOG_DEBUG("FUNC_CODE_DIR_D %d\r\n",msg->msgType);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return 0;
}
/**
  \fn
  \brief
  \return
*/
int32_t add_recorder_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;
    return 0;
}
/**
  \fn
  \brief
  \return
*/
int32_t add_recorder_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("add_recorder_proc \r\n");

    return 0;
}
/**
  \fn
  \brief
  \return
*/
int32_t del_recorder_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;
    return 0;
}
/**
  \fn
  \brief
  \return
*/
int32_t del_recorder_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("del_recorder_proc \r\n");

    return 0;
}
