#include <string.h>
#include "FreeRTOS.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "charge.h"
#include "ps_sms_if.h"
#ifdef FEATURE_DRIVER_KEYPAD_ENABLE
#include "keypad.h"
#include "kpc.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include "systime.h"
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
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "jsonDb.h"
#endif
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
#include "teleservice.h"
#endif

#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "ui.h"
#include "merged.h"
#endif
#include "featureSms.h"


extern int cur_menu_func_id;
extern sub_func sub_func_table[];
int32_t submenu_Proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
extern AppT mainApp;

extern lv_obj_t *ui_app_sms_show(fpui_smshow_t *data);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
static PLAT_FPSRAM_ZI_CUST char gSmsText[40 + 40 + PSIL_SMS_MAX_TXT_SIZE + 1 + 8] = {0};
#else
static char gSmsText[40 + 40 + PSIL_SMS_MAX_TXT_SIZE + 1 + 8] = {0};
#endif
static const char gSmsDetailbtn[3][8] = {"", "", "返回"};
static const fpui_smshow_t gSmsData = {.title = "信息详情", .message = gSmsText, .bottomBtn = (char (*)[8])gSmsDetailbtn};
static bool gSmsInDetail = false;

fpui_sms_t presetSms_data = {0};
int presetSms_total_num = 0;
int presetSms_cur_item = 0;
char list_presetSms[6][40] = {0};
static char list_presetSmsbtn[3][8] = {
    "发送",
    "",
    "返回",
};

fpui_sms_t smsInbox_data = {0};
int smsInbox_total_num = 0;
int smsInbox_cur_item = 0;
char list_smsInbox[2][3][40] = {0};
static char list_smsInboxbtn[3][8] = {
    "查看",
    "",
    "返回",
};

fpui_sms_t smsOutbox_data = {0};
int smsOutbox_total_num = 0;
int smsOutbox_cur_item = 0;
char list_smsOutbox[2][3][40] = {0};
static char list_smsOutboxbtn[3][8] = {
    "",
    "",
    "返回",
};

extern char cur_menu_name[];

static void showPresetSms(void)
{
    presetSms_db *presetSmsDb = NULL;
    uint32_t      indexBegin  = 0;
    uint32_t      indexEnd    = 0;
    uint32_t      total       = 6;

    SYSLOG_DEBUG("cur_menu_func_id:%d\r\n",sub_func_table[cur_menu_func_id].index);
    SYSLOG_DEBUG("presetSms_cur_item:%d\r\n",presetSms_cur_item);

    presetSms_total_num = get_db_item_length(sub_func_table[cur_menu_func_id].index);

    memset(&presetSms_data, 0, sizeof(presetSms_data));
    memset(list_presetSms, 0, sizeof(list_presetSms));
    if (presetSms_total_num > 0)
    {
        if ((presetSms_total_num > total) && (presetSms_cur_item > total - 1))
        {
            presetSms_data.total  = total;
            presetSms_data.select = presetSms_data.total - 1;
            indexBegin            = presetSms_cur_item - (presetSms_data.total - 1);
            indexEnd              = presetSms_cur_item;
        }
        else
        {
            presetSms_data.total  = (presetSms_total_num > total) ? total : presetSms_total_num;
            presetSms_data.select = presetSms_cur_item;
            indexBegin            = 0;
            indexEnd              = presetSms_data.total - 1;
        }

        for (uint32_t i=indexBegin; i<=indexEnd; i++)
        {
            get_db_item(sub_func_table[cur_menu_func_id].index, i, (char **)&presetSmsDb);
            if (presetSmsDb != NULL)
            {
                memcpy(list_presetSms[i - indexBegin], presetSmsDb, strlen((char *)presetSmsDb));
                SYSLOG_DEBUG("presetSms_db[%d]: %s\r\n", i, presetSmsDb->data);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
                vPortFree_Psram(presetSmsDb);
#else
                free(presetSmsDb);
#endif
                presetSmsDb = NULL;
            }
        }
    }

    presetSms_data.title = "常用短信";
    presetSms_data.lines = 1;
    presetSms_data.context = (char *)list_presetSms;
    presetSms_data.bottomBtn = list_presetSmsbtn;
    SYSLOG_DEBUG("presetSms_data: title=%s, total=%d, select=%d, lines=%d\r\n", presetSms_data.title, presetSms_data.total, presetSms_data.select, presetSms_data.lines);
    // ui_sms_set(&presetSms_data);
}

static void showSmsInbox(void)
{
    smsInbox_db *smsInboxDb = NULL;

    SYSLOG_DEBUG("cur_menu_func_id:%d\r\n",sub_func_table[cur_menu_func_id].index);
    SYSLOG_DEBUG("smsInbox_cur_item:%d\r\n",smsInbox_cur_item);

    smsInbox_total_num = get_db_item_length(sub_func_table[cur_menu_func_id].index);

    memset(gSmsText, 0, sizeof(gSmsText));
    memset(&smsInbox_data, 0, sizeof(smsInbox_data));
    memset(list_smsInbox, 0, sizeof(list_smsInbox));
    if(smsInbox_total_num >= 2)
    {
        smsInbox_data.total = 2;
        smsInbox_data.select = (smsInbox_cur_item >= 1) ? 1 : 0;

        int index0 = smsInbox_cur_item;
        int index1 = smsInbox_cur_item+1;
    
        if(index1 >= smsInbox_total_num || smsInbox_data.select == 1)
        {
            index0 = smsInbox_cur_item - 1;
            index1 = smsInbox_cur_item;
        }

        get_db_item(sub_func_table[cur_menu_func_id].index, index0, (char **)&smsInboxDb);
        if (smsInboxDb != NULL)
        {
            memcpy(list_smsInbox[0], smsInboxDb, sizeof(list_smsInbox[0]) - 1);
            SYSLOG_DEBUG("list_smsInbox[0][0]: %s\r\n", list_smsInbox[0][0]);
            SYSLOG_DEBUG("list_smsInbox[0][1]: %s\r\n", list_smsInbox[0][1]);
            SYSLOG_DEBUG("list_smsInbox[0][2]: %s\r\n", list_smsInbox[0][2]);
            if (smsInbox_data.select == 0)
            {
                memcpy(gSmsText, smsInboxDb->number, strlen(smsInboxDb->number));
                memcpy(&gSmsText[strlen(gSmsText)], "\r\n", strlen("\r\n"));
                memcpy(&gSmsText[strlen(gSmsText)], smsInboxDb->time, strlen(smsInboxDb->time));
                memcpy(&gSmsText[strlen(gSmsText)], "\r\n", strlen("\r\n"));
                memcpy(&gSmsText[strlen(gSmsText)], smsInboxDb->data, strlen(smsInboxDb->data));
            }
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
            vPortFree_Psram(smsInboxDb);
#else
            free(smsInboxDb);
#endif
            smsInboxDb = NULL;
        }

        get_db_item(sub_func_table[cur_menu_func_id].index, index1, (char **)&smsInboxDb);
        if (smsInboxDb != NULL)
        {
            memcpy(list_smsInbox[1], smsInboxDb, sizeof(list_smsInbox[1]) - 1);
            SYSLOG_DEBUG("list_smsInbox[1][0]: %s\r\n", list_smsInbox[1][0]);
            SYSLOG_DEBUG("list_smsInbox[1][1]: %s\r\n", list_smsInbox[1][1]);
            SYSLOG_DEBUG("list_smsInbox[1][2]: %s\r\n", list_smsInbox[1][2]);
            if (smsInbox_data.select == 1)
            {
                memcpy(gSmsText, list_smsInbox[1][0], strlen(list_smsInbox[1][0]));
                memcpy(&gSmsText[strlen(gSmsText)], "\r\n", strlen("\r\n"));
                memcpy(&gSmsText[strlen(gSmsText)], list_smsInbox[1][1], strlen(list_smsInbox[1][1]));
                memcpy(&gSmsText[strlen(gSmsText)], "\r\n", strlen("\r\n"));
                memcpy(&gSmsText[strlen(gSmsText)], list_smsInbox[1][2], strlen(list_smsInbox[1][2]));
            }
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
            vPortFree_Psram(smsInboxDb);
#else
            free(smsInboxDb);
#endif
            smsInboxDb = NULL;
        }
    }
    else
    {
        smsInbox_data.total = smsInbox_total_num;
        if(smsInbox_total_num > 0)
        {
            get_db_item(sub_func_table[cur_menu_func_id].index, smsInbox_cur_item, (char **)&smsInboxDb);
            if (smsInboxDb != NULL)
            {
                memcpy(list_smsInbox[0], smsInboxDb, sizeof(list_smsInbox[0]) - 1);
                SYSLOG_DEBUG("list_smsInbox[0][0]: %s\r\n", list_smsInbox[0][0]);
                SYSLOG_DEBUG("list_smsInbox[0][1]: %s\r\n", list_smsInbox[0][1]);
                SYSLOG_DEBUG("list_smsInbox[0][2]: %s\r\n", list_smsInbox[0][2]);
                memcpy(gSmsText, list_smsInbox[0][0], strlen(list_smsInbox[0][0]));
                memcpy(&gSmsText[strlen(gSmsText)], "\r\n", strlen("\r\n"));
                memcpy(&gSmsText[strlen(gSmsText)], list_smsInbox[0][1], strlen(list_smsInbox[0][1]));
                memcpy(&gSmsText[strlen(gSmsText)], "\r\n", strlen("\r\n"));
                memcpy(&gSmsText[strlen(gSmsText)], list_smsInbox[0][2], strlen(list_smsInbox[0][2]));
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
                vPortFree_Psram(smsInboxDb);
#else
                free(smsInboxDb);
#endif
                smsInboxDb = NULL;
            }
        }
    }

    smsInbox_data.title = "收件箱";
    smsInbox_data.lines = 3;
    smsInbox_data.context = (char *)list_smsInbox;
    smsInbox_data.bottomBtn = list_smsInboxbtn;
    SYSLOG_DEBUG("smsInbox_data: title=%s, total=%d, select=%d, lines=%d\r\n", smsInbox_data.title, smsInbox_data.total, smsInbox_data.select, smsInbox_data.lines);
    // ui_sms_set(&smsInbox_data);
}

static void showSmsOutbox(void)
{
    smsOutbox_db *smsOutboxDb = NULL;

    SYSLOG_DEBUG("cur_menu_func_id:%d\r\n",sub_func_table[cur_menu_func_id].index);
    SYSLOG_DEBUG("smsOutbox_cur_item:%d\r\n",smsOutbox_cur_item);

    smsOutbox_total_num = get_db_item_length(sub_func_table[cur_menu_func_id].index);

    memset(&smsOutbox_data, 0, sizeof(smsOutbox_data));
    memset(list_smsOutbox, 0, sizeof(list_smsOutbox));
    if(smsOutbox_total_num >= 2)
    {
        smsOutbox_data.total = 2;
        smsOutbox_data.select = (smsOutbox_cur_item >= 1) ? 1 : 0;

        int index0 = smsOutbox_cur_item;
        int index1 = smsOutbox_cur_item+1;
    
        if(index1 >= smsOutbox_total_num || smsOutbox_data.select == 1)
        {
            index0 = smsOutbox_cur_item - 1;
            index1 = smsOutbox_cur_item;
        }

        get_db_item(sub_func_table[cur_menu_func_id].index, index0, (char **)&smsOutboxDb);
        if (smsOutboxDb != NULL)
        {
            memcpy(list_smsOutbox[0], smsOutboxDb, sizeof(smsOutbox_db));
            SYSLOG_DEBUG("list_smsOutbox[0][0]: %s\r\n", list_smsOutbox[0][0]);
            SYSLOG_DEBUG("list_smsOutbox[0][1]: %s\r\n", list_smsOutbox[0][1]);
            SYSLOG_DEBUG("list_smsOutbox[0][2]: %s\r\n", list_smsOutbox[0][2]);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
            vPortFree_Psram(smsOutboxDb);
#else
            free(smsOutboxDb);
#endif
            smsOutboxDb = NULL;
        }


        get_db_item(sub_func_table[cur_menu_func_id].index, index1, (char **)&smsOutboxDb);
        if (smsOutboxDb != NULL)
        {
            memcpy(list_smsOutbox[1], smsOutboxDb, sizeof(smsOutbox_db));
            SYSLOG_DEBUG("list_smsOutbox[1][0]: %s\r\n", list_smsOutbox[1][0]);
            SYSLOG_DEBUG("list_smsOutbox[1][1]: %s\r\n", list_smsOutbox[1][1]);
            SYSLOG_DEBUG("list_smsOutbox[1][2]: %s\r\n", list_smsOutbox[1][2]);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
            vPortFree_Psram(smsOutboxDb);
#else
            free(smsOutboxDb);
#endif
            smsOutboxDb = NULL;
        }
    }
    else
    {
        smsOutbox_data.total = smsOutbox_total_num;
        if(smsOutbox_total_num > 0)
        {
            get_db_item(sub_func_table[cur_menu_func_id].index, smsOutbox_cur_item, (char **)&smsOutboxDb);
            if (smsOutboxDb != NULL)
            {
                memcpy(list_smsOutbox[0], smsOutboxDb, sizeof(smsOutbox_db));
                SYSLOG_DEBUG("list_smsOutbox[0][0]: %s\r\n", list_smsOutbox[0][0]);
                SYSLOG_DEBUG("list_smsOutbox[0][1]: %s\r\n", list_smsOutbox[0][1]);
                SYSLOG_DEBUG("list_smsOutbox[0][2]: %s\r\n", list_smsOutbox[0][2]);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
                vPortFree_Psram(smsOutboxDb);
#else
                free(smsOutboxDb);
#endif
                smsOutboxDb = NULL;
            }
        }
    }

    smsOutbox_data.title = "发件箱";
    smsOutbox_data.lines = 3;
    smsOutbox_data.context = (char *)list_smsOutbox;
    smsOutbox_data.bottomBtn = list_smsOutboxbtn;
    SYSLOG_DEBUG("smsInbox_data: title=%s, total=%d, select=%d, lines=%d\r\n", smsOutbox_data.title, smsOutbox_data.total, smsOutbox_data.select, smsOutbox_data.lines);
    // ui_sms_set(&smsOutbox_data);
}

int32_t newSms_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("newSms_init\r\n");

    return 0;
}

int32_t newSms_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("newSms_proc\r\n");
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

int32_t presetSms_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("presetSms_init\r\n");

    presetSms_cur_item = 0;
    showPresetSms();

    return 0;
}

static void saveSmsIntoOutbox(char *number, char *text)
{
    if ((number != NULL) && (text != NULL))
    {
        char outbox[3][40] = {0};

        memset(outbox, 0, sizeof(outbox));
        memcpy(outbox[0], number, strlen(number));

        time_t     time1  = time_time(NULL);
        struct tm *tmTime = time_gmtime(&time1);
        snprintf(outbox[1], sizeof(outbox[1]), "%d-%d-%d %d:%d:%d", 1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);

        memcpy(outbox[2], text, strlen(text));
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
        jsonDbAddItem("D:/smsOutboxDb.json", "D:/smsOutboxDb.type", outbox);
#endif
    }
}

int32_t presetSms_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    char *number = "123";
    char *text   = list_presetSms[presetSms_data.select];

    SYSLOG_DEBUG("presetSms_proc\r\n");
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

                case FUNC_CODE_MENU:
                    SYSLOG_DEBUG("Send SMS: number=%s, text=%s\r\n", number, text);
                    smsSendText(number, text);
                    saveSmsIntoOutbox(number, text);
                    break;

                case FUNC_CODE_DIR_U:
                    if (--presetSms_cur_item < 0)
                    {
                        presetSms_cur_item = presetSms_total_num - 1;
                    }
                    showPresetSms();

                    SYSLOG_DEBUG("presetSms_total_num:%d\r\n",presetSms_total_num);
                    SYSLOG_DEBUG("presetSms_cur_item %d\r\n",presetSms_cur_item);
                    break;

                case FUNC_CODE_DIR_D:
                    if (++presetSms_cur_item >= presetSms_total_num)
                    {
                        presetSms_cur_item = 0;
                    }
                    showPresetSms();

                    SYSLOG_DEBUG("presetSms_total_num:%d\r\n",presetSms_total_num);
                    SYSLOG_DEBUG("presetSms_cur_item %d\r\n",presetSms_cur_item);
                    break;

                case FUNC_CODE_BACK:
                    mainApp.msgProc = submenu_Proc;
                    show_list();
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
int32_t deleteSms_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("deleteSms_proc\r\n");
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
int32_t editSms_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("editSms_proc\r\n");
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

int32_t smsOutboxList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("smsOutboxList_init\r\n");

    smsOutbox_cur_item = 0;
    showSmsOutbox();

    return 0;
}

int32_t smsOutboxList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("smsOutboxList_proc\r\n");
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

                case FUNC_CODE_MENU:
                    break;

                case FUNC_CODE_DIR_U:
                    if (--smsOutbox_cur_item < 0)
                    {
                        smsOutbox_cur_item = smsOutbox_total_num - 1;
                    }
                    showSmsOutbox();

                    SYSLOG_DEBUG("smsOutbox_total_num:%d\r\n",smsOutbox_total_num);
                    SYSLOG_DEBUG("smsOutbox_cur_item %d\r\n",smsOutbox_cur_item);
                    break;

                case FUNC_CODE_DIR_D:
                    if (++smsOutbox_cur_item >= smsOutbox_total_num)
                    {
                        smsOutbox_cur_item = 0;
                    }
                    showSmsOutbox();

                    SYSLOG_DEBUG("smsOutbox_total_num:%d\r\n",smsOutbox_total_num);
                    SYSLOG_DEBUG("smsOutbox_cur_item %d\r\n",smsOutbox_cur_item);
                    break;

                case FUNC_CODE_BACK:
                    mainApp.msgProc = submenu_Proc;
                    show_list();
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

int32_t smsInboxList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("smsInboxList_init\r\n");

    smsInbox_cur_item = 0;
    showSmsInbox();

    return 0;
}

int32_t smsInboxList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("smsInboxList_proc\r\n");
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

                case FUNC_CODE_MENU:
                    if ((smsInbox_total_num != 0) && (gSmsInDetail != true))
                    {
                        SYSLOG_DEBUG("gSmsText=%s\r\n", gSmsText);
                        gSmsInDetail = true;
                        ui_app_sms_show((fpui_smshow_t *)&gSmsData);
                    }
                    break;

                case FUNC_CODE_DIR_U:
                    if (gSmsInDetail != true)
                    {
                        if (--smsInbox_cur_item < 0)
                        {
                            smsInbox_cur_item = smsInbox_total_num - 1;
                        }
                        showSmsInbox();

                        SYSLOG_DEBUG("smsInbox_total_num:%d\r\n",smsInbox_total_num);
                        SYSLOG_DEBUG("smsInbox_cur_item %d\r\n",smsInbox_cur_item);
                    }
                    break;

                case FUNC_CODE_DIR_D:
                    if (gSmsInDetail != true)
                    {
                        if (++smsInbox_cur_item >= smsInbox_total_num)
                        {
                            smsInbox_cur_item = 0;
                        }
                        showSmsInbox();

                        SYSLOG_DEBUG("smsInbox_total_num:%d\r\n",smsInbox_total_num);
                        SYSLOG_DEBUG("smsInbox_cur_item %d\r\n",smsInbox_cur_item);
                    }
                    break;

                case FUNC_CODE_BACK:
                    if (gSmsInDetail == true)
                    {
                        gSmsInDetail = false;
                        showSmsInbox();
                    }
                    else
                    {
                        mainApp.msgProc = submenu_Proc;
                        show_list();
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