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
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
#include "teleservice.h"
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
#include "featureCallList.h"

#include "phoneMenu.h"
#include "phoneCallList.h"
#include "hal_screen.h"


extern void show_dial();
extern void run_call();
extern sub_func_init get_init(int index);
extern sub_func_proc get_func(int index);
extern lv_obj_t *ui_App_callListDetail_set(fpui_callListDetail_t *data);
extern int cur_menu_func_id;
extern sub_func sub_func_table[];
int32_t add_contact_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t contact_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t menu_Proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
extern AppT mainApp;
extern PhoneUI_call_t call_data;
extern char feature_lv0_menu_buf[10][MENU_STR_LEN_MAX+1];
extern char feature_lv1_menu_buf[10][MENU_STR_LEN_MAX+1];
extern char feature_lv2_menu_buf[10][MENU_STR_LEN_MAX+1];
extern int32_t feature_lv0_func_id_buf[10];
extern int32_t feature_lv1_func_id_buf[10];
extern int32_t feature_lv2_func_id_buf[10];
extern MenuInfoT  feature_menuInfo;
extern char cur_menu_name[];

int callList_total_num = 0;
int callList_cur_item = 0;
int callList_cur_menu_item = 0;
int callList_item_mode = 0;

#define SHOW_COUNT_MAX 2
static PhoneCallListT gCallList[SHOW_COUNT_MAX] = {0};
static uint32_t       gSelect                   = 0;


void showCallList(void)
{
    char                  *text = NULL;
    static char            sCallListContext[SHOW_COUNT_MAX][3][40] = {0};
    static char            sCallListBtn[3][8] = {"选项", "呼叫", "返回"};
    static fpui_callist_t  sCallList = {0};

    gSelect = (callList_cur_item < (SHOW_COUNT_MAX - 1)) ? callList_cur_item : (SHOW_COUNT_MAX - 1);
    SYSLOG_DEBUG("callList_total_num=%d, gSelect=%d\r\n", callList_total_num, gSelect);

    memset(&sCallList, 0, sizeof(sCallList));
    sCallList.title     = cur_menu_name;
    sCallList.select    = gSelect;
    sCallList.total     = (callList_total_num < SHOW_COUNT_MAX) ? callList_total_num : SHOW_COUNT_MAX;
    sCallList.context   = sCallListContext;
    sCallList.bottomBtn = sCallListBtn;

    memset(gCallList, 0, sizeof(gCallList));
    if(callList_total_num >= SHOW_COUNT_MAX)
    {
        int index0 = callList_cur_item;
        int index1 = callList_cur_item+1;
    
        if(index1 >= callList_total_num || sCallList.select == 1)
        {
            index0 = callList_cur_item - 1;
            index1 = callList_cur_item;
        }

        index0 = callList_total_num - 1 - index0;
        index1 = callList_total_num - 1 - index1;

        getCallList(index0, &gCallList[0]);
        getCallList(index1, &gCallList[1]);
    }
    else if (callList_total_num == 1)
    {
        getCallList(callList_cur_item, &gCallList[0]);
    }

    memset(sCallListContext, 0, sizeof(sCallListContext));
    for (uint32_t i=0; i<sCallList.total; i++)
    {
        SYSLOG_DEBUG("gCallList[%d]: number=%s, type=%d, sim=%d, time=%d, timeStamp=%d\r\n", i,
                     gCallList[i].number, gCallList[i].type, gCallList[i].sim, gCallList[i].time, gCallList[i].timeStamp);

        memcpy(sCallListContext[i][0], gCallList[i].number, MIN(strlen(gCallList[i].number), (sizeof(sCallListContext[i][0]) - 1)));

        text = callTypeToString(gCallList[i].type);
        memcpy(sCallListContext[i][1], text, MIN(strlen(text), (sizeof(sCallListContext[i][1]) - 1)));

        text = timeStampToString(gCallList[i].timeStamp);
        memcpy(sCallListContext[i][2], text, MIN(strlen(text), (sizeof(sCallListContext[i][2]) - 1)));

        for (uint32_t j=0; j<3; j++)
        {
            SYSLOG_DEBUG("sCallListContext[%d][%d]=%s\r\n", i, j, sCallListContext[i][j]);
        }
    }

    ui_callist_set(&sCallList);
}

static void showSubMenuBottom(void)
{
    char list_callListBtn[3][8] = {"确定", "", "返回"};

    if (callList_item_mode == 1)
    {
        ui_btn_set(list_callListBtn, lv_color_white(), lv_color_black());
    }
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

static void dialledWithCheck(void)
{
    fpui_message_t fpuiMessage = {0};

    memset(&fpuiMessage, 0, sizeof(fpuiMessage));
    fpuiMessage.height = 50;
    fpuiMessage.color  = WHITE;

    if (callList_total_num == 0)
    {
        return;
    }

    if (strlen(gCallList[gSelect].number) == 0)
    {
        fpuiMessage.width   = 160;
        fpuiMessage.context = "号码为空";
        ui_message_set(&fpuiMessage);
        osDelay(700);
        ui_message_set(NULL);
    }
    else if (simGetStatus(0) != SIM_READY)
    {
        fpuiMessage.width   = 150;
        fpuiMessage.context = "无SIM卡";
        ui_message_set(&fpuiMessage);
        osDelay(700);
        ui_message_set(NULL);
    }
    else if (getCsqPercentage() == 0)
    {
        fpuiMessage.width   = 120;
        fpuiMessage.context = "无信号";
        ui_message_set(&fpuiMessage);
        osDelay(700);
        ui_message_set(NULL);
    }
    else if (imsIsReady() == false)
    {
        fpuiMessage.width   = 120;
        fpuiMessage.context = "无网络";
        ui_message_set(&fpuiMessage);
        osDelay(700);
        ui_message_set(NULL);
    }
    else
    {
        memset(call_data.numstr, 0, sizeof(call_data.numstr));
        memcpy(call_data.numstr, gCallList[gSelect].number, strlen(gCallList[gSelect].number));
        SYSLOG_DEBUG("call_data.numstr=%s\r\n", call_data.numstr);
        show_dial();
        run_call();
        mainApp.msgProc = callInCall_proc;
    }
}

#define CALL_LIST_DETAIL_TOTAL  3
#define CALL_LIST_DETAIL_LINES  2
static void showCallListDetail(int8_t upDown)
{
    static char sCallListDetailItems[CALL_LIST_DETAIL_TOTAL][CALL_LIST_DETAIL_LINES][40] =
    {
        {"主叫",     "2000-01-01 00:00"},
        {"被叫",     "2000-01-01 00:00"},
        {"主叫未通", "2000-01-01 00:00"}
    };
    static char sCallListDetailBtn[3][8] = {"选项", "呼叫", "返回"};
    static fpui_callListDetail_t sCallListDetail =
    {
        .title     = NULL,
        .select    = 0,
        .total     = 0,
        .lines     = CALL_LIST_DETAIL_LINES,
        .items     = (char (*)[40])sCallListDetailItems,
        .bottomBtn = sCallListDetailBtn,
    };
    PhoneCallListT   sCallList[CALL_LIST_DETAIL_TOTAL] = {0};
    static uint32_t  sCurrentIndex = 0;
    uint32_t         count         = 0;
    uint32_t         begin         = 0;
    uint32_t         index         = 0;
    char            *text          = NULL;

    if (callList_total_num == 0)
    {
        return;
    }

    if (upDown == 0)
    {
        ui_listpop_set(NULL);
        callList_item_mode = 2;
    }

    getCallListOneItemCount(gCallList[gSelect].number, &count);
    uint32_t ids[count];
    getCallListOneItemIds(gCallList[gSelect].number, ids, &count);

    if (upDown == 0)
    {
        sCurrentIndex = 0;
    }
    else if ((upDown == -1) && (sCurrentIndex == 0))
    {
        sCurrentIndex = count - 1;
    }
    else if ((upDown == 1) && (sCurrentIndex == (count - 1)))
    {
        sCurrentIndex = 0;
    }
    else
    {
        sCurrentIndex += upDown;
    }

    sCallListDetail.title  = gCallList[gSelect].number;
    sCallListDetail.total  = (count < CALL_LIST_DETAIL_TOTAL) ? count : CALL_LIST_DETAIL_TOTAL;
    sCallListDetail.select = (sCurrentIndex < (CALL_LIST_DETAIL_TOTAL - 1)) ? sCurrentIndex : (CALL_LIST_DETAIL_TOTAL - 1);
    begin                  = (sCurrentIndex < (CALL_LIST_DETAIL_TOTAL - 1)) ? 0 : (sCurrentIndex - (CALL_LIST_DETAIL_TOTAL - 1));

    memset(sCallList, 0, sizeof(sCallList));
    memset(sCallListDetailItems, 0, sizeof(sCallListDetailItems));
    for (uint32_t i=0; i<sCallListDetail.total; i++)
    {
        index = count - 1 - (begin + i);
        getCallList(ids[index], &sCallList[i]);
        SYSLOG_DEBUG("sCallList[%d]: number=%s, type=%d, sim=%d, time=%d, timeStamp=%d\r\n", i,
                     sCallList[i].number, sCallList[i].type, sCallList[i].sim, sCallList[i].time, sCallList[i].timeStamp);

        text = callTypeToString(sCallList[i].type);
        memcpy(sCallListDetailItems[i][0], text, MIN(strlen(text), (sizeof(sCallListDetailItems[i][0]) - 1)));

        text = timeStampToString(sCallList[i].timeStamp);
        memcpy(sCallListDetailItems[i][1], text, MIN(strlen(text), (sizeof(sCallListDetailItems[i][1]) - 1)));

        for (uint32_t j=0; j<CALL_LIST_DETAIL_LINES; j++)
        {
            SYSLOG_DEBUG("sCallListDetailItems[%d][%d]=%s\r\n", i, j, sCallListDetailItems[i][j]);
        }
    }

    ui_App_callListDetail_set(&sCallListDetail);
}

void deleteCurrentCallList(int8_t yesNo)
{
    fpui_message_t fpuiMessage = {0};

    memset(&fpuiMessage, 0, sizeof(fpuiMessage));
    fpuiMessage.context = "确定删除";
    fpuiMessage.width   = 160;
    fpuiMessage.height  = 50;
    fpuiMessage.color   = WHITE;

    if (callList_total_num == 0)
    {
        return;
    }

    switch (yesNo)
    {
        case 0:
            ui_message_set(&fpuiMessage);
            callList_item_mode = 3;
            break;

        case -1:
            ui_message_set(NULL);
            ui_listpop_set(NULL);
            callList_item_mode = 0;
            showCallList();
            break;

        case 1:
            ui_message_set(NULL);
            ui_listpop_set(NULL);

            SYSLOG_DEBUG("gCallList[%d]: number=%s, type=%d, sim=%d, time=%d, timeStamp=%d\r\n", gSelect,
                         gCallList[gSelect].number, gCallList[gSelect].type, gCallList[gSelect].sim,
                         gCallList[gSelect].time,   gCallList[gSelect].timeStamp);
            deleteCallList(callList_total_num - 1 - callList_cur_item);

            if (--callList_total_num == 0)
            {
                callList_cur_item = 0;
            }
            else if (--callList_cur_item < 0)
            {
                callList_cur_item = 0;
            }

            callList_item_mode = 0;
            showCallList();
            break;

            default:
                SYSLOG_DEBUG("Unknown type: yesNo=%d\r\n", yesNo);
                break;
    }
}

void deleteAllCallList(int8_t yesNo)
{
    fpui_message_t fpuiMessage = {0};

    memset(&fpuiMessage, 0, sizeof(fpuiMessage));
    fpuiMessage.context = "确定删除全部";
    fpuiMessage.height  = 50;
    fpuiMessage.width   = 238;
    fpuiMessage.color   = WHITE;

    if (callList_total_num == 0)
    {
        return;
    }

    switch (yesNo)
    {
        case 0:
            ui_message_set(&fpuiMessage);
            callList_item_mode = 4;
            break;

        case -1:
            ui_message_set(NULL);
            ui_listpop_set(NULL);
            callList_item_mode = 0;
            showCallList();
            break;

        case 1:
            ui_message_set(NULL);
            ui_listpop_set(NULL);

            for (int32_t i=(callList_total_num-1); i>=0; i--)
            {
                deleteCallList(i);
            }
            callList_total_num = 0;
            callList_cur_item  = 0;

            callList_item_mode = 0;
            showCallList();
            break;

            default:
                SYSLOG_DEBUG("Unknown type: yesNo=%d\r\n", yesNo);
                break;
    }
}

int32_t callList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
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

int32_t callList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("callList_proc\r\n");
    
    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_DEBUG("menu_Proc msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
            switch(msg->param1)
            {
                case FUNC_CODE_NO:
                    mainApp.msgProc = normalAppMsgProc;
                    show_home();
                    break;

                case FUNC_CODE_YES:
                    if ((callList_item_mode == 0) || (callList_item_mode == 2))
                    {
                        dialledWithCheck();
                    }
                    break;

                case FUNC_CODE_MENU:
                    if (callList_item_mode == 0)
                    {
                        callList_item_mode = 1;
                        callList_cur_menu_item = 0;
                        ui_listpop_set(NULL);
                        showCallList();
                        osDelay(180);
                        showCallList_menu();
                        showSubMenuBottom();
                    }
                    else if (callList_item_mode == 1)
                    {
                        sub_func_init menu_init = get_init(feature_lv0_func_id_buf[callList_cur_menu_item]);
                        if (menu_init != NULL)
                        {
                            menu_init(appInfo,(uint32_t)msg,reserved2,syscallTable);
                        }
                        sub_func_proc menu_fun = get_func(feature_lv0_func_id_buf[callList_cur_menu_item]);
                        if(menu_fun != NULL)
                        {
                            menu_fun(appInfo,msg,reserved2,syscallTable);
                        }
                    }
                    else if (callList_item_mode == 3)
                    {
                        deleteCurrentCallList(1);
                    }
                    else if (callList_item_mode == 4)
                    {
                        deleteAllCallList(1);
                    }
                    break;
                case FUNC_CODE_BACK:
                    if(callList_item_mode == 0)
                    {
                        mainApp.msgProc = menu_Proc;
                        show_menu();                        
                    }
                    if ((callList_item_mode == 1) || (callList_item_mode == 2))
                    {
                        callList_item_mode = 0;
                        showCallList();
                    }
                    else if (callList_item_mode == 3)
                    {
                        deleteCurrentCallList(-1);
                    }
                    else if (callList_item_mode == 4)
                    {
                        deleteAllCallList(-1);
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
                    else if (callList_item_mode == 2)
                    {
                        showCallListDetail(-1);
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
                    else if (callList_item_mode == 2)
                    {
                        showCallListDetail(1);
                    }
                        
                    SYSLOG_DEBUG("callList_item_mode %d\r\n",callList_item_mode);
                    SYSLOG_DEBUG("callList_total_num:%d\r\n",callList_total_num);
                    SYSLOG_DEBUG("callList_cur_item %d\r\n",callList_cur_item);
                    SYSLOG_DEBUG("callList_cur_menu_item %d\r\n",callList_cur_menu_item);
                    break;

                case FUNC_CODE_DIAL:
                    if ((callList_item_mode == 0) || (callList_item_mode == 2))
                    {
                        dialledWithCheck();
                    }
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

int32_t callListDialled_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t callListDialled_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    dialledWithCheck();

    return 0;
}

int32_t callListNewSms_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t callListNewSm_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");

    return 0;
}

int32_t callListDetail_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t callListDetail_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    showCallListDetail(0);

    return 0;
}

int32_t callListAddToContact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t callListAddToContact_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    if (callList_total_num > 0)
    {
        callList_item_mode = 0;
        mainApp.msgProc    = contact_proc;
        add_contact_proc(NULL, NULL, (uint32_t)(gCallList[gSelect].number), 0);
    }

    return 0;
}

int32_t callListAddToBlockList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t callListAddToBlockList_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");

    return 0;
}

int32_t callListDelete_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t callListDelete_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    deleteCurrentCallList(0);

    return 0;
}

int32_t callListDeleteAll_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t callListDeleteAll_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    deleteAllCallList(0);

    return 0;
}
