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

#ifdef FEATURE_SUBSYS_GUI_FPUI_ENABLE
#include "fpui.h"
#include "ui.h"
#include "merged.h"
#endif
#include "featureContact.h"
#include "cJSON.h"
#include "phoneMenu.h"


extern int cur_menu_func_id;
extern sub_func sub_func_table[];
void show_dial();
void run_call();
void add_call_list();
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

fpui_contact_t contact_data = {0};

int contact_total_num = 0;
int contact_cur_item = 0;
int contact_cur_menu_item = 0;

int contact_item_mode = 0;

char list_contact[2][3][40] = {0};

static char list_contactbtn[3][8] = {
    "选项",
    "",
    "返回",
};

extern char cur_menu_name[];

static void showContact(void)
{
    contact_total_num = get_db_item_length(sub_func_table[cur_menu_func_id].index);

#if 1
    contact_db* contact_db_1=0;
    contact_db* contact_db_2=0;

    contact_data.title = cur_menu_name;
    contact_data.select = contact_cur_item;

    SYSLOG_DEBUG("cur_menu_func_id:%d\r\n",sub_func_table[cur_menu_func_id].index);
    SYSLOG_DEBUG("contact_cur_item:%d\r\n",contact_cur_item);

    if(contact_total_num>=2)
    {
        contact_data.total = 2;

        if(contact_cur_item>=1)
            contact_data.select = 1;
        else
            contact_data.select = 0;

        int index0 = contact_cur_item;
        int index1 = contact_cur_item+1;
    
        if(index1 >= contact_total_num || contact_data.select == 1)
        {
            index0 = contact_cur_item - 1;
            index1 = contact_cur_item;
        }

        get_db_item(sub_func_table[cur_menu_func_id].index,index0,&contact_db_1);
        SYSLOG_DEBUG("contact_db.number:%s\r\n",contact_db_1->number);
        SYSLOG_DEBUG("contact_db.name:%s\r\n",contact_db_1->name);

        get_db_item(sub_func_table[cur_menu_func_id].index,index1,&contact_db_2);
        SYSLOG_DEBUG("contact_db.number:%s\r\n",contact_db_2->number);
        SYSLOG_DEBUG("contact_db.name:%s\r\n",contact_db_2->name);

        memcpy(&list_contact[0][0][0],contact_db_1,sizeof(contact_db));
        SYSLOG_DEBUG("list_contact[0].number:%s\r\n",&list_contact[0][0]);
        SYSLOG_DEBUG("list_contact[0][1]:%s\r\n",&list_contact[0][1]);
        SYSLOG_DEBUG("list_contact[0][2]:%s\r\n",&list_contact[0][2]);

        memcpy(&list_contact[1][0][0],contact_db_2,sizeof(contact_db));
        SYSLOG_DEBUG("list_contact[1].number:%s\r\n",&list_contact[1][0]);


    }
    else
    {
        contact_data.total = contact_total_num;
        if(contact_total_num>0)
        {
            get_db_item(sub_func_table[cur_menu_func_id].index,contact_cur_item,&contact_db_1);
            SYSLOG_DEBUG("contact_db.number:%s\r\n",contact_db_1->number);
            SYSLOG_DEBUG("contact_db.name:%s\r\n",contact_db_1->name);

            memcpy(&list_contact[0][0][0],contact_db_1,sizeof(contact_db));
            SYSLOG_DEBUG("list_contact[0].number:%s\r\n",&list_contact[0][0]);
        }
        else
        {
            memset(&list_contact[0][0][0], 0, sizeof(contact_db) * 2);
        }
    }
#endif

    contact_data.title = cur_menu_name;
    contact_data.context = list_contact;
    contact_data.bottomBtn = list_contactbtn;
    ui_contact_set(&contact_data);

#if 1
    if(contact_total_num>=2)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(contact_db_1);
        vPortFree_Psram(contact_db_2);
#else
        free(contact_db_1);
        free(contact_db_2);        
#endif
    }else if(contact_total_num>0)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(contact_db_1);
#else
        free(contact_db_1);
#endif
    }
#endif
}

MenuQueryT menuQueryContact =
{
    .level      = 1,
    .level0     = 3,
    .startIndex = 0,
    .count      = -1,
    .type       = "name"
};

void listContact_uiloader()
{
    feature_menu_loader("C:/contact_cn.json");
    phoneMenuQuery("C:/contact_cn.json", &menuQueryContact, feature_lv1_menu_buf);
}

fpui_listpop_t listpopContact = {
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

void showContact_menu()
{
    int i=0;
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
            for(int i=0;i<5;i++)
            {
                listpopContact.context[i]=&feature_lv0_menu_buf[contact_cur_menu_start+i][0];
                SYSLOG_DEBUG("app_menu_index:%d,app_menu_name:%s\r\n",i,&feature_lv0_menu_buf[i][0]);
            }  
        }else{
            listpopContact.select = contact_cur_menu_item;
            for(i=0;i<5;i++)
            {
                listpopContact.context[i] = &feature_lv0_menu_buf[i][0];
            }
        }
        listpopContact.total = 5;
    }
    else
    {
        for(i=0;i<feature_menuInfo.level0Count;i++)
        {
            listpopContact.context[i] = &feature_lv0_menu_buf[i][0];
        }
        listpopContact.select = contact_cur_menu_item;
        listpopContact.total = feature_menuInfo.level0Count;
    }
    SYSLOG_DEBUG("listpopContact.select:%d,listpopContact.total:%d\r\n",listpopContact.select,listpopContact.total);
    SYSLOG_DEBUG("listpopContact.offsetX:%d,listpopContact.offsetY:%d,listpopContact.width:%d,listpopContact.height:%d\r\n",listpopContact.offsetX,listpopContact.offsetY,listpopContact.width,listpopContact.height);

    ui_listpop_set(&listpopContact);
}

int32_t contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    contact_item_mode =0;
    contact_cur_item  = 0;
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

    fpui_message_t fpuiMessage = {.height = 50, .color = WHITE};

    switch(msg->msgType)
    {
        case APP_KEY_MSG:
            SYSLOG_DEBUG("menu_Proc msgType:%d,key:0x%x\r\n",msg->msgType,msg->param1);
            switch(msg->param1)
            {
                case FUNC_CODE_MENU:
                    if (contact_item_mode == 0)
                    {
                        contact_item_mode = 1;
                        contact_cur_menu_item = 0;
                        showContact_menu();
                    }
                    else if (contact_item_mode == 1)
                    {
                        sub_func_init menu_init = get_init(feature_lv0_func_id_buf[contact_cur_menu_item]);
                        if (menu_init != NULL)
                        {
                            menu_init(appInfo,msg,reserved2,syscallTable);
                        }
                        sub_func_proc menu_fun = get_func(feature_lv0_func_id_buf[contact_cur_menu_item]);
                        if(menu_fun != NULL)
                        {
                            menu_fun(appInfo,msg,reserved2,syscallTable);
                        }
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
                    break;
                case FUNC_CODE_DIR_U:
                    if(contact_item_mode == 0)
                    {
                        contact_cur_item--;
                        if(contact_cur_item<0)
                            contact_cur_item = contact_total_num-1;
                        showContact();
                    }
                        
                    if(contact_item_mode == 1)
                    {
                        contact_cur_menu_item--;
                        if(contact_cur_menu_item<0)
                            contact_cur_menu_item = feature_menuInfo.level0Count-1;
                        
                        showContact_menu();
                    }
                        
                    SYSLOG_DEBUG("contact_item_mode %d\r\n",contact_item_mode);
                    SYSLOG_DEBUG("contact_total_num:%d\r\n",contact_total_num);
                    SYSLOG_DEBUG("contact_cur_item %d\r\n",contact_cur_item);
                    SYSLOG_DEBUG("contact_cur_menu_item %d\r\n",contact_cur_menu_item);
                    break;
                case FUNC_CODE_DIR_D:
                    if(contact_item_mode == 0)
                    {
                        contact_cur_item++;
                        if(contact_cur_item>=contact_total_num)
                            contact_cur_item = 0;
                        showContact(); 
                    }

                    if(contact_item_mode == 1)
                    {
                        contact_cur_menu_item++;
                        if(contact_cur_menu_item>=feature_menuInfo.level0Count)
                            contact_cur_menu_item = 0;
                        showContact_menu();
                    }
                        
                    SYSLOG_DEBUG("contact_item_mode %d\r\n",contact_item_mode);
                    SYSLOG_DEBUG("contact_total_num:%d\r\n",contact_total_num);
                    SYSLOG_DEBUG("contact_cur_item %d\r\n",contact_cur_item);
                    SYSLOG_DEBUG("contact_cur_menu_item %d\r\n",contact_cur_menu_item);
                    break;

                case FUNC_CODE_DIAL:
                    if(nwIsReady() == true)
                    {
                        if (strlen(list_contact[contact_data.select][1]) > 0)
                        {
                            memset(call_data.numstr, 0, sizeof(call_data.numstr));
                            memcpy(call_data.numstr, list_contact[contact_data.select][1], strlen(list_contact[contact_data.select][1]));
                            SYSLOG_DEBUG("call_data.numstr=%s\r\n", call_data.numstr);
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
    SYSLOG_DEBUG("add_contact_proc \r\n");

    char pathModel[40] = {0};
    char pathType[40]  = {0};

    memset(pathModel, 0, sizeof(pathModel));
    snprintf(pathModel, sizeof(pathModel), "D:/%s.json", sub_func_table[cur_menu_func_id].name);
    SYSLOG_DEBUG("pathModel: %s\r\n",pathModel);

    memset(pathType, 0, sizeof(pathType));
    snprintf(pathType, sizeof(pathType), "D:/%s.type", sub_func_table[cur_menu_func_id].name);
    SYSLOG_DEBUG("pathType: %s\r\n",pathType);

    uint32_t index = rand() % 3;
    const char contact[3][2][40] =
    {
        {"中国移动", "10086"},
        {"中国电信", "10000"},
        {"中国联通", "10010"}
    };
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbAddItem(pathModel, pathType, contact[index]);
#endif
}

int32_t del_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t del_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("del_contact_proc \r\n");

    char pathModel[40] = {0};
    char pathType[40]  = {0};

    memset(pathModel, 0, sizeof(pathModel));
    snprintf(pathModel, sizeof(pathModel), "D:/%s.json", sub_func_table[cur_menu_func_id].name);
    SYSLOG_DEBUG("pathModel: %s\r\n",pathModel);

    memset(pathType, 0, sizeof(pathType));
    snprintf(pathType, sizeof(pathType), "D:/%s.type", sub_func_table[cur_menu_func_id].name);
    SYSLOG_DEBUG("pathType: %s\r\n",pathType);

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    jsonDbDeleteItem(pathModel, pathType, contact_cur_item);
#endif

    if (--contact_total_num == 0)
    {
        contact_cur_item = 0;
    }
    else if (--contact_cur_item < 0)
    {
        contact_cur_item = contact_total_num - 1;
    }
}

int32_t move_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t move_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("move_contact_proc \r\n");
}

int32_t locate_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t locate_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{}

int32_t import_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t import_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{}

int32_t export_contact_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t export_contact_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{}

int32_t querycapacity_init(AppInfoT *appInfo, uint32_t reserved1, uint32_t reserved2, uint32_t syscallTable)
{
    SYSLOG_DEBUG("\r\n");
    appInfo->initStatus = 1;

    return 0;
}

int32_t querycapacity_proc(AppInfoT *appInfo,AppMsgT *msg, uint32_t reserved2, uint32_t syscallTable)
{}
