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

#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "ui.h"
#include "merged.h"
#endif

#include "featureCallList.h"

#include "cJSON.h"
#include "phoneMenu.h"

extern int cur_menu_func_id;
extern sub_func sub_func_table[];
int32_t menu_Proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
extern AppT mainApp;

extern char feature_lv0_menu_buf[10][MENU_STR_LEN_MAX+1];
extern char feature_lv1_menu_buf[10][MENU_STR_LEN_MAX+1];
extern char feature_lv2_menu_buf[10][MENU_STR_LEN_MAX+1];
extern MenuInfoT  feature_menuInfo;

fpui_callist_t callList_data = {0};

int callList_total_num = 0;
int callList_cur_item = 0;
int callList_cur_menu_item = 0;

int callList_item_mode = 0;

char list_call[2][3][40] = {0};

static char list_callbtn[3][8] = {
    "选项",
    "",
    "返回",
};

extern char cur_menu_name[];

void showCallList(void)
{
#if 1
    call_list_db* calllist_db_1=0;
    call_list_db* calllist_db_2=0;
    // memset(&list_call[0],0,sizeof(list_call));
    // memset(&list_call[1],0,sizeof(list_call));

    callList_data.title = cur_menu_name;
    callList_data.select = callList_cur_item;

    SYSLOG_DEBUG("cur_menu_func_id:%d\r\n",sub_func_table[cur_menu_func_id].index);
    SYSLOG_DEBUG("callList_cur_item:%d\r\n",callList_cur_item);

    if(callList_total_num>=2)
    {
        callList_data.total = 2;

        if(callList_cur_item>=1)
            callList_data.select = 1;
        else
            callList_data.select = 0;

        int index0 = callList_cur_item;
        int index1 = callList_cur_item+1;
    
        if(index1 >= callList_total_num || callList_data.select == 1)
        {
            index0 = callList_cur_item - 1;
            index1 = callList_cur_item;
        }

        get_db_item(sub_func_table[cur_menu_func_id].index,index0,&calllist_db_1);
        SYSLOG_DEBUG("calllist_db.number:%s\r\n",calllist_db_1->number);
        SYSLOG_DEBUG("calllist_db.time:%s\r\n",calllist_db_1->time);

        get_db_item(sub_func_table[cur_menu_func_id].index,index1,&calllist_db_2);
        SYSLOG_DEBUG("calllist_db.number:%s\r\n",calllist_db_2->number);
        SYSLOG_DEBUG("calllist_db.time:%s\r\n",calllist_db_2->time);

        memcpy(&list_call[0][0][0],calllist_db_1,sizeof(call_list_db));
        SYSLOG_DEBUG("list_call[0].number:%s\r\n",&list_call[0][0]);
        SYSLOG_DEBUG("list_call[0][1]:%s\r\n",&list_call[0][1]);
        SYSLOG_DEBUG("list_call[0][2]:%s\r\n",&list_call[0][2]);

        memcpy(&list_call[1][0][0],calllist_db_2,sizeof(call_list_db));
        SYSLOG_DEBUG("list_call[1].number:%s\r\n",&list_call[1][0]);


    }
    else
    {
        callList_data.total = callList_total_num;
        if(callList_total_num>0)
        {
            get_db_item(sub_func_table[cur_menu_func_id].index,callList_cur_item,&calllist_db_1);
            SYSLOG_DEBUG("calllist_db.number:%s\r\n",calllist_db_1->number);
            SYSLOG_DEBUG("calllist_db.time:%s\r\n",calllist_db_1->time);

            memcpy(&list_call[0][0][0],calllist_db_1,sizeof(call_list_db));
            SYSLOG_DEBUG("list_call[0].number:%s\r\n",&list_call[0][0]);
        }
    }
#endif

    callList_data.title = cur_menu_name;
    callList_data.context = list_call;
    callList_data.bottomBtn = list_callbtn;
    ui_callist_set(&callList_data);

#if 1
    if(callList_total_num>=2)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(calllist_db_1);
        vPortFree_Psram(calllist_db_2);
#else
        free(calllist_db_1);
        free(calllist_db_2);        
#endif
    }else if(callList_total_num>0)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(calllist_db_1);
#else
        free(calllist_db_1);
#endif
    }
#endif
}

MenuQueryT menuQuery =
{
    .level      = 1,
    .level0     = 3,
    .startIndex = 0,
    .count      = -1,
    .type       = "name"
};

void listCallList_uiloader()
{
    feature_menu_loader("C:/callList_cn.json");
    phoneMenuQuery("C:/callList_cn.json", &menuQuery, feature_lv1_menu_buf);
}

fpui_listpop_t listpop = {
    .object = NULL,
    .screen = NULL,  
    .select = 1,
    .total = 5,
    .context[0] = "测试项1",
    .context[1] = "测试项2",
    .context[2] = "测试项3",
    .context[3] = "测试项4",
    .context[4] = "测试项5",
    .overlay = true,       //是否叠加显示,true=不删除原弹窗
    .offsetX = 0,   //x偏移量 如果为0则居中 -100 ~ 100
    .offsetY = 30,   //y偏移量 如果为0则居中 -100 ~ 100
    .width = 240,     //宽度 40 ~ 240
    .height = 280,    //高度 40 ~ 320
    .color=0x0,
};

void showCallList_menu()
{
    int i=0;
    if(feature_menuInfo.level0Count>5)
    {
        if(callList_cur_menu_item>=5)
        {
            listpop.select = 4;
            int callList_cur_menu_start = (callList_cur_menu_item-4);
            int callList_cur_menu_end = (callList_cur_menu_start+4);
            if(callList_cur_menu_end>=feature_menuInfo.level0Count)
            {
                callList_cur_menu_end = feature_menuInfo.level0Count-1;
                callList_cur_menu_start = (callList_cur_menu_end-4);
            }
            for(int i=0;i<5;i++)
            {
                listpop.context[i]=&feature_lv0_menu_buf[callList_cur_menu_start+i][0];
                SYSLOG_DEBUG("app_menu_index:%d,app_menu_name:%s\r\n",i,&feature_lv0_menu_buf[i][0]);
            }  
        }else{
            listpop.select = callList_cur_menu_item;
            for(i=0;i<5;i++)
            {
                listpop.context[i] = &feature_lv0_menu_buf[i][0];
            }
        }
        listpop.total = 5;
    }
    else
    {
        for(i=0;i<feature_menuInfo.level0Count;i++)
        {
            listpop.context[i] = &feature_lv0_menu_buf[i][0];
        }
        listpop.select = callList_cur_menu_item;
        listpop.total = feature_menuInfo.level0Count;
    }
    SYSLOG_DEBUG("listpop.select:%d,listpop.total:%d\r\n",listpop.select,listpop.total);
    SYSLOG_DEBUG("listpop.offsetX:%d,listpop.offsetY:%d,listpop.width:%d,listpop.height:%d\r\n",listpop.offsetX,listpop.offsetY,listpop.width,listpop.height);
    ui_listpop_set(&listpop);
}

int32_t listCallList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    callList_item_mode =0;
    callList_cur_item  = 0;
    callList_total_num = get_db_item_length(sub_func_table[cur_menu_func_id].index);
    SYSLOG_DEBUG("callList_total_num:%d\r\n",callList_total_num);

    showCallList();
    listCallList_uiloader();
    appInfo->initStatus = 1;
    SYSLOG_DEBUG("feature_menuInfo.level0Count %d\r\n",feature_menuInfo.level0Count);
    return 0;
}

int32_t listCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{

    SYSLOG_DEBUG("listCallList_proc\r\n");
    
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_DEBUG("menu_Proc msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
            switch(msg->param1)
            {
                case FUNC_CODE_MENU:
                    callList_item_mode = 1;
                    callList_cur_menu_item = 0;
                    showCallList_menu();
                    break;
                case FUNC_CODE_BACK:
                    if(callList_item_mode == 0)
                    {
                        mainApp.msgProc = menu_Proc;
                        show_menu();                        
                    }
                    if(callList_item_mode == 1)
                    {
                        callList_item_mode = 0;
                        showCallList();
                    }
                    break;
                case FUNC_CODE_DIR_U:
                    if(callList_item_mode == 0)
                    {
                        callList_cur_item--;
                        if(callList_cur_item<0)
                            callList_cur_item = callList_total_num-1;
                        showCallList();
                    }
                        
                    if(callList_item_mode == 1)
                    {
                        callList_cur_menu_item--;
                        if(callList_cur_menu_item<0)
                            callList_cur_menu_item = feature_menuInfo.level0Count-1;
                        
                        showCallList_menu();
                    }
                        
                    SYSLOG_DEBUG("callList_item_mode %d\r\n",callList_item_mode);
                    SYSLOG_DEBUG("callList_total_num:%d\r\n",callList_total_num);
                    SYSLOG_DEBUG("callList_cur_item %d\r\n",callList_cur_item);
                    SYSLOG_DEBUG("callList_cur_menu_item %d\r\n",callList_cur_menu_item);
                    break;
                case FUNC_CODE_DIR_D:
                    if(callList_item_mode == 0)
                    {
                        callList_cur_item++;
                        if(callList_cur_item>=callList_total_num)
                            callList_cur_item = 0;
                        showCallList(); 
                    }

                    if(callList_item_mode == 1)
                    {
                        callList_cur_menu_item++;
                        if(callList_cur_menu_item>=feature_menuInfo.level0Count)
                            callList_cur_menu_item = 0;
                        showCallList_menu();
                    }
                        
                    SYSLOG_DEBUG("callList_item_mode %d\r\n",callList_item_mode);
                    SYSLOG_DEBUG("callList_total_num:%d\r\n",callList_total_num);
                    SYSLOG_DEBUG("callList_cur_item %d\r\n",callList_cur_item);
                    SYSLOG_DEBUG("callList_cur_menu_item %d\r\n",callList_cur_menu_item);
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

int32_t newCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("newCallList_proc\r\n");
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_DEBUG("menu_Proc msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
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

int32_t deleteCallList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t deleteCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("deleteCallList_proc\r\n");
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_DEBUG("menu_Proc msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
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

int32_t deleteAllCallList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t deleteAllCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("deleteAllCallList_proc\r\n");
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_DEBUG("menu_Proc msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
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

int32_t queryCallList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t queryCallList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("queryCallList_proc\r\n");
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_DEBUG("menu_Proc msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
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