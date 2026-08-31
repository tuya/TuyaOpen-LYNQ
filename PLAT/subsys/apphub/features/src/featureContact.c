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
#ifdef FEATURE_SUBSYS_MISC_ENABLE
#include "misc.h"
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
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_TELECOM_ENABLE
#include "teleservice.h"
#endif
#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "ui.h"
#include "merged.h"
#endif
#include "featureContact.h"
#include "cJSON.h"
#include "phoneMenu.h"
#include "phoneContact.h"
#include "hal_screen.h"


extern sub_func_init get_init(int index);
extern sub_func_proc get_func(int index);
extern lv_obj_t *ui_App_contactDetail_set(fpui_contactDetail_t *data);
extern lv_obj_t *ui_App_contactSettingStorage_set(fpui_contactSettingStorage_t *data);
extern lv_obj_t *ui_App_contactOtherCapacity_set(fpui_contactOtherCapacity_t *data);
extern int cur_menu_func_id;
extern sub_func sub_func_table[];
void show_dial();
void run_call();
void showCallList(void);
int32_t callList_proc(AppInfoT *appInfo, AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
int32_t callInCall_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable);
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
extern int menu_index;

int contact_total_num = 0;
int contact_cur_item = 0;
int contact_cur_menu_item = 0;

int contact_item_mode = 0;


#define SHOW_COUNT_MAX      4
#define SELECT_INPUT        4

static PhoneContactT gPhoneContact[SHOW_COUNT_MAX] = {0};
static uint32_t      gSelect                       = 0;

static void showContact(void)
{
    char     list_contactbtn[3][8]                     = {"选项", "", "返回"};
    uint8_t  show_type[SHOW_COUNT_MAX + 1]             = {0};
    char     context[SHOW_COUNT_MAX][LABEL_LENGTH_MAX] = {0};
    uint32_t showCount                                 = (contact_total_num > SHOW_COUNT_MAX) ? SHOW_COUNT_MAX : contact_total_num;
    uint32_t begin                                     = 0;

    if (contact_cur_item == -1)
    {
        gSelect = SELECT_INPUT;
        begin   = 0;
    }
    else
    {
        gSelect = (contact_cur_item > (SHOW_COUNT_MAX - 1)) ? (SHOW_COUNT_MAX - 1) : contact_cur_item;
        begin   = (contact_cur_item < (SHOW_COUNT_MAX - 1)) ? 0 : (contact_cur_item - showCount + 1);
    }

    memset(gPhoneContact, 0, sizeof(gPhoneContact));
    for (uint32_t i=0; i<showCount; i++)
    {
        getContact(begin + i, &gPhoneContact[i], PHONE_LOCATION_PHONE);
    }

    for (uint32_t i=0; i<showCount; i++)
    {
        SYSLOG_DEBUG("gPhoneContact[%d]: name=%s, mobileNumber=%s, officeNumber=%s\r\n",
                     i, gPhoneContact[i].name, gPhoneContact[i].mobileNumber, gPhoneContact[i].officeNumber);
    }

    memset(show_type, 3, SHOW_COUNT_MAX);
    show_type[SHOW_COUNT_MAX] = 2;

    memset(context, 0, sizeof(context));
    for (uint32_t i=0; i<showCount; i++)
    {
        if (strlen(gPhoneContact[i].name) > 0)
        {
            memcpy(&context[i], gPhoneContact[i].name, MIN(strlen(gPhoneContact[i].name), (LABEL_LENGTH_MAX - 1)));
        }
        else if (strlen(gPhoneContact[i].mobileNumber) > 0)
        {
            memcpy(&context[i], gPhoneContact[i].mobileNumber, MIN(strlen(gPhoneContact[i].mobileNumber), (LABEL_LENGTH_MAX - 1)));
        }
        else if (strlen(gPhoneContact[i].officeNumber) > 0)
        {
            memcpy(&context[i], gPhoneContact[i].officeNumber, MIN(strlen(gPhoneContact[i].officeNumber), (LABEL_LENGTH_MAX - 1)));
        }
        show_type[i] = 0;
    }

    ui_search_set(gSelect + 1, 0, cur_menu_name, NULL, (uint8_t (*)[5])show_type, context, list_contactbtn);
}

static void showSubMenuBottom(void)
{
    static char list_contactbtn[3][8] = {"确定", "", "返回"};

    if (contact_item_mode == 1)
    {
        ui_btn_set(list_contactbtn, lv_color_white(), lv_color_black());
    }
}

void listContact_uiloader()
{
    MenuQueryT menuQueryContact =
    {
        .level      = 1,
        .level0     = 3,
        .startIndex = 0,
        .count      = -1,
        .type       = "name"
    };

    feature_menu_loader("C:/contact_cn.json");
    phoneMenuQuery("C:/contact_cn.json", &menuQueryContact, feature_lv1_menu_buf);
}

void showContact_menu()
{
    fpui_listpop_t listpopContact =
    {
        .object     = NULL,
        .screen     = NULL,
        .select     = 1,
        .total      = 5,
        .context[0] = NULL,
        .context[1] = NULL,
        .context[2] = NULL,
        .context[3] = NULL,
        .context[4] = NULL,
        .overlay    = true,    //是否叠加显示,true=不删除原弹窗
        .offsetX    = 0,        //x偏移量 如果为0则居中 -100 ~ 100
        .offsetY    = 30,       //y偏移量 如果为0则居中 -100 ~ 100
        .width      = 240,      //宽度 40 ~ 240
        .height     = 280,      //高度 40 ~ 320
        .color      = 0,
    };

    if(feature_menuInfo.level0Count>5)
    {
        if(contact_cur_menu_item>=5)
        {
            listpopContact.select = 4;
            int contact_cur_menu_start = (contact_cur_menu_item-4);
            int contact_cur_menu_end = (contact_cur_menu_start+4);
            if(contact_cur_menu_end>=feature_menuInfo.level0Count)
            {
                contact_cur_menu_end = feature_menuInfo.level0Count-1;
                contact_cur_menu_start = (contact_cur_menu_end-4);
            }
            for(uint32_t i=0;i<5;i++)
            {
                listpopContact.context[i]=&feature_lv0_menu_buf[contact_cur_menu_start+i][0];
                SYSLOG_DEBUG("app_menu_index:%d,app_menu_name:%s\r\n",i,&feature_lv0_menu_buf[i][0]);
            }  
        }else{
            listpopContact.select = contact_cur_menu_item;
            for(uint32_t i=0;i<5;i++)
            {
                listpopContact.context[i] = &feature_lv0_menu_buf[i][0];
            }
        }
        listpopContact.total = 5;
    }
    else
    {
        for(uint32_t i=0;i<feature_menuInfo.level0Count;i++)
        {
            listpopContact.context[i] = &feature_lv0_menu_buf[i][0];
        }
        listpopContact.select = contact_cur_menu_item;
        listpopContact.total = feature_menuInfo.level0Count;
    }

    ui_listpop_set(&listpopContact);
}

static int8_t gNewContactSelect = 0;
static char   gNewContactInput[3][CONTACT_NAME_LENGTH + 1] = {0};
void showNewContact(int8_t upDown)
{
    char bottomBtn[3][BTN_LENGTH_MAX] = {"保存", "", "返回"};
    char title[8][MAX_TITLE_LENGTH] =
    {
        "新联系人", "30",
        "姓名",     "pinyin",
        "移动电话", "123",
        "单位电话", "123"
    };

    gNewContactSelect += upDown;
    if (gNewContactSelect > 2)
    {
        gNewContactSelect = 0;
    }
    else if (gNewContactSelect < 0)
    {
        gNewContactSelect = 2;
    }

    char *text = (strlen(gNewContactInput[gNewContactSelect]) > 0) ? "清除" : "返回";
    memset(bottomBtn[2], 0, sizeof(bottomBtn[2]));
    memcpy(bottomBtn[2], text, strlen(text));

    ui_contact_set(gNewContactSelect + 1, 0, gNewContactInput[0], gNewContactInput[1], gNewContactInput[2], title, bottomBtn);
}

int32_t saveNewContact(void)
{
    PhoneContactT contact = {0};
    uint32_t      length  = 0;

    for (uint32_t i=0; i<3; i++)
    {
        SYSLOG_DEBUG("gNewContactInput[%d]=%s\r\n", i, gNewContactInput[i]);
    }

    memset(&contact, 0, sizeof(contact));

    length = strlen(gNewContactInput[0]);
    if (length > CONTACT_NAME_LENGTH)
    {
        length = CONTACT_NAME_LENGTH;
    }
    memcpy(contact.name, gNewContactInput[0], length);

    length = strlen(gNewContactInput[1]);
    if (length > CONTACT_NUMBER_LENGTH)
    {
        length = CONTACT_NUMBER_LENGTH;
    }
    memcpy(contact.mobileNumber, gNewContactInput[1], length);

    length = strlen(gNewContactInput[2]);
    if (length > CONTACT_NUMBER_LENGTH)
    {
        length = CONTACT_NUMBER_LENGTH;
    }
    memcpy(contact.officeNumber, gNewContactInput[2], length);

    SYSLOG_DEBUG("contact: name=%s, mobileNumber=%s, officeNumber=%s\r\n", contact.name, contact.mobileNumber, contact.officeNumber);

    return addContact(&contact, PHONE_LOCATION_PHONE);
}

#define CONTACT_DETAIL_TOTAL    3
#define CONTACT_DETAIL_LINES    2
void showContactDetail(void)
{
    uint32_t    length = 0;
    static char sContactDetailItems[CONTACT_DETAIL_TOTAL][CONTACT_DETAIL_LINES][40] =
    {
        {"姓名",     ""},
        {"移动电话", ""},
        {"单位电话", ""}
    };
    uint32_t    size = sizeof(sContactDetailItems[0][1]) - 1;
    static char sContactDetailBottomBtn[3][8] = {"", "", "返回"};
    static fpui_contactDetail_t sContactDetail =
    {
        .title     = "详情",
        .select    = 0,
        .total     = CONTACT_DETAIL_TOTAL,
        .lines     = CONTACT_DETAIL_LINES,
        .items     = (char (*)[40])sContactDetailItems,
        .bottomBtn = sContactDetailBottomBtn,
    };

    if (contact_cur_item > -1)
    {
        SYSLOG_DEBUG("gPhoneContact[%d]: name=%s, mobileNumber=%s, officeNumber=%s\r\n", gSelect,
                     gPhoneContact[gSelect].name, gPhoneContact[gSelect].mobileNumber, gPhoneContact[gSelect].officeNumber);

        for (uint32_t i=0; i<CONTACT_DETAIL_TOTAL; i++)
        {
            memset(sContactDetailItems[i][1], 0, size + 1);
        }

        length = (strlen(gPhoneContact[gSelect].name) < size) ? strlen(gPhoneContact[gSelect].name) : size;
        memcpy(sContactDetailItems[0][1], gPhoneContact[gSelect].name, length);

        length = (strlen(gPhoneContact[gSelect].mobileNumber) < size) ? strlen(gPhoneContact[gSelect].mobileNumber) : size;
        memcpy(sContactDetailItems[1][1], gPhoneContact[gSelect].mobileNumber, length);

        length = (strlen(gPhoneContact[gSelect].officeNumber) < size) ? strlen(gPhoneContact[gSelect].officeNumber) : size;
        memcpy(sContactDetailItems[2][1], gPhoneContact[gSelect].officeNumber, length);

        for (uint32_t i=0; i<CONTACT_DETAIL_TOTAL; i++)
        {
            SYSLOG_DEBUG("sContactDetailItems[%d]=%s:%s\r\n", i, sContactDetailItems[i][0], sContactDetailItems[i][1]);
        }

        ui_App_contactDetail_set(&sContactDetail);
    }
}

void showContactSettingStorage(int8_t upDown)
{
    static char sContactSettingStorageItems[4][40]    = {"手机和SIM卡", "手机", "SIM1", "SIM2"};
    static char sContactSettingStorageBottomBtn[3][8] = {"", "选择", "返回"};
    static fpui_contactSettingStorage_t sContactSettingStorage =
    {
        .title     = "存储",
        .select    = 0,
        .total     = 0,
        .lines     = 1,
        .items     = sContactSettingStorageItems,
        .bottomBtn = sContactSettingStorageBottomBtn,
    };
    static uint32_t sSelect = 0xFF;
    uint8_t         sim1    = simGetStatus(0);
    uint8_t         sim2    = simGetStatus(1);

    if (sSelect == 0xFF)
    {
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        uint32_t *pSelect = iniKeyValueRead(PHONE_CONFIG_FILE, PHONE_CONFIG_KEY_CONTACT_SETTING_STORAGE_SELECT, INI_VALUE_INT);
        if (pSelect != NULL)
        {
            sSelect = *pSelect;
            free(pSelect);
            pSelect = NULL;
        }
        else
        {
            SYSLOG_DEBUG("No matched key \"%s\" in %s\r\n", PHONE_CONFIG_KEY_CONTACT_SETTING_STORAGE_SELECT, PHONE_CONFIG_FILE);
            sSelect = 0;
            if (iniKeyValueWrite(PHONE_CONFIG_FILE, PHONE_CONFIG_KEY_CONTACT_SETTING_STORAGE_SELECT, INI_VALUE_INT, &sSelect) != 0)
            {
                SYSLOG_DEBUG("Failed to write the value of key \"%s\" to %s\r\n", PHONE_CONFIG_KEY_CONTACT_SETTING_STORAGE_SELECT, PHONE_CONFIG_FILE);
            }
        }
#else
        sSelect = 0；
#endif
    }

    if (upDown == 0)
    {
        sContactSettingStorage.select = sSelect;
        SYSLOG_DEBUG("sSelect=%d, sContactSettingStorage.select=%d\r\n", sSelect, sContactSettingStorage.select);
    }
    else if (upDown == 0x7F)
    {
        sSelect = sContactSettingStorage.select;
        SYSLOG_DEBUG("sSelect=%d, sContactSettingStorage.select=%d\r\n", sSelect, sContactSettingStorage.select);
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        if (iniKeyValueWrite(PHONE_CONFIG_FILE, PHONE_CONFIG_KEY_CONTACT_SETTING_STORAGE_SELECT, INI_VALUE_INT, &sSelect) != 0)
        {
            SYSLOG_DEBUG("Failed to write the value of key \"%s\" to %s\r\n", PHONE_CONFIG_KEY_CONTACT_SETTING_STORAGE_SELECT, PHONE_CONFIG_FILE);
        }
#endif
        return;
    }

    memset(sContactSettingStorageItems, 0, sizeof(sContactSettingStorageItems));

    if ((sim1 != SIM_READY) && (sim2 != SIM_READY))
    {
        sContactSettingStorage.total = 1;
        memcpy(sContactSettingStorageItems[0], "手机", strlen("手机"));
    }
    else
    {
        sContactSettingStorage.total = 2;
        memcpy(sContactSettingStorageItems[0], "手机和SIM卡", strlen("手机和SIM卡"));
        memcpy(sContactSettingStorageItems[1], "手机",        strlen("手机"));
        if (sim1 == SIM_READY)
        {
            memcpy(sContactSettingStorageItems[sContactSettingStorage.total], "SIM1", strlen("SIM1"));
            sContactSettingStorage.total++;
        }
        if (sim2 == SIM_READY)
        {
            memcpy(sContactSettingStorageItems[sContactSettingStorage.total], "SIM2", strlen("SIM2"));
            sContactSettingStorage.total++;
        }
    }

    if ((sContactSettingStorage.select == 0) && (upDown == -1))
    {
        sContactSettingStorage.select = sContactSettingStorage.total - 1;
    }
    else if ((sContactSettingStorage.select == (sContactSettingStorage.total - 1)) && (upDown == 1))
    {
        sContactSettingStorage.select = 0;
    }
    else
    {
        sContactSettingStorage.select += upDown;
    }

    for (uint32_t i=0; i<sContactSettingStorage.total; i++)
    {
        SYSLOG_DEBUG("sContactSettingStorageItems[%d]: %s\r\n", i, sContactSettingStorageItems[i]);
    }

    ui_App_contactSettingStorage_set(&sContactSettingStorage);
}

void showContactOtherCapacity(void)
{
    static char sContactOtherCapacityItems[3][40]    = {"卡一: 0/0", "卡二: 0/0", "手机: 0/0"};
    static char sContactOtherCapacityBottomBtn[3][8] = {"", "", "返回"};
    static fpui_contactOtherCapacity_t sContactOtherCapacity =
    {
        .title     = "容量查询",
        .total     = 0,
        .lines     = 1,
        .items     = sContactOtherCapacityItems,
        .bottomBtn = sContactOtherCapacityBottomBtn,
    };
    uint8_t   sim1     = simGetStatus(0);
    uint8_t   sim2     = simGetStatus(1);
    uint32_t  count    = 0;
    uint32_t  capacity = 0;
    uint32_t  size     = sizeof(sContactOtherCapacityItems[0]);

    sContactOtherCapacity.total = 0;
    memset(sContactOtherCapacityItems, 0, sizeof(sContactOtherCapacityItems));

    if (sim1 == SIM_READY)
    {
        getContactUsedCount(&count,   1);
        getContactCapacity(&capacity, 1);
        snprintf(sContactOtherCapacityItems[sContactOtherCapacity.total], size, "%s %d/%d", "卡一: ", count, capacity);
        sContactOtherCapacity.total++;
    }

    if (sim2 == SIM_READY)
    {
        getContactUsedCount(&count,   2);
        getContactCapacity(&capacity, 2);
        snprintf(sContactOtherCapacityItems[sContactOtherCapacity.total], size, "%s %d/%d", "卡二: ", count, capacity);
        sContactOtherCapacity.total++;
    }

    getContactUsedCount(&count,   0);
    getContactCapacity(&capacity, 0);
    snprintf(sContactOtherCapacityItems[sContactOtherCapacity.total], size, "%s %d/%d", "手机: ", count, capacity);
    sContactOtherCapacity.total++;

    for (uint32_t i=0; i<sContactOtherCapacity.total; i++)
    {
        SYSLOG_DEBUG("sContactOtherCapacityItems[%d]: %s\r\n", i, sContactOtherCapacityItems[i]);
    }

    ui_App_contactOtherCapacity_set(&sContactOtherCapacity);
}

void deleteCurrentContact(int8_t yesNo)
{
    fpui_message_t fpuiMessage = {0};

    memset(&fpuiMessage, 0, sizeof(fpuiMessage));
    fpuiMessage.width  = 160;
    fpuiMessage.height = 50;
    fpuiMessage.color  = WHITE;

    if (contact_cur_item == -1)
    {
        SYSLOG_DEBUG("Not select number.\r\n");

        fpuiMessage.context = "未选号码";
        ui_message_set(&fpuiMessage);
        osDelay(700);
        ui_message_set(NULL);

        ui_listpop_set(NULL);
        contact_item_mode = 0;
        showContact();
    }
    else
    {
        switch (yesNo)
        {
            case 0:
                fpuiMessage.context = "确定删除";
                ui_message_set(&fpuiMessage);
                contact_item_mode = 8;
                break;

            case -1:
                ui_message_set(NULL);
                ui_listpop_set(NULL);
                contact_item_mode = 0;
                showContact();
                break;

            case 1:
                ui_message_set(NULL);
                ui_listpop_set(NULL);

                SYSLOG_DEBUG("gPhoneContact[%d]: name=%s, mobileNumber=%s, officeNumber=%s\r\n", gSelect,
                             gPhoneContact[gSelect].name, gPhoneContact[gSelect].mobileNumber, gPhoneContact[gSelect].officeNumber);
                deleteContact(contact_cur_item, PHONE_LOCATION_PHONE);

                if (--contact_total_num == 0)
                {
                    contact_cur_item = -1;
                }
                else if (--contact_cur_item < 0)
                {
                    contact_cur_item = 0;
                }

                contact_item_mode = 0;
                showContact();
                break;

                default:
                    SYSLOG_DEBUG("Unknown type: yesNo=%d\r\n", yesNo);
                    break;
        }
    }
}

int32_t contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    contact_item_mode = 0;
    contact_cur_item  = -1;
    contact_total_num = get_db_item_length(sub_func_table[cur_menu_func_id].index);
    SYSLOG_DEBUG("contact_total_num:%d\r\n",contact_total_num);

    showContact();
    listContact_uiloader();
    appInfo->initStatus = 1;
    SYSLOG_DEBUG("feature_menuInfo.level0Count %d\r\n",feature_menuInfo.level0Count);

    return 0;
}

int32_t contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("contact_proc \r\n");

    uint32_t       length      = 0;
    fpui_message_t fpuiMessage = {0};

    memset(&fpuiMessage, 0, sizeof(fpuiMessage));
    fpuiMessage.height = 50;
    fpuiMessage.color  = WHITE;

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
                    if (contact_item_mode == 0)
                    {
                        if (contact_cur_item > -1)
                        {
                            contact_item_mode = 7;
                            showContactDetail();
                        }
                    }
                    else if (contact_item_mode == 5)
                    {
                        SYSLOG_DEBUG("Save Contact->Setting->Storage->Select\r\n");
                        showContactSettingStorage(0x7F);
                        contact_item_mode = 0;
                        showContact();
                    }
                    break;

                case FUNC_CODE_MENU:
                    if (contact_item_mode == 0)
                    {
                        contact_item_mode = 1;
                        contact_cur_menu_item = 0;
                        ui_listpop_set(NULL);
                        showContact();
                        osDelay(180);
                        showContact_menu();
                        showSubMenuBottom();
                    }
                    else if (contact_item_mode == 1)
                    {
                        sub_func_init menu_init = get_init(feature_lv0_func_id_buf[contact_cur_menu_item]);
                        if (menu_init != NULL)
                        {
                            menu_init(appInfo,(uint32_t)msg,reserved2,syscallTable);
                        }
                        sub_func_proc menu_fun = get_func(feature_lv0_func_id_buf[contact_cur_menu_item]);
                        if(menu_fun != NULL)
                        {
                            menu_fun(appInfo,msg,reserved2,syscallTable);
                        }
                    }
                    else if (contact_item_mode == 2)
                    {
                        if ((strlen(gNewContactInput[1]) == 0) && (strlen(gNewContactInput[2]) == 0))
                        {
                            fpuiMessage.width   = 160;
                            fpuiMessage.context = "号码为空";
                            SYSLOG_DEBUG("%s\r\n", (char *)(fpuiMessage.context));
                            ui_message_set(&fpuiMessage);
                            osDelay(700);
                            ui_message_set(NULL);
                        }
                        else
                        {
                            if (saveNewContact() == 0)
                            {
                                fpuiMessage.width   = 160;
                                fpuiMessage.context = "保存成功";
                                SYSLOG_DEBUG("%s\r\n", (char *)(fpuiMessage.context));
                                ui_message_set(&fpuiMessage);
                                osDelay(700);
                                ui_message_set(NULL);

                                if (menu_index == 2)
                                {
                                    mainApp.msgProc = callList_proc;
                                    showCallList();
                                }
                                else
                                {
                                    contact_total_num++;
                                    contact_item_mode = 0;
                                    showContact();
                                }
                            }
                            else
                            {
                                fpuiMessage.width   = 160;
                                fpuiMessage.context = "保存失败";
                                SYSLOG_DEBUG("%s\r\n", (char *)(fpuiMessage.context));
                                ui_message_set(&fpuiMessage);
                                osDelay(700);
                                ui_message_set(NULL);
                            }
                        }
                    }
                    else if (contact_item_mode == 3)
                    {
                        SYSLOG_DEBUG("Show Setting->Storage\r\n");
                        contact_item_mode = 5;
                        ui_listpop_set(NULL);
                        showContactSettingStorage(0);
                    }
                    else if (contact_item_mode == 4)
                    {
                        SYSLOG_DEBUG("Show Other->Capacity\r\n");
                        contact_item_mode = 6;
                        ui_listpop_set(NULL);
                        showContactOtherCapacity();
                    }
                    else if (contact_item_mode == 8)
                    {
                        SYSLOG_DEBUG("Delete current selected number.\r\n");
                        deleteCurrentContact(1);
                    }
                    break;
                case FUNC_CODE_BACK:
                    if(contact_item_mode == 0)
                    {
                        mainApp.msgProc = menu_Proc;
                        show_menu();                        
                    }
                    if(contact_item_mode == 1)
                    {
                        contact_item_mode = 0;
                        showContact();
                    }
                    else if (contact_item_mode == 2)
                    {
                        length = strlen(gNewContactInput[gNewContactSelect]);
                        if (length > 0)
                        {
                            gNewContactInput[gNewContactSelect][length - 1] = 0;
                            showNewContact(0);
                        }
                        else if (menu_index == 2)
                        {
                            mainApp.msgProc = callList_proc;
                            showCallList();
                        }
                        else
                        {
                            contact_item_mode = 0;
                            showContact();
                        }
                    }
                    else if ((contact_item_mode == 3) || (contact_item_mode == 4))
                    {
                        contact_item_mode = 1;
                        showContact_menu();
                    }
                    else if ((contact_item_mode == 5) || (contact_item_mode == 6) || (contact_item_mode == 7))
                    {
                        contact_item_mode = 0;
                        showContact();
                    }
                    else if (contact_item_mode == 8)
                    {
                        deleteCurrentContact(-1);
                    }
                    break;
                case FUNC_CODE_DIR_U:
                    if(contact_item_mode == 0)
                    {
                        if (contact_cur_item >= 0)
                        {
                            contact_cur_item--;
                        }
                        else if (contact_total_num > 0)
                        {
                            contact_cur_item = contact_total_num - 1;
                        }

                        showContact();
                    }
                        
                    if(contact_item_mode == 1)
                    {
                        contact_cur_menu_item--;
                        if(contact_cur_menu_item<0)
                            contact_cur_menu_item = feature_menuInfo.level0Count-1;
                        
                        showContact_menu();
                    }
                    else if (contact_item_mode == 2)
                    {
                        showNewContact(-1);
                    }
                    else if (contact_item_mode == 5)
                    {
                        showContactSettingStorage(-1);
                    }
                        
                    SYSLOG_DEBUG("contact_item_mode %d\r\n",contact_item_mode);
                    SYSLOG_DEBUG("contact_total_num:%d\r\n",contact_total_num);
                    SYSLOG_DEBUG("contact_cur_item %d\r\n",contact_cur_item);
                    SYSLOG_DEBUG("contact_cur_menu_item %d\r\n",contact_cur_menu_item);
                    break;
                case FUNC_CODE_DIR_D:
                    if(contact_item_mode == 0)
                    {
                        if (contact_cur_item == -1)
                        {
                            if (contact_total_num > 0)
                            {
                                contact_cur_item++;
                            }
                        }
                        else
                        {
                            if (contact_cur_item < contact_total_num - 1)
                            {
                                contact_cur_item++;
                            }
                            else
                            {
                                contact_cur_item = -1;
                            }
                        }

                        showContact(); 
                    }

                    if(contact_item_mode == 1)
                    {
                        contact_cur_menu_item++;
                        if(contact_cur_menu_item>=feature_menuInfo.level0Count)
                            contact_cur_menu_item = 0;
                        showContact_menu();
                    }
                    else if (contact_item_mode == 2)
                    {
                        showNewContact(1);
                    }
                    else if (contact_item_mode == 5)
                    {
                        showContactSettingStorage(1);
                    }
                        
                    SYSLOG_DEBUG("contact_item_mode %d\r\n",contact_item_mode);
                    SYSLOG_DEBUG("contact_total_num:%d\r\n",contact_total_num);
                    SYSLOG_DEBUG("contact_cur_item %d\r\n",contact_cur_item);
                    SYSLOG_DEBUG("contact_cur_menu_item %d\r\n",contact_cur_menu_item);
                    break;

                case FUNC_CODE_DIAL:
                    if ((contact_item_mode != 0) && (contact_item_mode != 7))
                    {
                        SYSLOG_DEBUG("Not support.\r\n");
                    }
                    else if (gSelect == SELECT_INPUT)
                    {
                        SYSLOG_DEBUG("No select number.\r\n");
                    }
                    else if (imsIsReady() == true)
                    {
                        if (strlen(gPhoneContact[gSelect].mobileNumber) > 0)
                        {
                            memset(call_data.numstr, 0, sizeof(call_data.numstr));
                            memcpy(call_data.numstr, gPhoneContact[gSelect].mobileNumber, strlen(gPhoneContact[gSelect].mobileNumber));
                            SYSLOG_DEBUG("call_data.numstr=%s\r\n", call_data.numstr);
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

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (contact_item_mode == 2)
                    {
                        if (gNewContactSelect == 0)
                        {
                            length = strlen(gNewContactInput[gNewContactSelect]);
                            if (length < CONTACT_NAME_LENGTH)
                            {
                                gNewContactInput[gNewContactSelect][length] = msg->param1;
                            }
                        }
                        else
                        {
                            length = strlen(gNewContactInput[gNewContactSelect]);
                            if (length < CONTACT_NUMBER_LENGTH)
                            {
                                gNewContactInput[gNewContactSelect][length] = msg->param1;
                            }
                        }
                        showNewContact(0);
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

int32_t add_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t add_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    ui_listpop_set(NULL);

    contact_item_mode = 2;
    gNewContactSelect = 0;
    memset(gNewContactInput, 0, sizeof(gNewContactInput));
    if (reserved2 != 0)
    {
        memcpy(gNewContactInput[1], (char *)reserved2, strlen((char *)reserved2));
    }
    showNewContact(0);

    return 0;
}

int32_t del_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t del_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    deleteCurrentContact(0);

    return 0;
}

int32_t copy_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t copy_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");

    return 0;
}

int32_t move_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t move_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");

    return 0;
}

int32_t set_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t set_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    fpui_listpop_t listpopContactSetting =
    {
        .object     = NULL,
        .screen     = NULL,
        .total      = 1,
        .context[0] = "存储",
        .select     = 0,
        .overlay    = true,     //是否叠加显示,true=不删除原弹窗
        .offsetX    = 82,       //x偏移量 如果为0则居中 -100 ~ 100
        .offsetY    = 100,      //y偏移量 如果为0则居中 -100 ~ 100
        .width      = 72,       //宽度 40 ~ 240
        .height     = 50,       //高度 40 ~ 320
        .color      = 0,
    };

    contact_item_mode = 3;

    ui_listpop_set(&listpopContactSetting);

    return 0;
}

int32_t import_export_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t import_export_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");

    return 0;
}

int32_t other_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t other_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    fpui_listpop_t listpopContactOther =
    {
        .object     = NULL,
        .screen     = NULL,
        .total      = 1,
        .context[0] = "容量查询",
        .select     = 0,
        .overlay    = true,     //是否叠加显示,true=不删除原弹窗
        .offsetX    = 46,       //x偏移量 如果为0则居中 -100 ~ 100
        .offsetY    = 100,      //y偏移量 如果为0则居中 -100 ~ 100
        .width      = 144,      //宽度 40 ~ 240
        .height     = 50,       //高度 40 ~ 320
        .color      = 0,
    };

    contact_item_mode = 4;

    ui_listpop_set(&listpopContactOther);

    return 0;
}
