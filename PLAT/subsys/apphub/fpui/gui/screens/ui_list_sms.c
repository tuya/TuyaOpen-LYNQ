/**
 * @file ui_list_sms.c
 * @brief 短信编辑界面实现，包含标题、联系人输入、短信内容编辑和底部按钮
 * 
 * 功能特性：
 * 1. 支持中英文输入法切换显示
 * 2. 双输入框焦点管理（联系人/内容）
 * 3. 底部功能按钮动态配置
 * 4. 界面隐藏/显示状态管理
 */

#include "ui.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include DEBUG_LOG_HEADER_FILE

/* 日志追踪宏 */
#define EPAT_TRACE(subId, argLen, format, ...) \
    ECOMM_TRACE(UNILOG_TEST, subId, P_VALUE, argLen, format, ##__VA_ARGS__)

#define SMS_LENGTH_MAX      240  // 短信内容最大长度

PLAT_FPSRAM_ZI_CUST char sms_context[SMS_LENGTH_MAX+1];      // 短信内容缓存
/**
 * @struct fpui_sms_obj_t
 * @brief UI对象容器结构体
 * 
 * @var screen    - 主屏幕容器对象
 * @var title     - 标题栏容器
 * @var title_txt - 标题文本标签
 * @var title_ime - 输入法状态指示
 * @var contact   - 联系人输入框
 * @var text_area - 短信内容编辑框
 * @var input_normal  - 输入框常规样式
 * @var input_focused - 输入框聚焦样式
 */
typedef struct {
    lv_obj_t *screen;
    lv_obj_t *title;
    lv_obj_t *title_txt;
    lv_obj_t *title_ime;
    lv_obj_t *contact;
    lv_obj_t *text_area;
    lv_style_t input_normal;
    lv_style_t input_focused;
} fpui_sms_obj_t;

PLAT_FPSRAM_ZI_CUST fpui_sms_obj_t fpui_sms; // UI对象实例
PLAT_FPSRAM_ZI_CUST fpui_sms_raw_t sms_data; // 数据实例
/**
 * @brief 初始化输入框样式
 * @param ui UI对象指针
 */
static void init_sms_styles(fpui_sms_obj_t *ui)
{
    // 常规状态样式
    lv_style_init(&ui->input_normal);
    lv_style_set_bg_color(&ui->input_normal, lv_color_black());
    lv_style_set_text_color(&ui->input_normal, lv_color_white());
    lv_style_set_border_width(&ui->input_normal, 2);
    lv_style_set_border_color(&ui->input_normal, lv_color_white());
    lv_style_set_bg_opa(&ui->input_normal, LV_OPA_COVER);

    // 聚焦状态样式
    lv_style_init(&ui->input_focused);
    lv_style_set_bg_color(&ui->input_focused, lv_color_white());
    lv_style_set_text_color(&ui->input_focused, lv_color_black());
    lv_style_set_border_width(&ui->input_focused, 2);
    lv_style_set_border_color(&ui->input_focused, lv_color_make(0xff, 0x0, 0x0));
    lv_style_set_bg_opa(&ui->input_focused, LV_OPA_COVER);
}

/**
 * @brief 创建或更新短信编辑界面
 * @param data 输入数据，NULL表示隐藏界面
 * @return 全局UI对象指针
 * 
 * 功能流程：
 * 1. 数据校验与样式初始化
 * 2. 屏幕容器创建
 * 3. 标题栏构建
 * 4. 输入框创建与状态管理
 * 5. 底部按钮配置
 */
fpui_sms_obj_t* ui_list_sms_screen(fpui_sms_raw_t *data)
{
    /* 隐藏界面处理 */
    if(data == NULL) {
        if(lv_obj_is_valid(fpui_sms.screen)) {
            lv_obj_add_flag(fpui_sms.screen, LV_OBJ_FLAG_HIDDEN);
        }
        return &fpui_sms;
    }

    /* 首次使用时初始化输入框样式 */
    if(fpui_sms.input_normal.prop_cnt == 0){
        init_sms_styles(&fpui_sms);
    }

    /* 屏幕对象管理 */
    if(lv_obj_is_valid(fpui_sms.screen)) {
        lv_obj_del_async(fpui_sms.screen); // 异步删除旧对象
        fpui_sms.screen = NULL;
    }

    /* 创建新屏幕 */
    fpui_sms.screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(fpui_sms.screen); // 清除默认样式
    lv_obj_set_size(fpui_sms.screen, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(fpui_sms.screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(fpui_sms.screen, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(fpui_sms.screen, lv_color_black(), 0);

    /* 构建标题栏 ------------------------------------------------------*/
    fpui_sms.title = lv_obj_create(fpui_sms.screen);
    lv_obj_set_size(fpui_sms.title, LV_PCT(100), 40);
    lv_obj_set_style_bg_opa(fpui_sms.title, LV_OPA_TRANSP, 0);

    // 标题文本
    fpui_sms.title_txt = lv_label_create(fpui_sms.title);
    lv_obj_set_size(fpui_sms.title_txt, LV_PCT(80), 40);
    lv_obj_set_style_bg_opa(fpui_sms.title_txt, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(fpui_sms.title_txt, lv_color_white(), 0);
    lv_obj_set_style_text_align(fpui_sms.title_txt, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(fpui_sms.title_txt, data->title);

    // 输入法指示器
    fpui_sms.title_ime = lv_label_create(fpui_sms.title);
    lv_obj_set_size(fpui_sms.title_ime, LV_PCT(20), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(fpui_sms.title_ime, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_font(fpui_sms.title_ime, pSystem16, 0);
    lv_obj_set_style_text_color(fpui_sms.title_ime, lv_color_make(0xff, 0xff, 0x0), 0);
    lv_obj_align_to(fpui_sms.title_ime, fpui_sms.title_txt, LV_ALIGN_OUT_RIGHT_MID, 0, 4);
    lv_obj_set_style_text_align(fpui_sms.title_ime, LV_TEXT_ALIGN_RIGHT, 0);
    // 设置输入法显示文本
    const char *ime_text[] = {"123", "abc", "pinyin"};
    lv_label_set_text(fpui_sms.title_ime, (data->imType <= 2) ? ime_text[data->imType] : "");

    /* 联系人输入框 ----------------------------------------------------*/
    fpui_sms.contact = lv_textarea_create(fpui_sms.screen);
    lv_obj_set_size(fpui_sms.contact, LV_PCT(100), 40);
    lv_obj_align_to(fpui_sms.contact, fpui_sms.title, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
    lv_textarea_set_text(fpui_sms.contact, data->contact);
    
    // 样式状态管理
    lv_obj_add_style(fpui_sms.contact, &fpui_sms.input_normal, 0);
    lv_obj_add_style(fpui_sms.contact, &fpui_sms.input_focused, LV_STATE_FOCUSED);
    lv_obj_add_state(fpui_sms.contact, (data->select == 1) ? LV_STATE_FOCUSED : LV_STATE_DEFAULT);

    /* 短信内容编辑框 --------------------------------------------------*/
    fpui_sms.text_area = lv_textarea_create(fpui_sms.screen);
    lv_obj_set_size(fpui_sms.text_area, LV_PCT(100), 200);
    lv_obj_align_to(fpui_sms.text_area, fpui_sms.contact, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    lv_textarea_set_text(fpui_sms.text_area, data->context);
    
    // 样式状态管理
    lv_obj_add_style(fpui_sms.text_area, &fpui_sms.input_normal, 0);
    lv_obj_add_style(fpui_sms.text_area, &fpui_sms.input_focused, LV_STATE_FOCUSED);
    lv_obj_add_state(fpui_sms.text_area, (data->select == 2) ? LV_STATE_FOCUSED : LV_STATE_DEFAULT);

    /* 底部按钮配置 ----------------------------------------------------*/
    ui_btn_set(data->bottomBtn, lv_color_white(), lv_color_black());
    
    // 加载屏幕
    lv_disp_load_scr(fpui_sms.screen);
    return &fpui_sms;
}
/**
 * @brief 配置短信界面数据
 * @param data 输入数据
 * @return 处理后的数据指针
 * 
 * 数据流向：
 * 输入数据 -> 临时存储 -> 消息队列 -> 界面刷新
 */
fpui_sms_raw_t *ui_sms_set(fpui_sms_raw_t *data)
{
    GuiMsgT msgPtr = {0};
    if(data == NULL) {
        memset(&sms_data, 0, sizeof(fpui_sms_raw_t));
        if(lv_obj_is_valid(fpui_sms.screen)) {
            lv_obj_add_flag(fpui_sms.screen, LV_OBJ_FLAG_HIDDEN);
        }
        return &sms_data;
    }
    else if(data == &sms_data) {
        msgPtr.ui_data = data;
    }
    else {
        /* 数据有效性检查 */
        if(data->select == 0) { // 特殊隐藏指令
            if(lv_obj_is_valid(fpui_sms.screen)) {
                lv_obj_add_flag(fpui_sms.screen, LV_OBJ_FLAG_HIDDEN);
            }
            return &sms_data;
        }
        /* 数据更新 */
        sms_data.select = (data->select < 3) ? data->select : sms_data.select;
        sms_data.imType = (data->imType < 3) ? data->imType : sms_data.imType;
        if(data->title != NULL){
            memset(sms_data.title, 0, MAX_TITLE_LENGTH);
            strncpy(sms_data.title,data->title,MAX_TITLE_LENGTH-1);
        }
        // 联系人信息拷贝
        if(data->contact != NULL){
            strncpy(sms_data.contact, data->contact, LABEL_LENGTH_MAX-1);
            sms_data.contact[LABEL_LENGTH_MAX-1] = '\0'; // 确保终止符
        }

        // 短信内容拷贝
        if(data->context != NULL){
            strncpy(sms_context, data->context, SMS_LENGTH_MAX);
            sms_context[SMS_LENGTH_MAX] = '\0';
            sms_data.context = sms_context;
        }

        // 底部按钮更新
        if(data->bottomBtn != NULL) {
            for(uint8_t i=0; i<3; i++) {
                if(data->bottomBtn[i] != NULL) {
                    strncpy(sms_data.bottomBtn[i], data->bottomBtn[i], BTN_LENGTH_MAX-1);
                    sms_data.bottomBtn[i][BTN_LENGTH_MAX-1] = '\0';
                }
            }
        }
        msgPtr.ui_data = &sms_data; // 使用内部数据副本
    }
    /* 发送GUI更新消息 */
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_list_sms_screen;
    osDelay(1); // 确保消息队列处理
    guiSendMsg(&msgPtr);
    osDelay(1);
    return msgPtr.ui_data;
}

/**
 * @brief 短信界面测试用例
 * @param case_type 测试类型
 */
void ui_sms_test(ui_test_case_t case_type)
{
    static char _sms_context[] = "短信测试超长文本_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX123456789";
    // static const char DEFAULT_BTNS[3][BTN_LENGTH_MAX] = {"确认", "选择", "返回"};
    
    static fpui_sms_raw_t sms_data = {
        .select = 1,
        .total = 70,
        .imType = 2,
        .title = "短信",
        .contact = "10086",
        .context = _sms_context,
        .bottomBtn = {"确认", "选择", "返回"}
    };
    switch(case_type) {
        case TEST_CASE_NORMAL:
            ui_sms_set(&sms_data);
            osDelay(1000);
            sms_data.select = 2; // 切换焦点到内容框
            ui_sms_set(&sms_data);
            break;
        default:
            break;
    }
}