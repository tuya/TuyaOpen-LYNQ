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
#include "featureProfile.h"


extern lv_obj_t *ui_App_Profilelist_set(fpui_profilelist_t *data);
int32_t profileNormalList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable);
int32_t profileNormalList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t submenu_Proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
extern AppT mainApp;

static char list_profile_btn[3][8] = {
    "设置",
    "",
    "返回",
};
static char list_profile_normal_btn[3][8] = {
    "",
    "",
    "返回",
};
static char list_profile_items[4][40] = {
    "一般模式",
    "静音模式",
    "会议模式",
    "户外模式"
};
fpui_profilelist_t profileList_data = {0};


#define PROFILE_SUB_ITEMS_COUNT     3

static char list_profile_sub_items[PROFILE_SUB_ITEMS_COUNT][3][40] = {0};
// char list_profile_sub_items[2][3][40] = {0};

int profile_total_num = 0;
int profile_cur_item = 0;
int profile_sub_cur_item = 0;

static const uint8_t profileValueMax[3] = {1, RING_VOLUME_INDEX_MAX, CALL_VOLUME_INDEX_MAX};

void show_profilelist()
{
    SYSLOG_DEBUG("profile_cur_item:%d\r\n",profile_cur_item);

    profile_total_num = 4;

    profileList_data.title = "情景模式";
    profileList_data.total = profile_total_num;
    profileList_data.lines = 1;
    profileList_data.items = list_profile_items;
    profileList_data.select = profile_cur_item;
    profileList_data.bottomBtn = list_profile_btn;
    SYSLOG_DEBUG("profileList_data: title=%s, total=%d, select=%d, lines=%d\r\n", profileList_data.title, profileList_data.total, profileList_data.select, profileList_data.lines);
    ui_App_Profilelist_set(&profileList_data);
}

int32_t profileList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("profileList_init\r\n");

    profile_cur_item = 0;
    show_profilelist();

    return 0;
}

int32_t profileList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("profileList_proc\r\n");
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

                case FUNC_CODE_BACK:
                    mainApp.msgProc   = submenu_Proc;
                    show_list();
                    break;
                case FUNC_CODE_MENU:
                    if (profile_cur_item == 0)
                    {
                        mainApp.msgProc = profileNormalList_proc;
                        profileNormalList_init(NULL, 0, 0, 0);
                    }
                    break;

                case FUNC_CODE_DIR_U:
                    if (--profile_cur_item < 0)
                    {
                        profile_cur_item = profile_total_num - 1;
                    }
                    show_profilelist();

                    SYSLOG_DEBUG("profile_total_num:%d\r\n",profile_total_num);
                    SYSLOG_DEBUG("profile_cur_item %d\r\n",profile_cur_item);
                    break;

                case FUNC_CODE_DIR_D:
                    if (++profile_cur_item >= profile_total_num)
                    {
                        profile_cur_item = 0;
                    }
                    show_profilelist();

                    SYSLOG_DEBUG("profile_total_num:%d\r\n",profile_total_num);
                    SYSLOG_DEBUG("profile_cur_item %d\r\n",profile_cur_item);
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

void show_profileNormalList()
{
    uint32_t indexBegin = 0;
    uint32_t indexEnd   = 0;
    uint32_t total      = 2;

    SYSLOG_DEBUG("profile_sub_cur_item:%d\r\n",profile_sub_cur_item);

    memset(&profileList_data, 0, sizeof(profileList_data));
    if (profile_total_num > 0)
    {
        if ((profile_total_num > total) && (profile_sub_cur_item > total - 1))
        {
            profileList_data.total  = total;
            profileList_data.select = profileList_data.total - 1;
            indexBegin              = profile_sub_cur_item - (profileList_data.total - 1);
            indexEnd                = profile_sub_cur_item;
        }
        else
        {
            profileList_data.total  = (profile_total_num > total) ? total : profile_total_num;
            profileList_data.select = profile_sub_cur_item;
            indexBegin              = 0;
            indexEnd                = profileList_data.total - 1;
        }

        for (uint32_t i=indexBegin; i<=indexEnd; i++)
        {
            SYSLOG_DEBUG("list_profile_sub_items[%d]: name=%s, value=%s\r\n", i, list_profile_sub_items[i][0], list_profile_sub_items[i][1]);
        }
    }

    profileList_data.title = "一般模式";
    profileList_data.lines = 3;
    profileList_data.items = (char (*)[40])&list_profile_sub_items[indexBegin];
    profileList_data.bottomBtn = list_profile_normal_btn;
    SYSLOG_DEBUG("profileList_data: title=%s, total=%d, select=%d, lines=%d\r\n", profileList_data.title, profileList_data.total, profileList_data.select, profileList_data.lines);
    ui_App_Profilelist_set(&profileList_data);
}

int32_t profileNormalList_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("profileNormalList_init\r\n");

    profile_db *profileDb = NULL;
    uint32_t    index     = 51;

    profile_sub_cur_item = 0;
    profile_total_num    = PROFILE_SUB_ITEMS_COUNT;

    for (uint32_t i=0; i<profile_total_num; i++)
    {
        get_db_item(index, i, (char **)&profileDb);
        if (profileDb != NULL)
        {
            memcpy(list_profile_sub_items[i], profileDb, sizeof(profile_db));
            SYSLOG_DEBUG("profile_db[%d]: name=%s, value=%s\r\n", i, profileDb->name, profileDb->value);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
            vPortFree_Psram(profileDb);
#else
            free(profileDb);
#endif
            profileDb = NULL;
        }
    }

    show_profileNormalList();

    return 0;
}

int32_t profileNormalList_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    uint8_t value = 0;

    SYSLOG_DEBUG("profileNormalList_proc\r\n");
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

                case FUNC_CODE_DIR_U:
                    if (--profile_sub_cur_item < 0)
                    {
                        profile_sub_cur_item = profile_total_num - 1;
                    }
                    show_profileNormalList();

                    SYSLOG_DEBUG("profile_total_num:%d\r\n",profile_total_num);
                    SYSLOG_DEBUG("profile_sub_cur_item %d\r\n",profile_sub_cur_item);
                    break;

                case FUNC_CODE_DIR_D:
                    if (++profile_sub_cur_item >= profile_total_num)
                    {
                        profile_sub_cur_item = 0;
                    }
                    show_profileNormalList();

                    SYSLOG_DEBUG("profile_total_num:%d\r\n",profile_total_num);
                    SYSLOG_DEBUG("profile_sub_cur_item %d\r\n",profile_sub_cur_item);
                    break;

                case FUNC_CODE_DIR_L:
                    value = atoi(list_profile_sub_items[profile_sub_cur_item][1]);
                    SYSLOG_DEBUG("settings: name=%s, Current: %s, %d\r\n", list_profile_sub_items[profile_sub_cur_item][0], list_profile_sub_items[profile_sub_cur_item][1], value);
                    value = (value == 0) ? profileValueMax[profile_sub_cur_item] : (value - 1);
                    memset(list_profile_sub_items[profile_sub_cur_item][1], 0, 40);
                    snprintf(list_profile_sub_items[profile_sub_cur_item][1], 40, "%d", value);
                    SYSLOG_DEBUG("settings: name=%s, New: %s, %d\r\n", list_profile_sub_items[profile_sub_cur_item][0], list_profile_sub_items[profile_sub_cur_item][1], value);
                    show_profileNormalList();
                    if (profile_sub_cur_item == 0)
                    {
                        saveRingSource((char *)list_profile_sub_items[profile_sub_cur_item]);
                    }
                    if (profile_sub_cur_item == 1)
                    {
                        saveRingVolume((char *)list_profile_sub_items[profile_sub_cur_item]);
                    }
                    if (profile_sub_cur_item == 2)
                    {
                        saveCallVolume((char *)list_profile_sub_items[profile_sub_cur_item]);
                    }
                    break;

                case FUNC_CODE_DIR_R:
                    value = atoi(list_profile_sub_items[profile_sub_cur_item][1]);
                    SYSLOG_DEBUG("settings: name=%s, Current: %s, %d\r\n", list_profile_sub_items[profile_sub_cur_item][0], list_profile_sub_items[profile_sub_cur_item][1], value);
                    value = (value >= profileValueMax[profile_sub_cur_item]) ? 0 : (value + 1);
                    memset(list_profile_sub_items[profile_sub_cur_item][1], 0, 40);
                    snprintf(list_profile_sub_items[profile_sub_cur_item][1], 40, "%d", value);
                    SYSLOG_DEBUG("settings: name=%s, New: %s, %d\r\n", list_profile_sub_items[profile_sub_cur_item][0], list_profile_sub_items[profile_sub_cur_item][1], value);
                    show_profileNormalList();
                    if (profile_sub_cur_item == 0)
                    {
                        saveRingSource((char *)list_profile_sub_items[profile_sub_cur_item]);
                    }
                    if (profile_sub_cur_item == 1)
                    {
                        saveRingVolume((char *)list_profile_sub_items[profile_sub_cur_item]);
                    }
                    if (profile_sub_cur_item == 2)
                    {
                        saveCallVolume((char *)list_profile_sub_items[profile_sub_cur_item]);
                    }
                    break;

                case FUNC_CODE_BACK:
                    mainApp.msgProc = profileList_proc;
                    profileList_init(NULL, 0, 0, 0);
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
