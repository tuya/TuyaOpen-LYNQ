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
#include "featureTts.h"


extern lv_obj_t *ui_tts_set(fpui_tts_t *data);
extern int cur_menu_func_id;
extern sub_func sub_func_table[];
void setTtsMainSwitch(char *ttsMainSwitch);
int32_t submenu_Proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
extern AppT mainApp;

fpui_tts_t tts_data = {0};

int tts_total_num = 0;
int tts_cur_item = 0;

char list_tts[2][40] = {0};

static char list_ttsbtn[3][8] = {
    "",
    "",
    "返回",
};

static uint8_t ttsIndex = 0;
static const char ttsName[2][40] = {"关闭", "打开"};

extern char cur_menu_name[];

static void showTts(void)
{
    tts_db *ttsDb = NULL;

    SYSLOG_DEBUG("cur_menu_func_id:%d\r\n",sub_func_table[cur_menu_func_id].index);
    SYSLOG_DEBUG("tts_cur_item:%d\r\n",tts_cur_item);

    tts_total_num = get_db_item_length(sub_func_table[cur_menu_func_id].index);

    memset(&tts_data, 0, sizeof(tts_data));
    memset(list_tts, 0, sizeof(list_tts));
    get_db_item(sub_func_table[cur_menu_func_id].index, 0, (char **)&ttsDb);
    if (ttsDb != NULL)
    {
        memcpy(list_tts, ttsDb, sizeof(tts_db));
        SYSLOG_DEBUG("list_tts[0]: %s\r\n", list_tts[0]);
        SYSLOG_DEBUG("list_tts[1]: %s\r\n", list_tts[1]);
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(ttsDb);
#else
        free(ttsDb);
#endif
        ttsDb = NULL;
    }

    tts_data.title = "语音王";
    tts_data.total = 1;
    tts_data.select = 0;
    tts_data.context = (char (*)[3][40])list_tts;
    tts_data.bottomBtn = list_ttsbtn;
    ui_tts_set(&tts_data);
}

int32_t tts_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("tts_init\r\n");

    tts_cur_item = 0;
    showTts();

    return 0;
}

int32_t tts_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("tts_proc\r\n");
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

                case FUNC_CODE_DIR_L:
                case FUNC_CODE_DIR_R:
                    ttsIndex = (ttsIndex == 0) ? 1 : 0;
                    setTtsMainSwitch((char *)ttsName[ttsIndex]);
                    memset(list_tts[1], 0, 40);
                    memcpy(list_tts[1], ttsName[ttsIndex], 40);
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
                    jsonDbUpdateItem("D:/ttsDb.json", "D:/ttsDb.type", tts_cur_item, list_tts);
#endif
                    SYSLOG_DEBUG("settings: name=%s, value=%s\r\n", list_tts[0], list_tts[1]);
                    showTts();
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

