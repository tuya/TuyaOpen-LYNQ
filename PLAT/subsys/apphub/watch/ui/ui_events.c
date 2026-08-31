
#include "ui_events.h"

#include <stdbool.h>
#include <stdio.h>

#include "app_hal.h"
#include "cmsis_os2.h"
#include "lvgl.h"
#include "open_image.h"
#include "storage.h"
#include "systime.h"
#include "ui.h"
#include "ui_helpers.h"
#include "camera.h"
#include "servicemanager.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#include DEBUG_LOG_HEADER_FILE
#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_LVGL, subId, debugLevel, format, ##__VA_ARGS__)

static task_node_t *task_list = NULL;

void task_list_print(void)
{
    task_node_t *temp = task_list;
    if(temp == NULL)
    {
        SYSLOG_INFO("Task list is empty!\r\n");
        return;
    }
    while(temp != NULL)
    {
        SYSLOG_INFO("Node: obj=0x%x, arg=%s, callback=0x%x\r\n",
                    (unsigned int)temp->obj, (char *)temp->arg,
                    (unsigned int)temp->callback);
        temp = temp->next;
    }
}
/**
 * @brief 向任务链表中添加一个新任务节点
 *
 * @param obj     与任务关联的LVGL对象
 * @param callback 任务回调函数
 * @param arg     传递给回调函数的参数
 * @param period  任务执行周期（单位：毫秒）
 */
void task_list_add(lv_obj_t *obj, task_callback_t callback, void *arg,
                   uint32_t period)
{
    // 检查输入参数的有效性
    if(!lv_obj_is_valid(obj) || callback == NULL)
    {
        SYSLOG_ERR("Invalid input parameters!\r\n");
        return;
    }

    // 分配新节点的内存
    task_node_t *new_node =
        (task_node_t *)pvPortZeroAssertMallocCust(sizeof(task_node_t));
    if(new_node == NULL)
    {
        SYSLOG_ERR("Failed to allocate memory for new node!\r\n");
        return;
    }

    // 初始化新节点的成员
    new_node->obj = obj;
    new_node->callback = callback;
    new_node->arg = arg;
    new_node->period = period;
    new_node->next = task_list;

    // 将新节点插入链表头部
    task_list = new_node;
}

/**
 * @brief 从任务链表中移除指定回调函数的任务节点
 *
 * @param callback 要移除的任务回调函数
 */
void task_list_remove(task_callback_t callback)
{
    // 检查链表是否为空
    if(task_list == NULL)
    {
        SYSLOG_ERR("Task list is empty!\r\n");
        return;
    }

    task_node_t *temp = task_list;
    task_node_t *prev = NULL;

    // 遍历链表，查找匹配的回调函数
    while(temp != NULL && temp->callback != callback)
    {
        prev = temp;
        temp = temp->next;
    }

    // 如果找到匹配的节点
    if(temp != NULL)
    {
        // 如果匹配的节点是链表头节点
        if(prev == NULL)
        {
            task_list = temp->next;
        }
        else
        {
            // 如果匹配的节点是中间或尾节点
            prev->next = temp->next;
        }

        // 释放节点内存
        vPortFreeCust(temp);
    }
    else
    {
        SYSLOG_ERR("callback not found!\r\n");
    }
}
/**
  \fn
  \brief    可添加到其他又空闲的task中实现多处调用
  \return
*/
AP_PLAT_COMMON_BSS static osMutexId_t taskMutex =
    NULL;  // 在guiTask和coTask中同时运行互斥
AP_PLAT_COMMON_BSS static uint32_t time_last = 0;
uint32_t ui_app_loop(uint32_t timeout)
{
    task_node_t *temp = task_list;
    if(temp == NULL || taskMutex == NULL) return 0;
    uint32_t time_mark = *(volatile uint32_t *)(0x4f020344);
    if(osMutexAcquire(taskMutex, timeout) == osOK)
    {
        do
        {
            if(temp->obj == lv_scr_act() && temp->callback != NULL)
            {
                if(temp->period > 0 && (*(volatile uint32_t *)(0x4f020344) -
                                        time_last) < (temp->period) * 32)
                {
                    // 周期未到
                }
                else
                {
                    temp->callback(temp->arg);
                }
            }
            else if(temp->obj == NULL)
            {
            }
            temp = temp->next;
        } while(temp != NULL);
        osMutexRelease(taskMutex);
    }
    time_last = *(volatile uint32_t *)(0x4f020344);
    return (time_last - time_mark);
}
/**
  \fn
  \brief
  \return
*/
static void ui_coTask(void *arg)
{
    task_node_t *list = task_list;
    if(arg != NULL)
    {
        list = (task_node_t *)arg;
    }
    while(1)
    {
        task_node_t *temp = list;
        if(temp != task_list)
        {
            // 自定义任务链表
        }
        else if(temp == NULL || taskMutex == NULL)
            osDelay(200);
        else if(osMutexAcquire(taskMutex, osWaitForever) == osOK)
        {
            do
            {
                if(temp->obj == NULL)
                {
                }
                else if(temp->obj == lv_scr_act() && temp->callback != NULL)
                {
                    if(temp->period > 0 && (*(volatile uint32_t *)(0x4f020344) -
                                            time_last) < (temp->period) * 32)
                    {
                        // 周期未到
                    }
                    else
                    {
                        temp->callback(temp->arg);
                    }
                }
                temp = temp->next;
            } while(temp != NULL);
            time_last = *(volatile uint32_t *)(0x4f020344);
            osMutexRelease(taskMutex);
        }
        osDelay(2);
    }
#if 1
    osThreadExit();
#else    
    Service_stop("service:/ui_coTask");
#endif
}
/**
  \fn
  \brief    如果不指定新的任务链表将默认运行guiTask伴随运行的任务链表
  \return
*/

static osThreadId_t coTaskHandle = NULL;
void *ui_coTask_init(void *para, void *task)
{
    static task_node_t *coTask_list = NULL;
    if(para != NULL)
    {
        coTask_list = para;
    }
    if(taskMutex == NULL)
    {
        taskMutex = osMutexNew(NULL);
    }
    if(task != NULL && coTaskHandle == NULL)
    {
        osThreadAttr_t *attr = task;
        if(attr->cb_mem == NULL)
        {
            static StaticTask_t task_static;
            attr->cb_mem = &task_static;
            attr->cb_size = sizeof(StaticTask_t);
        }
#if 0
        coTaskHandle = osThreadNew(ui_coTask, coTask_list, attr);
#else
        char serviceName[32] = {0};
        snprintf(serviceName, sizeof(serviceName), "service:/%s", attr->name);
        Service_reg(serviceName, ui_coTask, coTask_list, attr->cb_mem, attr->cb_size, attr->stack_mem, attr->stack_size, attr->priority);
        coTaskHandle = (osThreadId_t)Service_start(serviceName);
#endif
    }
    else if(coTaskHandle != NULL)
    {
        osThreadResume(coTaskHandle);
    }
    return coTaskHandle;
}
void ui_coTask_stop(void)
{
    if(osMutexAcquire(taskMutex, osWaitForever) == osOK)
    {
        osThreadSuspend(coTaskHandle);
    }
}

void ui_coTask_set_state(bool resume)
{
    if(coTaskHandle != NULL)
    {
        if(resume == true)
        {
            osThreadResume(coTaskHandle);
        }
        else
        {
            osThreadSuspend(coTaskHandle);
        }
    }
}

/**
  \fn
  \brief
  \return
*/
void ui_event_brightness_slider_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE)
    {
        lv_coord_t *s = lv_event_get_param(e);
        *s = LV_MAX(*s, 60);
    }
    else if(code == LV_EVENT_DRAW_PART_END)
    {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_param(e);
        if(dsc->part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED))
        {
            onBrightnessChange(e);
        }
    }
}

/**
  \fn
  \brief
  \return
*/
void ui_event_messageClick(lv_event_t *e)
{
    lv_disp_t *display = lv_disp_get_default();
    lv_obj_t *actScr = lv_disp_get_scr_act(display);
    if(actScr != ui_notificationScreen)
    {
        return;
    }
    lv_event_code_t event_code = lv_event_get_code(e);
    // lv_obj_t *target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED)
    {
        onMessageClick();
        lv_obj_scroll_to_y(ui_messagePanel, 0, LV_ANIM_ON);
        lv_obj_add_flag(ui_messageList, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
  \fn
  \brief
  \return
*/
uint8_t phone_text_index = 0;
extern ui_label_t call_label;
const char *btn_dial_list[20] = {"1",
                                 "2",
                                 "3",
                                 "\n",
                                 "4",
                                 "5",
                                 "6",
                                 "\n",
                                 "7",
                                 "8",
                                 "9",
                                 "\n",
                                 "*",
                                 "0",
                                 "#",
                                 "\n",
                                 LV_SYMBOL_CALL,
                                 LV_SYMBOL_STOP,
                                 LV_SYMBOL_NEXT};

static uint8_t dialout_number_add(uint8_t code)
{
    if((code >= '0' && code <= '9') || code == '*' || code == '#')
    {
        call_label.numstr[phone_text_index] = code;
        call_label.numstr[phone_text_index + 1] = '\0';
        phone_text_index++;
        if(phone_text_index >= sizeof(call_label.numstr)) phone_text_index = 0;
        if(phone_text_index < 15)
        {
            lv_label_set_text(ui_dialpadLabel, call_label.numstr);
        }
        else
        {
            lv_label_set_text(ui_dialpadLabel,
                              call_label.numstr + (phone_text_index - 14));
        }
    }
    return phone_text_index;
}
/**
  \fn
  \brief
  \return
*/
static uint8_t dialout_number_del(uint8_t code)
{
    if(code < phone_text_index)
    {
        phone_text_index -= code;
        call_label.numstr[phone_text_index] = '\0';
    }
    else
    {
        memset(call_label.numstr, 0, sizeof(call_label.numstr));
        phone_text_index = 0;
    }
    lv_label_set_text(ui_dialpadLabel, call_label.numstr);
    return phone_text_index;
}
/**
  \fn
  \brief
  \return
*/
lv_obj_t *ui_Label_Sim = NULL;
void ui_event_btnmatrix_dialpad(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    uint16_t id = lv_btnmatrix_get_selected_btn(obj);
    char *txt = lv_btnmatrix_get_btn_text(obj, id);
    if(code == LV_EVENT_CLICKED)
    {
        if(id >= 0 && id < 12)
        {
            dialout_number_add(txt[0]);
        }
        else if(id == 12)  // dialout
        {
            if(simGetStatus(0) != SIM_READY)
            {
                if(ui_Label_Sim == NULL)
                {
                    ui_Label_Sim = lv_label_create(lv_scr_act());
                    lv_obj_set_width(ui_Label_Sim, LV_SIZE_CONTENT);
                    lv_obj_set_height(ui_Label_Sim, LV_SIZE_CONTENT);
                    lv_label_set_text(ui_Label_Sim, "请插入SIM卡");
                    lv_obj_set_style_text_color(
                        ui_Label_Sim, lv_color_make(0xff, 0, 0),
                        LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_align(ui_Label_Sim, LV_ALIGN_TOP_MID, 0, 8);
                }
                lv_obj_clear_flag(ui_Label_Sim, LV_OBJ_FLAG_HIDDEN);
            }
            else if(phone_text_index > 2 && phone_text_index < 19)
            {
                SYSLOG_INFO("%s", call_label.numstr);
                EPAT_LOG(matrix_dialpad, P_INFO, "%s", call_label.numstr);
                _ui_screen_change(ui_callScreen, LV_SCR_LOAD_ANIM_MOVE_BOTTOM,
                                  500, 0);
                _ui_label_set_property(ui_callNumber, _UI_LABEL_PROPERTY_TEXT,
                                       call_label.numstr);
                _ui_state_modify(ui_callButton, LV_STATE_CHECKED,
                                 _UI_MODIFY_STATE_REMOVE);
                _ui_flag_modify(ui_callPanel, LV_OBJ_FLAG_HIDDEN,
                                _UI_MODIFY_FLAG_REMOVE);
                onDialout(call_label.numstr);
            }
        }
        else if(id == 13)
        {
            dialout_number_del(phone_text_index);
            if(ui_Label_Sim != NULL)
            {
                lv_obj_add_flag(ui_Label_Sim, LV_OBJ_FLAG_HIDDEN);
            }
        }
        else if(id == 14)
        {
            dialout_number_del(1);
        }
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_button_call(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    // SYSLOG_INFO("ui_event_button_call %d",event_code);
    if(event_code == LV_EVENT_VALUE_CHANGED &&
       !lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        // LV_LOG_USER("call -> hangup");
        SYSLOG_INFO("call -> hangup");
        onHangUp();
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
    }
    if(event_code == LV_EVENT_VALUE_CHANGED &&
       lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        SYSLOG_INFO("talk -> hangup");
        onHangUp();
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
    }
}
/**
  \fn
  \brief    来电->响铃->通话中or挂断
  \return
*/
extern ui_label_t ring_label;
void ui_event_button_ringing(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED &&
       lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        LV_LOG_USER("ring -> talk");
        memset(ring_label.name, 0, sizeof(ring_label.name));
        strcpy(ring_label.name, "通话中");
        _ui_label_set_property(ui_ringName, _UI_LABEL_PROPERTY_TEXT,
                               ring_label.name);

        memset(ring_label.button, 0, sizeof(ring_label.button));
        strcpy(ring_label.button, "挂断");
        _ui_label_set_property(ui_ringButtonText, _UI_LABEL_PROPERTY_TEXT,
                               ring_label.button);
        onTalking();
    }
    if(event_code == LV_EVENT_VALUE_CHANGED &&
       !lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        LV_LOG_USER("talk -> hangup");
        onHangUp();
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
    }
    if(event_code < LV_EVENT_COVER_CHECK && event_code > LV_EVENT_DRAW_PART_END)
    {
        LV_LOG_USER("%d", event_code);
    }
}
/**
  \fn
  \brief
  \return
*/
void ui_event_resp_ringing(ui_label_t *label)
{
    LV_LOG_USER("%s,%s,%s,%s", label->numstr, label->name, label->icon,
                label->button);
    _ui_screen_change(ui_ringScreen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0);
    _ui_label_set_property(ui_ringNumber, _UI_LABEL_PROPERTY_TEXT,
                           label->numstr);
    lv_obj_clear_flag(ui_ringIcon, LV_OBJ_FLAG_HIDDEN);
    _ui_label_set_property(ui_ringIcon, _UI_LABEL_PROPERTY_TEXT, label->icon);
    _ui_label_set_property(ui_ringName, _UI_LABEL_PROPERTY_TEXT, label->name);
    _ui_label_set_property(ui_ringButtonText, _UI_LABEL_PROPERTY_TEXT,
                           label->button);
    // _ui_label_set_property(ui_ringName, _UI_LABEL_PROPERTY_TEXT, "响铃中");
    // _ui_label_set_property(ui_ringButtonText, _UI_LABEL_PROPERTY_TEXT,
    // "接听");
    EPAT_LOG(ui_event_resp_ringing, P_INFO, "%s", label->numstr);
}
/**
  \fn
  \brief
  \return
*/
// extern ui_label_t ring_label;
// void ui_event_ringing(lv_event_t *e)
// {
//     lv_disp_t *display = lv_disp_get_default();
//     lv_obj_t *actScr = lv_disp_get_scr_act(display);
//     if (actScr != ui_controlScreen)
//     {
//         return;
//     }
//     lv_event_code_t event_code = lv_event_get_code(e);
//     lv_obj_t *target = lv_event_get_target(e);
//     if (event_code == LV_EVENT_CLICKED)
//     {
//         // memset(&ring_label,0,sizeof(ring_label));
//         memset(ring_label.numstr,0,sizeof(call_label.numstr));
//         strcpy(ring_label.numstr,"199199166");
//         memset(ring_label.name,0,sizeof(call_label.name));
//         strcpy(ring_label.name,"芯大爷");
//         memset(ring_label.icon,0,sizeof(call_label.icon));
//         strcpy(ring_label.icon,"D:/ui_img_ring.png");
//         // strcpy(ring_label.button,"接听");
//         ui_event_resp_ringing(&ring_label);
//         lv_obj_clear_flag(ui_ringIcon, LV_OBJ_FLAG_HIDDEN);
//     }
//     if(event_code < LV_EVENT_COVER_CHECK && event_code >
//     LV_EVENT_DRAW_PART_END)
//     {
//         LV_LOG_USER("%d",event_code);
//     }
// }

void onGameOpened() {}
void onGameClosed() {}
/**
  \fn
  \brief
  \return
*/
void ui_gameExit()
{
    lv_scr_load_anim(ui_gameListScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0,
                     false);
}
