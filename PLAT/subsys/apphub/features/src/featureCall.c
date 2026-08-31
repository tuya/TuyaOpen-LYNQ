#include <string.h>
#include "FreeRTOS.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "charge.h"
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
#include "bsp_custom.h"
#include DEBUG_LOG_HEADER_FILE
#include "plat_config.h"
#ifdef FEATURE_SUBSYS_APPHUB_ENABLE
#include "app.h"
#include "apphub.h"
#endif
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "jsonDb.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include "systime.h"
#endif
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
#include "teleservice.h"
#endif
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "ui.h"
#include "merged.h"
#endif
#include "featureCall.h"
#include "phoneCallList.h"
#include "hal_screen.h"


extern int cur_menu_func_id;
extern sub_func sub_func_table[];
static char call_btn[3][8] = {"选项","挂断","删除"};
extern AppT mainApp;

extern int32_t normalAppMsgProc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);

PhoneUI_call_t call_data = {0};
int call_num_index=0;

int call_mode = 0;

static uint32_t gCallBeginTime = 0;
static uint8_t  gCurrentSim    = 0;
static uint8_t  gCallState     = 0;


void show_call_incoming(char *num)
{
    memset(call_btn, 0, sizeof(call_btn));
    strcpy(call_btn[1], "挂断");
    strcpy(call_data.numstr,num);
    call_data.bottomBtn = call_btn;
    call_data.type = UI_CALL_RING;
    ui_call_set(&call_data);
}

void show_dial()
{
    memset(call_btn[0],0,sizeof(call_btn[0]));
    memset(call_btn[1],0,sizeof(call_btn[1]));
    memset(call_btn[2],0,sizeof(call_btn[2]));
    strcpy(call_btn[2],"挂断");
    
    if(call_mode == 0)
    {
        strcpy(call_btn[0],"免提");
    }
        
    if(call_mode == 1)
    {
        strcpy(call_btn[0],"听筒");
    }
    
    call_data.bottomBtn = call_btn;
    call_data.type = UI_CALL_DIAL;
    ui_call_set(&call_data);


}

void show_hangup()
{
    memset(call_btn[0],0,sizeof(call_btn[0]));
    memset(call_btn[1],0,sizeof(call_btn[1]));
    memset(call_btn[2],0,sizeof(call_btn[2]));
    call_data.bottomBtn = call_btn;
    call_data.type = UI_CALL_HANGUP;
    ui_call_set(&call_data);
}

void show_call_num(char num)
{
    char show_call_num[32] = {0};
    memset(call_btn[0],0,sizeof(call_btn[0]));
    memset(call_btn[1],0,sizeof(call_btn[1]));
    memset(call_btn[2],0,sizeof(call_btn[2]));
    strcpy(call_btn[0],"选项"); 
    strcpy(call_btn[2],"删除"); 
    call_data.bottomBtn = call_btn;
    call_data.type = UI_CALL_INPUT;
    if(call_num_index>17)
    {
        call_num_index = 17;
        for(int i=0;i<17;i++)
            show_call_num[i]=call_data.numstr[i+1];

        show_call_num[17]=num;

        for(int i=0;i<18;i++)
            call_data.numstr[i]=show_call_num[i];
    }else
    {
        call_data.numstr[call_num_index]=num;
        for(int i=0;i<32;i++)
            show_call_num[i]=call_data.numstr[i];
    }
    SYSLOG_DEBUG("num:%c\r\n",num);
    SYSLOG_DEBUG("call_num_index:%d\r\n",call_num_index);
    SYSLOG_DEBUG("call_data.numstr[call_num_index]:%S\r\n",&call_data.numstr[0]);
    SYSLOG_DEBUG("show_call_num:%s\r\n",&show_call_num[0]);
    ui_call_set(&call_data);

    cipherHandler(call_data.numstr);
}

fpui_bar_t vol_data = {0};

void show_vol(uint8_t volumeIndex)
{
    vol_data.range = CALL_VOLUME_INDEX_MAX;
    vol_data.value = volumeIndex;
    vol_data.offsetX = 115;
    vol_data.offsetY = 0;
    vol_data.width = 10;
    vol_data.height = 200;
    vol_data.color = 0xFFFF;
    ui_bar_set(&vol_data);
}

void run_call()
{
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
    phoneDialled(call_data.numstr);
#endif
}

void run_answer()
{
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
    phoneAnswer();
#endif
}

void run_hangup()
{
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
    phoneHangUp();
#endif
}

char *timeStampToString(uint32_t timeStamp)
{
    static char  buffer[] = "2000-01-01 00:00";
    struct tm   *tmTime   = time_localtime((const time_t *)&timeStamp);

    if (tmTime != NULL)
    {
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d",
                 tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                 tmTime->tm_hour,        tmTime->tm_min);

        return buffer;
    }

    return NULL;
}

char *callTypeToString(uint8_t type)
{
    switch (type)
    {
        case PHONE_CALL_TYPE_INCOMMING:             return "被叫";
        case PHONE_CALL_TYPE_DIALLING:              return "主叫";
        case PHONE_CALL_TYPE_INCOMMING_UNANSWERED:  return "被叫未接";
        case PHONE_CALL_TYPE_DIALLING_UNANSWERED:   return "主叫未通";
        default:                                    return NULL;
    }
}

void recordCallBeginTime(void)
{
    gCallBeginTime = time_time(NULL);
    SYSLOG_INFO("gCallBeginTime=%d\r\n", gCallBeginTime);
}

void setCallState(uint8_t state)
{
    gCallState = state;
    SYSLOG_INFO("gCallState=%d\r\n", gCallState);
}

uint8_t getCurrentSim(void)
{
    return gCurrentSim;
}

void add_call_list()
{
    PhoneCallListT callList = {0};

    memset(&callList, 0, sizeof(callList));
    memcpy(callList.number, call_data.numstr, strlen(call_data.numstr));
    callList.sim       = getCurrentSim();
    callList.timeStamp = gCallBeginTime;

    switch (gCallState)
    {
        case PHONE_CALL_STATE_RING:
            callList.type = PHONE_CALL_TYPE_INCOMMING_UNANSWERED;
            callList.time = 0;
            break;

        case PHONE_CALL_STATE_ATA:
            callList.type = PHONE_CALL_TYPE_INCOMMING;
            callList.time = time_time(NULL) - gCallBeginTime;
            break;

        case PHONE_CALL_STATE_ATD:
            callList.type = PHONE_CALL_TYPE_DIALLING_UNANSWERED;
            callList.time = 0;
            break;

        case PHONE_CALL_STATE_COLP:
            callList.type = PHONE_CALL_TYPE_DIALLING;
            callList.time = time_time(NULL) - gCallBeginTime;
            break;

        default:
            break;
    }

    SYSLOG_INFO("callList: number=%s, type=%d, sim=%d, time=%d, timeStamp=%d\r\n",
                callList.number, callList.type, callList.sim, callList.time, callList.timeStamp);

    addCallList(&callList);
}

int32_t callNumberInput_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    fpui_message_t fpuiMessage = {.height = 50, .color = WHITE};

    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_INFO("msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
            switch(msg->param1)
            {
                case FUNC_CODE_MENU:
                    break;
                case FUNC_CODE_YES:
                    memset(&call_data,0,sizeof(call_data));
                    call_num_index=0;
                    mainApp.msgProc = normalAppMsgProc;
                    show_home(); 
                    break;
                case FUNC_CODE_BACK:
                    call_num_index--;
                    if(call_num_index<0)
                    {
                        memset(&call_data,0,sizeof(call_data));
                        call_num_index=0;
                        mainApp.msgProc = normalAppMsgProc;
                        show_home();                         
                    }else
                    {
                        show_call_num(0x0);
                    }
                    break;
                case FUNC_CODE_DIAL:
                    if (imsIsReady() == true)
                    {
                        if (strlen(call_data.numstr) > 0)
                        {
                            show_dial();
                            run_call();
                            mainApp.msgProc = callInCall_proc;
                        }
                        else
                        {
                            fpuiMessage.width   = 160;
                            fpuiMessage.context = "号码为空";
                            ui_message_set(&fpuiMessage);
                            osDelay(700);
                            ui_message_set(NULL);
                        }
                    }
                    else if (simGetStatus(0) != SIM_READY)
                    {
                        fpuiMessage.width   = 150;
                        fpuiMessage.context = "无SIM卡";
                        ui_message_set(&fpuiMessage);
                        osDelay(700);
                        ui_message_set(NULL);
                        // show_dial();
                        // run_call();
                        // mainApp.msgProc = callInCall_proc;
                    }
                    else if (getCsqPercentage() == 0)
                    {
                        fpuiMessage.width   = 130;
                        fpuiMessage.context = "无信号";
                        ui_message_set(&fpuiMessage);
                        osDelay(700);
                        ui_message_set(NULL);
                    }
                    else
                    {
                        fpuiMessage.width   = 160;
                        fpuiMessage.context = "网络异常";
                        ui_message_set(&fpuiMessage);
                        osDelay(700);
                        ui_message_set(NULL);
                    }
                    break;
                case FUNC_CODE_NO:
                    memset(&call_data,0,sizeof(call_data));
                    call_num_index=0;
                    mainApp.msgProc = normalAppMsgProc;
                    show_home();  
                    break;
                case '1':
                    show_call_num('1');
                    call_num_index++;
                    break;
                case '2':
                    show_call_num('2');
                    call_num_index++;
                    break;
                case '3':
                    show_call_num('3');
                    call_num_index++;
                    break;
                case '4':
                    show_call_num('4');
                    call_num_index++;
                    break;
                case '5':
                    show_call_num('5');
                    call_num_index++;
                    break;
                case '6':
                    show_call_num('6');
                    call_num_index++;
                    break;
                case '7':
                    show_call_num('7');
                    call_num_index++;
                    break;
				case '8':
                    show_call_num('8');
                    call_num_index++;
					break;
				case '9':
                    show_call_num('9');
                    call_num_index++;
					break;
				case '0':
                    show_call_num('0');
                    call_num_index++;
					break;
				case '#':
                    show_call_num('#');
                    call_num_index++;
					break;
				case '*':
                    show_call_num('*');
                    call_num_index++;
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

int32_t callInCall_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    // appInfo->initStatus = 1;
    call_mode = 0; // 0: ear phone / 1 :Handsfree 

    return 0;
}

int32_t callInCall_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_INFO("msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
            switch(msg->param1)
            {
                case FUNC_CODE_MENU:
                    // if(call_mode==0)
                    //     call_mode = 1;
                    // else
                    //     call_mode = 0;
                    // show_dial();
                    break;
                case FUNC_CODE_BACK:
                case FUNC_CODE_NO:
                    show_hangup();
                    run_hangup();
                    osDelay(1000);
                    memset(&call_data,0,sizeof(call_data));
                    call_num_index=0;
                    mainApp.msgProc = normalAppMsgProc;
                    show_home();
                    break;
                case FUNC_CODE_DIR_U:
                    show_vol(adjustCallVolume(true));
                    osDelay(1000);
                    ui_bar_set(NULL);
                    break;
                case FUNC_CODE_DIR_D:
                    show_vol(adjustCallVolume(false));
                    osDelay(1000);
                    ui_bar_set(NULL);
                    break;
                case '1':
                    vtsSend("1");
                    SYSLOG_INFO("vtsSend:1\r\n");
                    break;
                case '2':
                    vtsSend("2");
                    SYSLOG_INFO("vtsSend:2\r\n");
                    break;
                case '3':
                    vtsSend("3");
                    SYSLOG_INFO("vtsSend:3\r\n");
                    break;
                case '4':
                    vtsSend("4");
                    SYSLOG_INFO("vtsSend:4\r\n");
                    break;
                case '5':
                    vtsSend("5");
                    SYSLOG_INFO("vtsSend:5\r\n");
                    break;
                case '6':
                    vtsSend("6");
                    SYSLOG_INFO("vtsSend:6\r\n");
                    break;
                case '7':
                    vtsSend("7");
                    SYSLOG_INFO("vtsSend:7\r\n");
                    break;
				case '8':
                    vtsSend("8");
                    SYSLOG_INFO("vtsSend:8\r\n");
					break;
				case '9':
                    vtsSend("9");
                    SYSLOG_INFO("vtsSend:9\r\n");
					break;
				case '0':
                    vtsSend("0");
                    SYSLOG_INFO("vtsSend:0\r\n");
					break;
				case '#':
                    vtsSend("#");
                    SYSLOG_INFO("vtsSend:#\r\n");
					break;
				case '*':
                    vtsSend("*");
                    SYSLOG_INFO("vtsSend:*\r\n");
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

int32_t callInComing_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_INFO("msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
            switch(msg->param1)
            {
                case FUNC_CODE_NO:
                    show_hangup();
                    run_hangup();
                    osDelay(1000);
                    memset(&call_data,0,sizeof(call_data));
                    call_num_index=0;
                    mainApp.msgProc = normalAppMsgProc;
                    show_home();
                    break;
                case FUNC_CODE_DIAL:
                    show_dial();
                    run_answer();
                    mainApp.msgProc = callInCall_proc;
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

int32_t callDialout_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_INFO("msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
            switch(msg->param1)
            {
                case FUNC_CODE_NO:
                    mainApp.msgProc = normalAppMsgProc;
                    show_home();
                    break;

                case FUNC_CODE_MENU:
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
