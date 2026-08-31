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
#include "featureDisplay.h"


extern lv_obj_t *ui_display_set(fpui_display_t *data);
extern int cur_menu_func_id;
extern sub_func sub_func_table[];
int32_t submenu_Proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
extern AppT mainApp;

fpui_display_t display_data = {0};

int display_total_num = 0;
int display_cur_item = 0;

char list_display[2][3][40] = {0};

static char list_displaybtn[3][8] = {
    "",
    "",
    "返回",
};


static uint8_t displayIndex[4] = {0xFF, 0xFF, 0xFF, 0xFF};
static const char displayName[4][6][40] =
{
    {"0", "1", "2", "0", "1", "2"},
    {"0", "1", "2", "3", "4", "5"},
    // {"AlwaysON",   "30s",        "20s",        "15s",        "10s",        "5s"},
    // {"ON",         "OFF",        "ON",         "OFF",        "ON",         "OFF"},
};

extern char cur_menu_name[];

static void showDisplay(void)
{
    display_total_num = get_db_item_length(sub_func_table[cur_menu_func_id].index);

#if 1
    display_db* display_db_1=0;
    display_db* display_db_2=0;

    display_data.select = display_cur_item;

    SYSLOG_DEBUG("cur_menu_func_id:%d\r\n",sub_func_table[cur_menu_func_id].index);
    SYSLOG_DEBUG("display_cur_item:%d\r\n",display_cur_item);

    if(display_total_num>=2)
    {
        display_data.total = 2;

        if(display_cur_item>=1)
            display_data.select = 1;
        else
            display_data.select = 0;

        int index0 = display_cur_item;
        int index1 = display_cur_item+1;
    
        if(index1 >= display_total_num || display_data.select == 1)
        {
            index0 = display_cur_item - 1;
            index1 = display_cur_item;
        }

        get_db_item(sub_func_table[cur_menu_func_id].index,index0,(char **)&display_db_1);
        SYSLOG_DEBUG("display_db: name=%s, value=%s\r\n", display_db_1->name, display_db_1->value);
        if (displayIndex[index0] == 0xFF)
        {
            displayIndex[index0] = atoi(display_db_1->value);
        }

        get_db_item(sub_func_table[cur_menu_func_id].index,index1,(char **)&display_db_2);
        SYSLOG_DEBUG("display_db: name=%s, value=%s\r\n", display_db_2->name, display_db_2->value);
        if (displayIndex[index1] == 0xFF)
        {
            displayIndex[index1] = atoi(display_db_2->value);
        }

        memcpy(list_display[0],display_db_1,sizeof(display_db));
        SYSLOG_DEBUG("list_display[0][0]:%s\r\n",list_display[0][0]);
        SYSLOG_DEBUG("list_display[0][1]:%s\r\n",list_display[0][1]);

        memcpy(list_display[1],display_db_2,sizeof(display_db));
        SYSLOG_DEBUG("list_display[1][0]:%s\r\n",list_display[1][0]);
        SYSLOG_DEBUG("list_display[1][1]:%s\r\n",list_display[1][1]);
    }
    else
    {
        display_data.total = display_total_num;
        if(display_total_num>0)
        {
            get_db_item(sub_func_table[cur_menu_func_id].index,display_cur_item,(char **)&display_db_1);
            SYSLOG_DEBUG("display_db.number:%s\r\n",display_db_1->value);
            SYSLOG_DEBUG("display_db.name:%s\r\n",display_db_1->name);

            memcpy(list_display[0],display_db_1,sizeof(display_db));
            SYSLOG_DEBUG("list_display[0].number:%s\r\n",list_display[0][0]);
        }
        else
        {
            memset(list_display, 0, sizeof(list_display));
        }
    }
#endif

    display_data.title = "显示设置";
    display_data.context = list_display;
    display_data.bottomBtn = list_displaybtn;
    ui_display_set(&display_data);

#if 1
    if(display_total_num>=2)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(display_db_1);
        vPortFree_Psram(display_db_2);
#else
        free(display_db_1);
        free(display_db_2);        
#endif
    }else if(display_total_num>0)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(display_db_1);
#else
        free(display_db_1);
#endif
    }
#endif
}

int32_t display_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("display_init\r\n");

    display_cur_item = 0;
    showDisplay();

    return 0;
}

int32_t display_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    uint8_t index = 0;

    SYSLOG_DEBUG("display_proc\r\n");
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

                case FUNC_CODE_MENU:
                    break;

                case FUNC_CODE_DIR_U:
                    if (--display_cur_item < 0)
                    {
                        display_cur_item = display_total_num - 1;
                    }
                    showDisplay();

                    SYSLOG_DEBUG("display_total_num:%d\r\n",display_total_num);
                    SYSLOG_DEBUG("display_cur_item %d\r\n",display_cur_item);
                    break;

                case FUNC_CODE_DIR_D:
                    if (++display_cur_item >= display_total_num)
                    {
                        display_cur_item = 0;
                    }
                    showDisplay();

                    SYSLOG_DEBUG("display_total_num:%d\r\n",display_total_num);
                    SYSLOG_DEBUG("display_cur_item %d\r\n",display_cur_item);
                    break;

                case FUNC_CODE_DIR_L:
                    displayIndex[display_cur_item] = (displayIndex[display_cur_item] == 0) ? 5 : (displayIndex[display_cur_item] - 1);
                    index = (display_cur_item == 0) ? 0 : 1;
                    memset(list_display[index][1], 0, 40);
                    memcpy(list_display[index][1], displayName[display_cur_item][displayIndex[display_cur_item]], 40);
                    SYSLOG_DEBUG("settings: name=%s, value=%s, %d\r\n", list_display[index][0], list_display[index][1], display_cur_item);
                    if (display_cur_item == 0)
                    {
                        saveWallPaper((char *)list_display[index]);
                    }
                    else if (display_cur_item == 1)
                    {
                        saveBacklight((char *)list_display[index]);
                    }
                    showDisplay();
                    break;

                case FUNC_CODE_DIR_R:
                    index = (display_cur_item == 0) ? 0 : 1;
                    memset(list_display[index][1], 0, 40);
                    memcpy(list_display[index][1], displayName[display_cur_item][++displayIndex[display_cur_item] % 6], 40);
                    SYSLOG_DEBUG("settings: name=%s, value=%s, %d\r\n", list_display[index][0], list_display[index][1], display_cur_item);
                    if (display_cur_item == 0)
                    {
                        saveWallPaper((char *)list_display[index]);
                    }
                    else if (display_cur_item == 1)
                    {
                        saveBacklight((char *)list_display[index]);
                    }
                    showDisplay();
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

