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

#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "ui.h"
#include "merged.h"
#endif
#include "featureCall.h"

extern int cur_menu_func_id;
extern sub_func sub_func_table[];
static char call_btn[3][8] = {"选项","挂断","删除"};
extern AppT mainApp;

extern int32_t normalAppMsgProc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);

PhoneUI_call_t call_data = {0};
int call_num_index=0;

int vol_num = 3;
int call_mode = 0;
int ear_vol_index[8] = {24,27,30,33,36,39,42,45};
int speaker_vol_index[8] = {45,50,55,60,65,70,75,80};

void show_dial()
{
    memset(call_btn[0],0,sizeof(call_btn[0]));
    memset(call_btn[1],0,sizeof(call_btn[1]));
    memset(call_btn[2],0,sizeof(call_btn[2]));
    strcpy(call_btn[2],"挂断");
    
    if(call_mode == 0)
    {
        strcpy(call_btn[0],"免提");
        vol_num = 3;
    }
        
    if(call_mode == 1)
    {
        strcpy(call_btn[0],"听筒");
        vol_num = 7;   
    }
    
    run_setvol(vol_num);
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

void show_vol()
{
    vol_data.range = 8;
    vol_data.value = vol_num;
    vol_data.offsetX = 90;
    vol_data.offsetY = 0;
    vol_data.width = 10;
    vol_data.height = 200;
    vol_data.color = 0xFFFF;
    // ui_volume_set(vol_num);
    ui_bar_set(&vol_data);
}

void run_setvol(int vol)
{
    if(call_mode == 0)
    {
        SYSLOG_INFO("vol:%d\r\n",vol);
        setVolume(ear_vol_index[vol]);
    }
        
    if(call_mode == 1)
    {
        SYSLOG_INFO("vol:%d\r\n",vol);
        setVolume(speaker_vol_index[vol]);
    }
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

void add_call_list()
{
    char time_str[7]={0}; 
    char date_str[12]={0};
    
    call_list_db call_list={0};
    time_t now = time_time(NULL);
    struct tm *timeinfo = time_localtime(&now);

    memset(time_str, 0, sizeof(time_str));
    memset(date_str, 0, sizeof(date_str));
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", timeinfo);
    strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);

    strcpy(call_list.number,call_data.numstr); 
    memcpy(call_list.time, date_str, strlen(date_str));
    memcpy(call_list.time+strlen(date_str), " ", strlen(" "));
    memcpy(call_list.time+strlen(date_str) + strlen(" "), time_str, strlen(time_str));

    call_list.flag = 0x01;
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbAddItem("D:/callListDb.json", "D:/callListDb.type", &call_list);
#endif
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
                    if(nwIsReady() == true)
                    {
                        if (strlen(call_data.numstr) > 0)
                        {
                            show_dial();
                            run_call();
                            add_call_list();
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
                    else if (simGetStatus() == SIM_REMOVED)
                    {
                        fpuiMessage.width   = 150;
                        fpuiMessage.context = "无SIM卡";
                        ui_message_set(&fpuiMessage);
                        osDelay(700);
                        ui_message_set(NULL);
                        // show_dial();
                        // run_call();
                        // add_call_list();
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
    vol_num = 3;
    run_setvol(vol_num);
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
                    vol_num = vol_num + 1;
                    if(vol_num >=8)
                        vol_num = 8;
                    show_vol();
                    run_setvol(vol_num);
                    osDelay(1000);
                    ui_bar_set(NULL);
                    break;
                case FUNC_CODE_DIR_D:
                    vol_num = vol_num - 1;
                    if(vol_num <=1)
                        vol_num = 1;
                    show_vol();
                    run_setvol(vol_num);
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
