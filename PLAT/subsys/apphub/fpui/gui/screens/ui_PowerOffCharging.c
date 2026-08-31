
#include "../ui.h"


void powerOffChargingScreen(fpui_powerOffCharging_t *powerOffCharging)
{
    static lv_obj_t *lvObj = NULL;

    if (lv_obj_is_valid(lvObj) && lvObj != NULL)
    {
        lv_obj_clean(lvObj);
        lv_obj_del_async(lvObj);
        lvObj = NULL;
    }

    if (powerOffCharging != NULL)
    {
        lvObj = lv_obj_create(NULL);
        lv_obj_clear_flag(lvObj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(lvObj, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_bg_opa(lvObj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * level = lv_label_create(lvObj);
        lv_obj_set_width(level, LV_PCT(90));
        lv_obj_set_height(level, 38);
        lv_obj_align(level, LV_ALIGN_CENTER, 0, 0);
        char batteryLevelString[16] = {0};
        snprintf(batteryLevelString, sizeof(batteryLevelString), "Battery:%d%%", powerOffCharging->batteryLevel);
        lv_label_set_text(level, batteryLevelString);
        lv_obj_set_style_text_color(level, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN | LV_STATE_DEFAULT );

        lv_disp_load_scr(lvObj);
    }

    osEventFlagsSet(uiEvtHandle, (1U << PAGE_APP));
}

int32_t ui_powerOffCharging_set(fpui_powerOffCharging_t *powerOffCharging)
{
    static fpui_powerOffCharging_t gPowerOffCharging = {0};
    int32_t retVal = -1;
    GuiMsgT msgPtr =
    {
        .refresh_ms = 10,
        .ui_set     = (ui_func_t)powerOffChargingScreen,
        .ui_data    = &gPowerOffCharging,
    };

    if(powerOffCharging != NULL)
    {
        memcpy(&gPowerOffCharging, powerOffCharging, sizeof(fpui_powerOffCharging_t));
        guiSendMsg(&msgPtr);
        osEventFlagsClear(uiEvtHandle, (1U << PAGE_APP));
        retVal = osEventFlagsWait(uiEvtHandle, (1U << PAGE_APP), osFlagsWaitAll, 300);
    }

    return retVal;
}
