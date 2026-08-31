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

#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
#include "openplayer.h"
#endif

#include "cJSON.h"
#include "phoneMenu.h"
#include "featureFile.h"


extern void *ui_file_set(fpui_file_t *data);
int32_t submenu_Proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
/**
  \fn
  \brief
  \return
*/
static uint8_t file_index = 0;
static uint8_t file_select = 0;
static char file_btn[3][8] = {"打开","","返回"};
static fpui_file_t file_data = {
    .object = NULL,
    .folder = "D:/",
    .title = "文件管理",
    .index = 0,
    .select = 1,
    .color = 0x001A,
    .bottomBtn = file_btn,
};
void showFile(void)
{
    SYSLOG_DEBUG("showFile\r\n");
    file_data.folder = "D:/";
    file_data.index = file_index;
    file_data.select = file_select;
    ui_file_set(&file_data);
}
/**
  \fn
  \brief
  \return
*/
int32_t file_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("file_init\r\n");
    showFile();
    return 0;
}

/**
  \fn
  \brief
  \return
*/
int32_t file_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("file_proc\r\n");
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
                    break;
                case FUNC_CODE_MENU:
                    break;
                case FUNC_CODE_DIR_L:
                case FUNC_CODE_DIR_R:
                    break;
                case FUNC_CODE_BACK:
                    mainApp.msgProc = submenu_Proc;
                    file_index = 0;
                    file_select = 0;
                    show_list();
                    break;
                case FUNC_CODE_DIR_U:
                    if(file_index) file_index--;
                    else if(file_select) file_select--;
                    showFile();
                    break;
                case FUNC_CODE_DIR_D:
                    if(file_select<5) file_select++;
                    else file_index++;
                    showFile();
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
int32_t add_file_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
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
int32_t add_file_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("add_file_proc \r\n");

    return 0;
}
/**
  \fn
  \brief
  \return
*/
int32_t del_file_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
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
int32_t del_file_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("del_file_proc \r\n");

    return 0;
}
