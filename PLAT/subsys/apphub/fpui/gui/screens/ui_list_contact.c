
#include "ui.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include DEBUG_LOG_HEADER_FILE
// #define EPAT_TRACE(subId, argLen, format,  ...) ECOMM_TRACE(UNILOG_TEST, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 
#define EPAT_TRACE(subId, argLen, format,  ...) 

static lv_obj_t * _ui_list_contact = NULL;
static lv_obj_t * _ui_list_contact_title[4] = {NULL};
static lv_obj_t * _ui_list_contact_label[4] = {NULL};
static lv_obj_t * _ui_list_contact_count[4] = {NULL};
static lv_obj_t * _ui_list_contact_input[3] = {NULL};
PLAT_FPSRAM_ZI_CUST fpui_contact_t _ui_list_contact_data;
static lv_style_t input_style_normal;
static lv_style_t input_style_focused;
static int PADDING = 4;       // 边距
static int LABEL_WIDTH_PCT = 70; // 主标签宽度占比
// static int INDEX_WIDTH_PCT = 30; // 索引宽度占比
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_list_contact_create(fpui_contact_t *data) 
{
    if(input_style_normal.prop_cnt == 0){
        // 输入框默认
        lv_style_init(&input_style_normal);
        lv_style_set_bg_color(&input_style_normal, lv_color_black());
        lv_style_set_text_color(&input_style_normal, lv_color_white());
        lv_style_set_border_width(&input_style_normal, 2);
        lv_style_set_border_color(&input_style_normal, lv_color_white());
        lv_style_set_bg_opa(&input_style_normal, LV_OPA_COVER);
        // 输入框聚焦
        lv_style_init(&input_style_focused);
        lv_style_set_bg_color(&input_style_focused, lv_color_white());
        lv_style_set_text_color(&input_style_focused, lv_color_black());
        lv_style_set_border_width(&input_style_focused, 2);
        lv_style_set_border_color(&input_style_focused, lv_color_make(0xff, 0x0, 0x0));
        lv_style_set_bg_opa(&input_style_focused, LV_OPA_COVER);
    }
    if(data == NULL){
        if(_ui_list_contact != NULL && lv_obj_is_valid(_ui_list_contact)){
            lv_obj_add_flag(_ui_list_contact, LV_OBJ_FLAG_HIDDEN);
        }
        return NULL;
    }

    if(!lv_obj_is_valid(data->screen) && data->screen != NULL){
        EPAT_TRACE(ui_list_contact_invalid, 1, "screen=0x%x", data->screen);
        return NULL;
    }

    if(_ui_list_contact == NULL){
        _ui_list_contact = lv_obj_create(data->screen);
        lv_obj_remove_style_all(_ui_list_contact);
        lv_obj_set_size(_ui_list_contact, LV_PCT(100), LV_PCT(100));
        
        lv_obj_set_flex_flow(_ui_list_contact, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(_ui_list_contact, 2, 0); 
        lv_obj_set_style_pad_column(_ui_list_contact, 0, 0);
        lv_obj_set_style_flex_main_place(_ui_list_contact, LV_FLEX_ALIGN_START, 0);

        lv_obj_clear_flag(_ui_list_contact, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(_ui_list_contact, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(_ui_list_contact, LV_OPA_COVER, LV_PART_MAIN);
    }
    lv_obj_clear_flag(_ui_list_contact, LV_OBJ_FLAG_HIDDEN);

    if(_ui_list_contact_title[0] == NULL){
        _ui_list_contact_title[0] = lv_obj_create(_ui_list_contact);
        lv_obj_set_size(_ui_list_contact_title[0], LV_PCT(100), 36);
        lv_obj_set_style_bg_color(_ui_list_contact_title[0], lv_color_black(), 0);
        lv_obj_set_flex_flow(_ui_list_contact_title[0], LV_FLEX_FLOW_ROW);
        lv_obj_set_style_flex_cross_place(_ui_list_contact_title[0], LV_FLEX_ALIGN_END, 0);

        _ui_list_contact_label[0] = lv_label_create(_ui_list_contact_title[0]);
        lv_obj_set_flex_grow(_ui_list_contact_label[0], 8);
        lv_obj_set_style_text_color(_ui_list_contact_label[0], lv_color_white(), 0);
        lv_obj_set_style_text_align(_ui_list_contact_label[0], LV_TEXT_ALIGN_RIGHT, 0);
        
        _ui_list_contact_count[0] = lv_label_create(_ui_list_contact_title[0]);
        lv_obj_set_style_text_font(_ui_list_contact_count[0], pSystem16, 0);
        lv_obj_set_style_text_color(_ui_list_contact_count[0], lv_color_make(0xff, 0xff, 0x0), 0);
        lv_obj_set_flex_grow(_ui_list_contact_count[0], 2); 
        lv_obj_set_style_text_align(_ui_list_contact_count[0], LV_TEXT_ALIGN_RIGHT, 0);
        // lv_obj_set_style_border_width(_ui_list_contact_count[0], 1, 0);
        // lv_obj_set_style_border_color(_ui_list_contact_count[0], lv_color_make(0, 0xff, 0), 0);
    }

    for(int i=0; i<3; i++){
        if(_ui_list_contact_title[i+1] == NULL){
            _ui_list_contact_title[i+1] = lv_obj_create(_ui_list_contact);
            lv_obj_set_size(_ui_list_contact_title[i+1], LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(_ui_list_contact_title[i+1], LV_FLEX_FLOW_ROW);
            lv_obj_set_style_flex_cross_place(_ui_list_contact_title[i+1], LV_FLEX_ALIGN_END, 0);
            lv_obj_set_style_bg_color(_ui_list_contact_title[i+1], lv_color_black(), 0);
            lv_obj_add_flag(_ui_list_contact_title[i+1], LV_OBJ_FLAG_HIDDEN);
            
            _ui_list_contact_label[i+1] = lv_label_create(_ui_list_contact_title[i+1]);
            lv_obj_set_flex_grow(_ui_list_contact_label[i+1], 6);
            lv_obj_set_style_text_color(_ui_list_contact_label[i+1], lv_color_white(), 0);
            
            _ui_list_contact_count[i+1] = lv_label_create(_ui_list_contact_title[i+1]);
            lv_obj_set_style_text_font(_ui_list_contact_count[i+1], pSystem16, 0);
            lv_obj_set_flex_grow(_ui_list_contact_count[i+1], 4);
            lv_obj_set_style_text_align(_ui_list_contact_count[i+1], LV_TEXT_ALIGN_RIGHT, 0);
            lv_obj_set_style_text_color(_ui_list_contact_count[i+1], lv_color_make(0x0, 0x0, 0xff), 0);
            // lv_obj_set_style_border_width(_ui_list_contact_count[i+1], 1, 0);
            // lv_obj_set_style_border_color(_ui_list_contact_count[i+1], lv_color_make(0, 0xff, 0), 0);
        }

        if(_ui_list_contact_input[i] == NULL){
            _ui_list_contact_input[i] = lv_textarea_create(_ui_list_contact);
            lv_obj_set_size(_ui_list_contact_input[i], LV_PCT(100), 40);
            lv_obj_add_style(_ui_list_contact_input[i], &input_style_normal, 0);
            lv_obj_add_style(_ui_list_contact_input[i], &input_style_focused, LV_STATE_FOCUSED);
            lv_textarea_set_one_line(_ui_list_contact_input[i], true);
            lv_obj_add_flag(_ui_list_contact_input[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    for (uint8_t i = 0; i < 4; i++) {
        const char* title = (data->title[i*2][0] != '\0') ? data->title[i*2] : NULL;
        const char* index = (data->title[i*2+1][0] != '\0') ? data->title[i*2+1] : NULL;
        if(title != NULL){
            EPAT_TRACE(ui_list_contact_title_check, 3, "title[%d]=%s,index=%s",i, title, index);
            lv_obj_clear_flag(_ui_list_contact_title[i], LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(_ui_list_contact_label[i], title);
            if(index != NULL) lv_label_set_text(_ui_list_contact_count[i], index);
            if(i) {
                lv_obj_clear_flag(_ui_list_contact_input[i-1], LV_OBJ_FLAG_HIDDEN);
                if(data->context[i-1] != NULL){
                    const char* txt = (data->context[i-1][0] != '\0') ? data->context[i-1] : "";
                    lv_textarea_set_text(_ui_list_contact_input[i-1], txt);
                    lv_obj_set_style_text_color(_ui_list_contact_input[i-1], lv_color_white(), LV_PART_CURSOR|LV_STATE_FOCUSED); 
                }
                if(data->select == i){    // 设置焦点
                    lv_obj_add_state(_ui_list_contact_input[i-1], LV_STATE_FOCUSED);
                }
                else {
                    lv_obj_clear_state(_ui_list_contact_input[i-1], LV_STATE_FOCUSED);
                }
            }
        }
        else {
            lv_obj_add_flag(_ui_list_contact_title[i], LV_OBJ_FLAG_HIDDEN);
            if(i) lv_obj_add_flag(_ui_list_contact_input[i-1], LV_OBJ_FLAG_HIDDEN);
        }
    }
    ui_btn_set(data->bottomBtn, lv_color_white(), lv_color_black());
    // lv_obj_update_layout(_ui_list_contact); // 强制刷新布局
    lv_disp_load_scr(_ui_list_contact);
    return _ui_list_contact;
}
/**
  \fn
  \brief
  \return
*/
void *ui_contact_set(uint8_t select, uint8_t offset,
                    char    *context1,  // IN - 姓名输入框内容（传NULL不更新原值）
                    char    *context2,  // IN - 电话输入框内容（传NULL不更新原值）
                    char    *context3,  // IN - 单位输入框内容（传NULL不更新原值）
                    char (*title)[MAX_TITLE_LENGTH], char (*bottomBtn)[BTN_LENGTH_MAX])
{
    if(_ui_list_contact == NULL){
        memset(&_ui_list_contact_data, 0, sizeof(fpui_contact_t));
    }
    if(select == 0){
        if(lv_obj_is_valid(_ui_list_contact)){
            lv_obj_add_flag(_ui_list_contact, LV_OBJ_FLAG_HIDDEN);
            return &_ui_list_contact_data;
        }
        else return NULL;
    }
    else if(select > 3){
        return NULL;
    }
    EPAT_TRACE(ui_contact_set, 4, "select %d,%s,%s,%s",select, context1, context2, context3);
    if(context1 != NULL && strlen(context1)<LABEL_LENGTH_MAX){
        memset(_ui_list_contact_data.context[0], 0, LABEL_LENGTH_MAX);
        strncpy(_ui_list_contact_data.context[0], context1, (LABEL_LENGTH_MAX-1));
    }
    if(context2 != NULL && strlen(context2)<LABEL_LENGTH_MAX){
        memset(_ui_list_contact_data.context[1], 0, LABEL_LENGTH_MAX);
        strncpy(_ui_list_contact_data.context[1], context2, (LABEL_LENGTH_MAX-1));
    }
    if(context3 != NULL && strlen(context3)<LABEL_LENGTH_MAX){
        memset(_ui_list_contact_data.context[2], 0, LABEL_LENGTH_MAX);
        strncpy(_ui_list_contact_data.context[2], context3, (LABEL_LENGTH_MAX-1));
    }
    if(title != NULL){
        for (uint8_t i = 0; i < 8; i++) {
            memset(_ui_list_contact_data.title[i], 0, MAX_TITLE_LENGTH);
            strncpy(_ui_list_contact_data.title[i], title[i], (MAX_TITLE_LENGTH-1)); 
        } 
    }
    if(bottomBtn != NULL){
        for (uint8_t i = 0; i < 3; i++) {
            if(bottomBtn[i] != NULL && strlen(bottomBtn[i])<BTN_LENGTH_MAX) {
                memset(_ui_list_contact_data.bottomBtn[i], 0, BTN_LENGTH_MAX);
                strncpy(_ui_list_contact_data.bottomBtn[i], bottomBtn[i], (BTN_LENGTH_MAX-1));
            }
        }
    }
    _ui_list_contact_data.offset = offset;
    _ui_list_contact_data.select = select;
    _ui_list_contact_data.object = _ui_list_contact;
    _ui_list_contact_data.screen = NULL;
    GuiMsgT msgPtr;
    msgPtr.ui_data = &_ui_list_contact_data;
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_list_contact_create;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(1);
    return msgPtr.ui_data;
}
/**
 * @brief 联系人界面测试用例
 * 
 * @param case_type 测试用例类型
 * 
 * @note 测试用例覆盖场景：
 * - 正常显示
 * - 关闭界面
 * - 边界值测试
 * - 异常参数测试
 * - 输入验证测试
 */
void ui_contact_test(ui_test_case_t case_type)
{
    static char base_title[8][MAX_TITLE_LENGTH] = {
        "新联系人", "30",       // 标题行
        "姓名",    "pinyin",    // 输入框1
        "移动电话", "123",      // 输入框2
        "单位电话", "123"       // 输入框3
    };
    static char input_context[3][LABEL_LENGTH_MAX] = {
        "营业厅", "10086", "中国移动"
    };
    static char base_bottomBtn[3][BTN_LENGTH_MAX] = {
        "选项", "", "清除"
    };

    switch(case_type) {
        case TEST_CASE_NORMAL:
            ui_contact_set(1, 0, 
                input_context[0], 
                input_context[1], 
                input_context[2], 
                base_title, 
                base_bottomBtn
            );
            break;
            
        /* 关闭界面测试 */
        case TEST_CASE_UI_CLOSE:
            ui_contact_set(0, 0, NULL, NULL, NULL, NULL, NULL);
            break;
            
        /* 边界值测试 */
        case TEST_CASE_BOUNDARY_VALUE:
            ui_contact_set(1, 0, "输入姓名", "13800138000", "输入公司", base_title, base_bottomBtn);
            osDelay(1000);
            // 测试选中越界值
            ui_contact_set(4, 0, 
                "越界测试1", "越界测试2", "越界测试3", 
                base_title, 
                base_bottomBtn
            );
            // 测试选中姓名输入框
            osDelay(1000);
            ui_contact_set(1, 0, 
                "[选中姓名]", "未输入", "未输入", 
                base_title, 
                base_bottomBtn
            );
            // 测试选中电话输入框
            osDelay(1000);
            ui_contact_set(2, 0, 
                "张三", "[选中电话]", "未输入", 
                base_title, 
                base_bottomBtn
            );
            // 测试选中单位输入框
            osDelay(1000);
            ui_contact_set(3, 0, 
                "李四", "13800138000", "[选中单位]", 
                base_title, 
                base_bottomBtn
            );
            // 测试双输入框
            osDelay(1000);
            memset(base_title[6], 0, MAX_TITLE_LENGTH);
            memset(base_title[7], 0, MAX_TITLE_LENGTH);
            ui_contact_set(2, 0, 
                "双输入框", "双输入框", "双输入框", 
                base_title, 
                base_bottomBtn
            );
            // 测试单输入框
            osDelay(1000);
            memset(base_title[4], 0, MAX_TITLE_LENGTH);
            memset(base_title[5], 0, MAX_TITLE_LENGTH);
            ui_contact_set(1, 0, 
                "单输入框", "单输入框", "单输入框", 
                base_title, 
                base_bottomBtn
            );
            break;
            
        /* 异常参数测试 */
        case TEST_CASE_INVALID_PARAM:
            // 测试空指针参数
            ui_contact_set(1, 0, NULL, NULL, NULL, NULL, NULL);
            
#if 0 // 此处编译报错，由于是测试代码，遂先关闭，需要此测试项时再打开
            // 测试超长字符串
            ui_contact_set(1, 0, 
                "超长姓名_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                "超长电话_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                "超长单位_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                (char[][MAX_TITLE_LENGTH]){
                    "超长标题1_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", "计数1",
                    "超长标题2_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", "计数2",
                    "超长标题3_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", "计数3",
                    "超长标题4_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", "计数4"
                },
                (char[][BTN_LENGTH_MAX]){
                    "超长左按钮_XXXXXXXXXXXXXXXXX",
                    "超长中按钮_XXXXXXXXXXXXXXXXX",
                    "超长右按钮_XXXXXXXXXXXXXXXXX"
                }
            );
#endif
            break;
        default:
            break;
    }
}


/**
 * @brief 联系人列表界面管理实现
 * 
 * 包含搜索界面创建、配置和测试功能
 */

#define SEARCH_LIST_ITEM_HEIGHT   40   // 列表项高度(像素)
#define SEARCH_INPUT_HEIGHT       40   // 输入框高度(像素)
#define SEARCH_TITLE_HEIGHT       40   // 标题栏高度(像素)

PLAT_FPSRAM_ZI_CUST fpui_search_t _ui_list_search_data;

static lv_obj_t* _ui_list_search = NULL;              // 主容器对象
static lv_obj_t* _ui_list_search_input = NULL;        // 输入框组件
static lv_obj_t* _ui_list_search_title = NULL;        // 标题栏容器
static lv_obj_t* _ui_list_search_list[4] = {NULL};    // 列表项容器数组
static lv_obj_t* _ui_list_search_label[5] = {NULL};   // 文本标签数组
static lv_obj_t* _ui_list_search_index[5] = {NULL};   // 状态指示器数组

/**
 * @brief 创建搜索界面组件
 * 
 * @param data 界面配置数据指针
 * @return lv_obj_t* 成功返回主容器对象，失败返回NULL
 * 
 * @note 使用静态样式对象避免重复初始化
 */
lv_obj_t* ui_list_search_create(fpui_search_t* data) 
{
    if(data == NULL) {
        if(lv_obj_is_valid(_ui_list_search)) {
            lv_obj_add_flag(_ui_list_search, LV_OBJ_FLAG_HIDDEN);
        }
        return NULL;
    }

    // 清理旧对象
    if(lv_obj_is_valid(_ui_list_search)) {
        lv_obj_clean(_ui_list_search);
        lv_obj_del_async(_ui_list_search);
        _ui_list_search = NULL;
    }

    if(input_style_normal.prop_cnt == 0){
        // 输入框默认
        lv_style_init(&input_style_normal);
        lv_style_set_bg_color(&input_style_normal, lv_color_black());
        lv_style_set_text_color(&input_style_normal, lv_color_white());
        lv_style_set_border_width(&input_style_normal, 2);
        lv_style_set_border_color(&input_style_normal, lv_color_white());
        lv_style_set_bg_opa(&input_style_normal, LV_OPA_COVER);
        // 输入框聚焦
        lv_style_init(&input_style_focused);
        lv_style_set_bg_color(&input_style_focused, lv_color_white());
        lv_style_set_text_color(&input_style_focused, lv_color_black());
        lv_style_set_border_width(&input_style_focused, 2);
        lv_style_set_border_color(&input_style_focused, lv_color_make(0xff, 0x0, 0x0));
        lv_style_set_bg_opa(&input_style_focused, LV_OPA_COVER);
    }
    /* 主容器创建 */
    _ui_list_search = lv_obj_create(NULL);
    lv_obj_remove_style_all(_ui_list_search);
    lv_obj_set_size(_ui_list_search, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(_ui_list_search, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(_ui_list_search, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(_ui_list_search, LV_OPA_COVER, 0);

    /* 标题部分 */
    _ui_list_search_title = lv_obj_create(_ui_list_search);
    lv_obj_set_size(_ui_list_search_title, lv_pct(100), SEARCH_TITLE_HEIGHT);
    lv_obj_set_pos(_ui_list_search_title, 0, 0);
    lv_obj_set_style_bg_opa(_ui_list_search_title, LV_OPA_TRANSP, 0);

    // 标题标签
    _ui_list_search_label[0] = lv_label_create(_ui_list_search_title);
    lv_obj_set_size(_ui_list_search_label[0], LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(_ui_list_search_label[0], LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_set_style_text_color(_ui_list_search_label[0], lv_color_white(), 0);
    
    // 标题索引
    _ui_list_search_index[0] = lv_label_create(_ui_list_search_title);
    lv_obj_set_size(_ui_list_search_index[0], LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(_ui_list_search_index[0], LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    // lv_obj_align_to(_ui_list_search_index[0], _ui_list_search_label[0], LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
    lv_obj_set_style_text_font(_ui_list_search_index[0], pSystem16, 0);
    lv_obj_set_style_text_color(_ui_list_search_index[0], lv_color_make(0xff, 0xff, 0x0), 0);
    lv_obj_set_style_text_align(_ui_list_search_index[0], LV_TEXT_ALIGN_RIGHT, 0);

    /* 输入框 */
    _ui_list_search_input = lv_textarea_create(_ui_list_search);
    lv_obj_set_size(_ui_list_search_input, lv_pct(100), SEARCH_INPUT_HEIGHT);
    lv_obj_align_to(_ui_list_search_input, _ui_list_search_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    lv_obj_add_style(_ui_list_search_input, &input_style_normal, 0);
    lv_obj_add_style(_ui_list_search_input, &input_style_focused, LV_STATE_FOCUSED);
    /* 列表项 */
    for(int i = 0; i < 4; i++) {
        // 列表项容器
        _ui_list_search_list[i] = lv_obj_create(_ui_list_search);
        lv_obj_set_size(_ui_list_search_list[i], lv_pct(100), SEARCH_LIST_ITEM_HEIGHT);
        
        // 主标签（左对齐）
        _ui_list_search_label[i+1] = lv_label_create(_ui_list_search_list[i]);
        lv_obj_set_size(_ui_list_search_label[i+1], lv_pct(LABEL_WIDTH_PCT), LV_SIZE_CONTENT);
        lv_obj_align(_ui_list_search_label[i+1], LV_ALIGN_LEFT_MID, PADDING, 0);
        
        // 索引标签（相对主标签右侧对齐）
        _ui_list_search_index[i+1] = lv_label_create(_ui_list_search_list[i]);
        lv_obj_set_style_text_font(_ui_list_search_index[i+1], pSystem16, 0);
        lv_obj_set_size(_ui_list_search_index[i+1], LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_align(_ui_list_search_index[i+1], LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        // lv_obj_align_to(_ui_list_search_index[i+1], _ui_list_search_label[i+1], LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
        lv_obj_set_style_text_align(_ui_list_search_index[i+1], LV_TEXT_ALIGN_RIGHT, 0);
        
        // 垂直布局
        if(i == 0) {
            lv_obj_align_to(_ui_list_search_list[i], _ui_list_search_input, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
        } else {
            lv_obj_align_to(_ui_list_search_list[i], _ui_list_search_list[i-1], LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
        }
    }

    /* 数据填充和样式更新 */
    if(data->title != NULL) {
        lv_obj_clear_flag(_ui_list_search_title, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(_ui_list_search_label[0], data->title);
        switch(data->show_type[4]) {
            case 0: lv_label_set_text(_ui_list_search_index[0], "123");    break;
            case 1: lv_label_set_text(_ui_list_search_index[0], "abc");    break;
            case 2: lv_label_set_text(_ui_list_search_index[0], "pinyin"); break;
            default: lv_label_set_text(_ui_list_search_index[0], "");      break;
        }
    } else {
        lv_obj_add_flag(_ui_list_search_title, LV_OBJ_FLAG_HIDDEN);
    }

    if(data->input != NULL) {
        lv_obj_clear_flag(_ui_list_search_input, LV_OBJ_FLAG_HIDDEN);
        lv_textarea_set_text(_ui_list_search_input, data->input);
        if(data->select == (5)){
            lv_obj_set_style_border_width(_ui_list_search_input, 1, 0);
            lv_obj_set_style_border_color(_ui_list_search_input, lv_color_make(0xff, 0, 0), 0);
            lv_obj_set_style_bg_color(_ui_list_search_input, lv_color_white(), 0);
            lv_obj_set_style_text_color(_ui_list_search_input, lv_color_black(), 0);
        }
        else {
            lv_obj_set_style_border_width(_ui_list_search_input, 1, 0);
            lv_obj_set_style_border_color(_ui_list_search_input, lv_color_white(), 0);
            lv_obj_set_style_bg_color(_ui_list_search_input, lv_color_black(), 0);
            lv_obj_set_style_text_color(_ui_list_search_input, lv_color_white(), 0);
        }
    } 
    else {
        lv_obj_add_flag(_ui_list_search_input, LV_OBJ_FLAG_HIDDEN);
    }

    for (uint8_t i = 0; i < 4; i++) {
        lv_obj_add_state(_ui_list_search_list[i],((data->select==(i+1))?LV_STATE_FOCUSED:0));
        if(data->context[i] != NULL) {
            lv_obj_clear_flag(_ui_list_search_list[i], LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(_ui_list_search_label[i+1], data->context[i]);
            switch(data->show_type[i]) {
                case 0: lv_label_set_text(_ui_list_search_index[i+1], "local"); break;
                case 1: lv_label_set_text(_ui_list_search_index[i+1], "sim1");  break;
                case 2: lv_label_set_text(_ui_list_search_index[i+1], "sim2");  break;
                default: lv_label_set_text(_ui_list_search_index[i+1], "");     break;
            }
            if(data->select == (i+1)){
                lv_obj_set_style_bg_color(_ui_list_search_list[i], lv_color_make(0xff, 0x0, 0x0), 0);
                lv_obj_set_style_text_color(_ui_list_search_label[i+1], lv_color_black(), 0);
                lv_obj_set_style_text_color(_ui_list_search_index[i+1], lv_color_black(), 0); 
            }
            else {
                lv_obj_set_style_bg_color(_ui_list_search_list[i], lv_color_black(), 0);
                lv_obj_set_style_text_color(_ui_list_search_label[i+1], lv_color_white(), 0);
                lv_obj_set_style_text_color(_ui_list_search_index[i+1], lv_color_white(), 0); 
            }
        }
        else {
            lv_obj_add_flag(_ui_list_search_list[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    ui_btn_set(data->bottomBtn, lv_color_white(), lv_color_black());

    lv_disp_load_scr(_ui_list_search);
    return _ui_list_search;
}

/**
 * @brief 更新搜索界面配置
 * 
 * @param select 当前选中项（0-关闭界面，1-3列表项，4-输入框）
 * @param offset 输入框光标偏移量
 * @param title 页标题文本
 * @param input 输入框内容
 * @param show_type 显示配置[存储区域1,存储区域2,存储区域3,输入法类型]
 * @param context 列表项文本数组
 * @param bottomBtn 底部按钮文本数组
 * @return void* 返回界面数据指针
 */
void* ui_search_set(uint8_t select, uint8_t offset, char* title, char* input, 
                    uint8_t (*show_type)[5], char (*context)[LABEL_LENGTH_MAX], 
                    char (*bottomBtn)[BTN_LENGTH_MAX])
{
    // if(_ui_list_search == NULL) {
    //     memset(&_ui_list_search_data, 0, sizeof(fpui_search_t));
    // }
    if(select == 0) {
        if(lv_obj_is_valid(_ui_list_search)) {
            lv_obj_add_flag(_ui_list_search, LV_OBJ_FLAG_HIDDEN);
            return &_ui_list_search_data;
        }
        else return NULL;
    }
    else if( select > 5) return NULL;
    EPAT_TRACE(ui_search_set, 7, "select=%d,offset=%d,title=%s,input=%s;context=%s,%s,%s,%s",
              select, offset, title, input, context[0], context[1], context[2], context[3]);

    if(title != NULL) {
        memset(_ui_list_search_data.title, 0, MAX_TITLE_LENGTH);
        strncpy(_ui_list_search_data.title, title, MAX_TITLE_LENGTH-1);
        // _ui_list_search_data.title[MAX_TITLE_LENGTH-1] = '\0';
    }

    if(input != NULL) {
        memset(_ui_list_search_data.input, 0, MAX_TITLE_LENGTH);
        strncpy(_ui_list_search_data.input, input, LABEL_LENGTH_MAX-1);
        // _ui_list_search_data.input[LABEL_LENGTH_MAX-1] = '\0';
    }
    if(show_type != NULL) {
        memcpy(_ui_list_search_data.show_type, show_type, sizeof(uint8_t)*5);
    }
    if(context != NULL) {
        for(uint8_t i=0; i<4; i++) {
            if(context[i] != NULL) {
                memset(_ui_list_search_data.context[i], 0, LABEL_LENGTH_MAX);
                strncpy(_ui_list_search_data.context[i], context[i], LABEL_LENGTH_MAX-1);
                // _ui_list_search_data.context[i][LABEL_LENGTH_MAX-1] = '\0';
            }
        }
    }

    if(bottomBtn != NULL) {
        for(uint8_t i=0; i<3; i++) {
            if(bottomBtn[i] != NULL) {
                memset(_ui_list_search_data.bottomBtn[i], 0, BTN_LENGTH_MAX);
                strncpy(_ui_list_search_data.bottomBtn[i], bottomBtn[i], BTN_LENGTH_MAX-1);
                // _ui_list_search_data.bottomBtn[i][BTN_LENGTH_MAX-1] = '\0';
            }
        }
    }
    /* 更新运行时数据 */
    _ui_list_search_data.select = select;
    _ui_list_search_data.offset = offset;
    _ui_list_search_data.object = _ui_list_search;
    _ui_list_search_data.screen = NULL;
    /* 发送界面更新消息 */
    GuiMsgT msgPtr = {0};
    msgPtr.ui_data = &_ui_list_search_data;
    msgPtr.refresh_ms = 10;
    msgPtr.ui_set = (void* (*)(void *))ui_list_search_create;
    msgPtr.ui_del = NULL;
    guiSendMsg(&msgPtr);
    osDelay(1);
    
    return msgPtr.ui_data;
}

/**
 * @brief 搜索界面测试用例
 * 
 * @param case_type 测试用例类型
 * 
 * @note 测试用例覆盖场景：
 * - 正常显示
 * - 关闭界面
 * - 边界值测试
 * - 异常参数测试
 */
void ui_search_test(ui_test_case_t case_type)
{
    static uint8_t show_type[4] = {
        0,  //储存区域-手机
        1,  //储存区域-SIM1
        2,  //储存区域-SIM2
        2,  //输入框-pinyin
    };
    static char context[3][LABEL_LENGTH_MAX] = {
        "工作组",
        "亲属组",
        "同学组",
    };
    static char bottomBtn[3][BTN_LENGTH_MAX] = {
        "",      // 左侧按钮
        "选择",  // 中间按钮
        "返回",  // 右侧按钮
    };
    switch(case_type) {
        case TEST_CASE_NORMAL:
            ui_search_set(4, 0, "电话簿", "张三", (uint8_t (*)[5])show_type, context, bottomBtn);
            break;
            
        case TEST_CASE_UI_CLOSE:
            ui_search_set(0, 0, "电话簿", "关闭界面", (uint8_t (*)[5])show_type, context, bottomBtn);
            break;
            
        /* 边界值测试 */
        case TEST_CASE_BOUNDARY_VALUE:
            // 测试select下限
            // ui_search_set(1, 0, "越界测试", "select1", show_type, context, bottomBtn);
            // osDelay(500);
            // // 测试select上限
            // ui_search_set(4, 0, "越界测试", "select4", show_type, context, bottomBtn);
            // osDelay(500);
            // 测试越界值
            // ui_search_set(5, 0, "越界测试", "select5", show_type, context, bottomBtn);
            // osDelay(500);
            // 测试选中第一个列表项
            ui_search_set(1, 0, "电话簿", "选中测试", (uint8_t (*)[5])show_type, 
                        (char[][LABEL_LENGTH_MAX]){"[选中项]", "选项二", "选项三"}, 
                        bottomBtn);
            // 测试选中第二个列表项
            osDelay(1000);
            ui_search_set(2, 0, "电话簿", "选中测试", (uint8_t (*)[5])show_type, 
                        (char[][LABEL_LENGTH_MAX]){"选项一", "[选中项]", "选项三"}, 
                        bottomBtn);
            // 测试选中第三个列表项
            osDelay(1000);
            ui_search_set(3, 0, "电话簿", "选中测试", (uint8_t (*)[5])show_type, 
                        (char[][LABEL_LENGTH_MAX]){"选项一", "选项二", "[选中项]"}, 
                        bottomBtn);
            /* 不显示输入框测试 */
            // 测试选中第一个列表项
            ui_search_set(1, 0, "电话簿", NULL, (uint8_t (*)[5])show_type, 
                    (char[][LABEL_LENGTH_MAX]){"[选中项]", "选项二", "选项三"}, 
                    bottomBtn);
            // 测试选中第二个列表项
            osDelay(1000);
            ui_search_set(2, 0, "电话簿", NULL, (uint8_t (*)[5])show_type, 
                        (char[][LABEL_LENGTH_MAX]){"选项一", "[选中项]", "选项三"}, 
                        bottomBtn);
            // 测试选中第三个列表项
            osDelay(1000);
            ui_search_set(3, 0, "电话簿", NULL, (uint8_t (*)[5])show_type, 
                        (char[][LABEL_LENGTH_MAX]){"选项一", "选项二", "[选中项]"}, 
                        bottomBtn);
            break;
            
        /* 异常参数测试 */
        case TEST_CASE_INVALID_PARAM:
            ui_search_set(4, 0, "电话簿", "参数测试", (uint8_t (*)[5])show_type, context, bottomBtn);
            osDelay(1000);
#if 0 // 此处编译报错，由于是测试代码，遂先关闭，需要此测试项时再打开
            // 测试超长字符串
            ui_search_set(4, 0, 
                "超长标题测试_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                "超长输入内容_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                show_type,
                (char[][LABEL_LENGTH_MAX]){
                    "超长列表项1_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                    "超长列表项2_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                    "超长列表项3_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                },
                (char[][BTN_LENGTH_MAX]){
                    "超长左按钮_XXXXXXXXXXXXXXXXX",
                    "超长中按钮_XXXXXXXXXXXXXXXXX",
                    "超长右按钮_XXXXXXXXXXXXXXXXX"
                }
            );
#endif
            osDelay(1000);
            ui_search_set(1, 0, "选中测试", "选中列表1", (uint8_t (*)[5])show_type, context, bottomBtn);
            osDelay(1000);
            ui_search_set(2, 0, "选中测试", "选中列表2", (uint8_t (*)[5])show_type, context, bottomBtn);
            osDelay(1000);
            ui_search_set(3, 0, "选中测试", "选中列表3", (uint8_t (*)[5])show_type, context, bottomBtn);
            break;
            
        /* 输入法切换测试 */
        case TEST_CASE_INPUT_METHOD:
#if 0 // 此处编译报错，由于是测试代码，遂先关闭，需要此测试项时再打开
            ui_search_set(4, 0, "电话簿", "数字输入法", (uint8_t[4]){0, 1, 2, 0}, context, bottomBtn);
            osDelay(1000);
            ui_search_set(4, 0, "电话簿", "英文输入法", (uint8_t[4]){0, 1, 2, 1}, context, bottomBtn);
            osDelay(1000);
            ui_search_set(4, 0, "电话簿", "拼英输入法", (uint8_t[4]){0, 1, 2, 2}, context, bottomBtn);
#endif
            break;
            
        default:
            break;
    }
}
