#ifndef __GUI_H_
#define __GUI_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include DEBUG_LOG_HEADER_FILE
#include "lvgl.h"
#include "cmsis_os2.h"
#include "ui_helpers.h"
#include "fpui.h"
#if UI_TEST_ITEMS || UI_TEST_ITEM
#define UI_LOG   LV_LOG_USER
#define UI_WRN   LV_LOG_WARN
#define UI_ERR   LV_LOG_ERROR
#else
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#define UI_LOG   SYSLOG_INFO
#define UI_WRN   SYSLOG_WARNING
#define UI_ERR   SYSLOG_ERR
#endif

typedef enum {
    MES_START = 0,
    MES_STEP_1,
    MES_STEP_2,
    MES_STEP_3,
    MES_STEP_ALL,
    MES_END = 254
} TimeMesIndex_e;
extern void timeMes(TimeMesIndex_e index);

typedef enum {
    PAGE_HOME = 0,
    PAGE_MENU ,
    PAGE_LIST ,
    PAGE_CALL ,
    PAGE_APP ,
    PAGE_MAX    
} uiPageIndex_e;

typedef enum {
    NEXT_HOLDON = 0,
    NEXT_UPDATE ,
    NEXT_DELETE ,
    NEXT_HIDDEN ,
    NEXT_ERROR    
} uiUpdateIndex_e;
uint8_t ui_home_auto_update(void);
void ui_auto_update_time(bool en);

typedef enum {
    BOTTOM_BTN_LEFT,
    BOTTOM_BTN_CENTER,
    BOTTOM_BTN_RIGHT
} BottomBtnIndex_e;

lv_obj_t **ui_btn_set(char (*bottomBtn)[8],lv_color_t text,lv_color_t bg);

/**
 * @brief UI组件测试用例类型枚举
 * 
 * 定义标准化测试场景，适用于所有UI组件的自动化测试
 */
typedef enum {
    TEST_CASE_NORMAL,          ///< 正常功能验证：基础功能完整性测试
    TEST_CASE_UI_CLOSE,        ///< 界面关闭测试：验证界面隐藏和资源释放
    TEST_CASE_BOUNDARY_VALUE,  ///< 边界值测试：参数上下限和临界值验证
    TEST_CASE_ITEM_SELECTION,  ///< 选项选中测试：列表项/输入框焦点切换验证
    TEST_CASE_INPUT_METHOD,    ///< 输入法测试：不同输入模式切换验证
    TEST_CASE_BUFFER_OVERFLOW, ///< 缓冲区溢出测试：超长字符串处理验证
    TEST_CASE_INVALID_PARAM,   ///< 非法参数测试：空指针/非法值容错验证
    TEST_CASE_I18N_SUPPORT,    ///< 多语言支持：宽字符/布局适配测试
    TEST_CASE_PERFORMANCE,     ///< 性能测试：渲染效率及内存占用验证
    TEST_CASE_STRESS,          ///< 压力测试：高频次操作稳定性验证
    TEST_CASE_ACCESSIBILITY    ///< 无障碍测试：辅助功能兼容性验证
} ui_test_case_t;

typedef void* (*ui_func_t)(void *data);

typedef enum {
    IME_SELECT_PINYIN,     
    IME_SELECT_HANZI,  
    IME_INPUT_EXIT,    
    IME_INVALID 
} ui_ime_type_e;

typedef struct __Gui_Msg_t
{
    uint32_t  refresh_ms;
    ui_func_t ui_del;
    ui_func_t ui_set;
    ui_func_t ui_get;
    void *ui_data;
} GuiMsgT;

void *ui_bar_set(fpui_bar_t *data);
lv_obj_t *ui_headinfo_set(char  *info);//标题一行不超过6个汉字

lv_obj_t *ui_scrollbar(lv_obj_t *base,uint8_t start,uint8_t total,uint8_t length);

lv_obj_t *ui_callist_set(fpui_callist_t *data);
extern void *ui_imebar_set(uint8_t type, uint8_t index, uint32_t color, char (*p_pinyin)[MAX_IMEBAR_STRLEN], char (*p_hanzi)[MAX_IMEBAR_STRLEN]);

lv_obj_t *ui_home_set(PhoneUI_home_t *data);
void ui_home_screen(PhoneUI_home_t *data);
void ui_time_set(char *date,char *time);
void ui_addr_set(lv_obj_t * img,char *addr);
extern lv_obj_t * ui_Home;
extern lv_obj_t * ui_ImageSig;
extern lv_obj_t * ui_ImageBat;
lv_obj_t *ui_menu_screen(PhoneUI_menu_t *data);
void *ui_menu_set(PhoneUI_menu_t *data);
void *ui_message_set(fpui_message_t *data);

lv_obj_t *ui_list_screen(PhoneUI_list_t *data);
void *ui_list_set(PhoneUI_list_t *data);
void *ui_listpop_set(fpui_listpop_t *data);

lv_obj_t *ui_play_screen(fpui_player_t *data);
void *ui_player_set(fpui_player_t *data);
void ui_call_screen(PhoneUI_call_t *call_data);
lv_obj_t *ui_call_set(PhoneUI_call_t *data);
uint8_t ui_obj_clean(lv_obj_t *object);
extern lv_font_t *pSystem16;
extern lv_font_t *pSystem66;
void ui_version_set(char *str);
uint32_t ui_hangup_cb_register(ui_func_t cb);
extern osEventFlagsId_t uiEvtHandle;
extern lv_disp_drv_t disp_drv;
void guiInit(void);
uint32_t conv_jpeg_rgb565(char *name,lv_img_dsc_t *img);
extern int32_t guiSendMsg(GuiMsgT* msgPtr);
#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif
