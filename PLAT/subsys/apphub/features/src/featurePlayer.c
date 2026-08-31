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
#include "audio.h"
#endif

#include "cJSON.h"
#include "phoneMenu.h"
#include "featurePlayer.h"

#ifdef FEATURE_SUBSYS_OPENPLAYER_API_ENABLE
static openPlayer gOpenPlayHandler = NULL;
#endif


int32_t submenu_Proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
/**
  \fn
  \brief
  \return
*/
static char player_btn[3][8] = {"列表","","返回"};
static fpui_player_t player_data;
void showPlayer(void)
{
    SYSLOG_DEBUG("showPlayer\r\n");
    player_data.title = "测试.mp3";
    player_data.image = LV_IMAGE_40;
    player_data.bottomBtn = player_btn;
    ui_player_set(&player_data);
}

void run_player_play(char* name)
{
	openPlayerConfigT opParam = {0};
	opParam.playParam.store = AUDIO_PLAY_FILE;
	gOpenPlayHandler = openPlayCreate(&opParam);
	openPlay(gOpenPlayHandler,(void *)name);    
}

void run_player_stop()
{
    if(gOpenPlayHandler !=NULL)
	    openPlayPause(gOpenPlayHandler,true);  
}
/**
  \fn
  \brief
  \return
*/
int32_t player_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("player_init\r\n");
    showPlayer();
    return 0;
}

/**
  \fn
  \brief
  \return
*/
int32_t player_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("player_proc\r\n");
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
                    run_player_play("D:/s1imy05_imy.mp3");
                    break;
                case FUNC_CODE_MENU:
                    break;
                case FUNC_CODE_DIR_L:
                case FUNC_CODE_DIR_R:
                    break;
                case FUNC_CODE_BACK:
                    mainApp.msgProc = submenu_Proc;
                    show_list();
                    run_player_stop();
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
int32_t add_player_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
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
int32_t add_player_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("add_player_proc \r\n");

    return 0;
}
/**
  \fn
  \brief
  \return
*/
int32_t del_player_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
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
int32_t del_player_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("del_player_proc \r\n");

    return 0;
}
