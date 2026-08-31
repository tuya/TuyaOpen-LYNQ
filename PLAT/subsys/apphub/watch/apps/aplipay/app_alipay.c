#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include <string.h>

#include "cmsis_os2.h"
#include "slpman.h"
#include "osasys.h"

#include "main_alipay.h"
#include "app_alipay.h"
#include "ui.h"
#include "ui_helpers.h"
#include "alipay_pay_result.h"
#include "ui_events.h"
#define THREAD_STACK_SIZE_ALIPAY_UI		  (15 * 1024)
#define THRESHOLD 						  (10)
#define ABS(x)  ((x) > 0 ? (x) : -(x))

extern aliAppInfo_t alipayInfo;

void ui_event_alipayScreen(lv_event_t *e);
lv_obj_t *ui_alipayScreen;

lv_obj_t *ui_payCodeLabel;
lv_obj_t *ui_payCodeChgLabel;

lv_obj_t *ui_payCodeScreen;
lv_obj_t *ui_bindCodeScreen;
lv_obj_t *ui_aliSettingScreen;
lv_obj_t *ui_unbindScreen;
lv_obj_t *ui_unbindSuccScreen;

lv_obj_t *ui_paymentScreen;
lv_obj_t *ui_paymentImg;
lv_obj_t *ui_paymentText;
lv_obj_t *ui_paymentBitmap;

lv_obj_t *ui_paymentPanel;
lv_obj_t *ui_paymentLabel;


lv_obj_t *ui_aliSettingTitle;
lv_obj_t *ui_aliTextLabel;

lv_obj_t *ui_aliQueryPanel;
lv_obj_t *ui_aliQueryLabel;

lv_obj_t *ui_aliCancelPanel;
lv_obj_t *ui_aliCancelLabel;

lv_obj_t *ui_aliQueryUnbindPanel;
lv_obj_t *ui_aliQueryUnbindLabel;
lv_obj_t *ui_unbindTitle;


lv_obj_t *ui_userIdPanel;
lv_obj_t *ui_userIdLabel;

lv_obj_t *ui_nickNamePanel;
lv_obj_t *ui_nickNameLabel;

lv_obj_t *ui_unBindPanel;
lv_obj_t *ui_unBindLabel;


lv_obj_t *ui_bindingImg;

lv_obj_t *ui_paycodePanel;
lv_obj_t *ui_alipyaSetPanel;

lv_obj_t *ui_alipayTitle;

lv_obj_t *ui_alipayPaycode;

lv_obj_t *ui_alipayPaycodeQr;
lv_obj_t *ui_alipayPayBarcode;

lv_obj_t *ui_alipayPaycodeStr;

lv_obj_t *ui_alipayBindcodeQr;
lv_obj_t *bindingTextLabel;


lv_obj_t *ui_alipaySet;
lv_obj_t *ui_apipayHelp;


void ui_event_alipayScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        toAppList = true; // flag was open from app list
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
    }
	
}

void ui_event_bindCodeScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        toAppList = true; // flag was open from app list
        _ui_screen_change(ui_home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
    }
	
}


void ui_event_payCodeScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        _ui_screen_change(ui_alipayScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
    }
	
}

void ui_event_unbindScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        _ui_screen_change(ui_aliSettingScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
    }
	
}

void ui_event_payCodeChgScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
    	alipayInfo.payCodeLen = ALI_PAY_CODE_LEN_MAX;
    	alipay_get_paycode(alipayInfo.payCode,&alipayInfo.payCodeLen);
    	ui_alipay_qr_update(ui_alipayPaycodeQr, alipayInfo.payCode, alipayInfo.payCodeLen);
    }
	
}


void ui_event_paycode_select(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
	static lv_point_t start_point = {0,0};
	lv_point_t pressing_point = {0,0};
	int16_t diff_x = 0;
    int16_t diff_y = 0;
    lv_event_code_t event_code = lv_event_get_code(e);
	switch(event_code) 
	{
        case LV_EVENT_PRESSED:
            lv_indev_get_point(lv_indev_get_act(),&start_point);
            break;
        case LV_EVENT_RELEASED:
            lv_indev_get_point(lv_indev_get_act(),&pressing_point);
			diff_x = pressing_point.x - start_point.x;
         	diff_y = pressing_point.y - start_point.y;

			if(ABS(diff_x) > THRESHOLD || ABS(diff_y) > THRESHOLD)
			{
				return;
			}
			else
			{
				ui_alipay_qr_update(ui_alipayPaycodeQr, alipayInfo.payCode, alipayInfo.payCodeLen);
        		_ui_screen_change(ui_payCodeScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
			}
			start_point.x = 0;
			start_point.y = 0;
            break;
        default:
            break;
    }	
}

void ui_event_query_unbind_select(lv_event_t *e)
{
	lv_obj_t *target = lv_event_get_target(e);
	static lv_point_t start_point = {0,0};
	lv_point_t pressing_point = {0,0};
	int16_t diff_x = 0;
    int16_t diff_y = 0;
    lv_event_code_t event_code = lv_event_get_code(e);
	switch(event_code) 
	{
        case LV_EVENT_PRESSED:
            lv_indev_get_point(lv_indev_get_act(),&start_point);
            break;
        case LV_EVENT_RELEASED:
            lv_indev_get_point(lv_indev_get_act(),&pressing_point);
			diff_x = pressing_point.x - start_point.x;
         	diff_y = pressing_point.y - start_point.y;

			if(ABS(diff_x) > THRESHOLD || ABS(diff_y) > THRESHOLD)
			{
				return;
			}
			else
			{
				 if((alipayInfo.status == E_ALI_STA_BOUND) || (alipayInfo.status == E_ALI_STA_BINDIND_SUCCESS))
		    	{
		    		int32_t ret = 0;
					//ret = alipay_unbinding();
					ECPLAT_PRINTF(UNILOG_PLAT_ALIPAY, ui_event_query_unbind_select, P_DEBUG, "ui_event_set_select unbound ret [%d]",ret);
					alipayInfo.status = E_ALI_STA_UNBOUNDIND;
					
					ui_binding_stat_show(E_ALI_STA_UNBOUND);
					_ui_screen_change(ui_unbindSuccScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);/*just for test*/
				}
			}
			start_point.x = 0;
			start_point.y = 0;
            break;
        default:
            break;
    }	
}

void ui_event_query_unbind_success_select(lv_event_t *e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	lv_obj_t *target = lv_event_get_target(e);
	if (event_code == LV_EVENT_CLICKED)
	{
		_ui_screen_change(ui_bindCodeScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);/*just for test*/		
	}	
}


void ui_event_cancel_select(lv_event_t *e)
{
	lv_obj_t *target = lv_event_get_target(e);
	static lv_point_t start_point = {0,0};
	lv_point_t pressing_point = {0,0};
	int16_t diff_x = 0;
    int16_t diff_y = 0;
    lv_event_code_t event_code = lv_event_get_code(e);
	switch(event_code) 
	{
        case LV_EVENT_PRESSED:
            lv_indev_get_point(lv_indev_get_act(),&start_point);
            break;
        case LV_EVENT_RELEASED:
            lv_indev_get_point(lv_indev_get_act(),&pressing_point);
			diff_x = pressing_point.x - start_point.x;
         	diff_y = pressing_point.y - start_point.y;

			if(ABS(diff_x) > THRESHOLD || ABS(diff_y) > THRESHOLD)
			{
				return;
			}
			else
			{
				 _ui_screen_change(ui_aliSettingScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
			}
			start_point.x = 0;
			start_point.y = 0;
            break;
        default:
            break;
    }	
}


void ui_event_unbind_select(lv_event_t *e)
{
   static lv_point_t start_point = {0,0};
	lv_point_t pressing_point = {0,0};
	int16_t diff_x = 0;
    int16_t diff_y = 0;
    lv_event_code_t event_code = lv_event_get_code(e);
	switch(event_code) 
	{
	    case LV_EVENT_PRESSED:
	        lv_indev_get_point(lv_indev_get_act(),&start_point);
	        break;
	    case LV_EVENT_RELEASED:
	        lv_indev_get_point(lv_indev_get_act(),&pressing_point);
			diff_x = pressing_point.x - start_point.x;
	     	diff_y = pressing_point.y - start_point.y;

			if(ABS(diff_x) > THRESHOLD || ABS(diff_y) > THRESHOLD)
			{
				return;
			}
			else
				_ui_screen_change(ui_unbindScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
			start_point.x = 0;
			start_point.y = 0;
	        break;
	    default:
	        break;
	}	
}

void ui_event_payment_select(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
    	_ui_screen_change(ui_payCodeScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
    }
	
}

void ui_event_payment_unbind_select(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
    	 if((alipayInfo.status == E_ALI_STA_BOUND) || (alipayInfo.status == E_ALI_STA_BINDIND_SUCCESS))
    	{
    		int32_t ret = 0;
			//ret = alipay_unbinding();
			alipayInfo.status = E_ALI_STA_UNBOUNDIND;
			
			ui_binding_stat_show(E_ALI_STA_UNBOUND);
			_ui_screen_change(ui_bindCodeScreen, LV_SCR_LOAD_ANIM_NONE, 0, 200);/*just for test*/
			 
			
    	}
    }
	
}


void ui_event_set_select(lv_event_t *e)
{
	static lv_point_t start_point = {0,0};
	lv_point_t pressing_point = {0,0};
	int16_t diff_x = 0;
    int16_t diff_y = 0;
    lv_event_code_t event_code = lv_event_get_code(e);
	ECPLAT_PRINTF(UNILOG_PLAT_ALIPAY, ui_event_set_select, P_DEBUG, "event_code [%d]",event_code);
		switch(event_code) 
		{
        case LV_EVENT_PRESSED:
            lv_indev_get_point(lv_indev_get_act(),&start_point);
            break;
        case LV_EVENT_RELEASED:
            lv_indev_get_point(lv_indev_get_act(),&pressing_point);
			diff_x = pressing_point.x - start_point.x;
         	diff_y = pressing_point.y - start_point.y;

			if(ABS(diff_x) > THRESHOLD || ABS(diff_y) > THRESHOLD)
			{
				return;
			}
			else
				_ui_screen_change(ui_aliSettingScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
            break;
        default:
            break;
    }
	
}

void ui_alipay_qr_update(lv_obj_t *obj,uint8_t *src,uint32_t srcLen)
{
	lv_qrcode_update(obj,src,srcLen);
	if(lv_obj_has_flag(obj,LV_OBJ_FLAG_HIDDEN))
		lv_obj_clear_flag(obj,LV_OBJ_FLAG_HIDDEN);
}



void bind_Animation( lv_obj_t *TargetObject, int delay, int32_t value)
{
    ui_anim_user_data_t *PropertyAnimation_0_user_data = lv_mem_alloc(sizeof(ui_anim_user_data_t));
    PropertyAnimation_0_user_data->target = TargetObject;
    PropertyAnimation_0_user_data->val = -1;
    lv_anim_t PropertyAnimation_0;
    lv_anim_init(&PropertyAnimation_0);
    lv_anim_set_time(&PropertyAnimation_0, 2000);
    lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
    lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_image_angle );
    lv_anim_set_values(&PropertyAnimation_0, 0, value );
    lv_anim_set_path_cb( &PropertyAnimation_0, lv_anim_path_linear);
    lv_anim_set_delay( &PropertyAnimation_0, delay + 0 );
    lv_anim_set_deleted_cb( &PropertyAnimation_0, _ui_anim_callback_free_user_data );
    lv_anim_set_playback_time(&PropertyAnimation_0, 0);
    lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
    lv_anim_set_repeat_count(&PropertyAnimation_0, LV_ANIM_REPEAT_INFINITE );
    lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
    lv_anim_set_early_apply( &PropertyAnimation_0, false );
    lv_anim_start(&PropertyAnimation_0);
}

void ui_bindScreen_callback(void *arg)
{
	(void)arg;
	static BOOL bcCnt = FALSE;
	static uint32_t curStatus = E_ALI_STA_MAX;
	
	if(curStatus == alipayInfo.status)
		return;
	curStatus = alipayInfo.status;
	switch(alipayInfo.status)
	{
		case E_ALI_STA_UNBOUND:
		{
			if(alipayInfo.bindCode && (strlen(alipayInfo.bindCode) > 0))
			{
				if(bcCnt == FALSE)
					ui_alipay_qr_update(ui_alipayBindcodeQr,alipayInfo.bindCode,alipayInfo.bindCodeLen);
				bcCnt = TRUE;
				if(lv_obj_has_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN))
					lv_obj_clear_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				lv_obj_add_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			}
			lv_obj_set_style_bg_color(ui_bindCodeScreen, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
			lv_obj_add_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN);
			
		}
		break;
		case E_ALI_STA_BINDIND:
		{
			lv_obj_add_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN);

			if(lv_obj_has_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN);
			
			lv_obj_set_style_bg_color(ui_bindCodeScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
			lv_img_set_src(ui_bindingImg,"D:/load_logo.png");
			lv_img_set_angle(ui_bindingImg,0);
			bind_Animation(ui_bindingImg,0,3600);
			lv_label_set_text(bindingTextLabel, "正在绑定");
			lv_obj_align_to(bindingTextLabel, ui_bindingImg, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
		}
		break;
		case E_ALI_STA_BINDIND_SUCCESS:
		{
			lv_anim_del_all();
			lv_obj_add_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN);
			lv_obj_set_style_bg_color(ui_bindCodeScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
			//lv_anim_del(ui_bindingImg,NULL);
			lv_img_set_src(ui_bindingImg,"D:/success_logo.png");
			
			lv_img_set_angle(ui_bindingImg,0);
			lv_label_set_text(bindingTextLabel, "绑定成功");
			lv_obj_align_to(bindingTextLabel, ui_bindingImg, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

			lv_label_set_text(ui_aliSettingTitle, "设置");
			lv_label_set_text_fmt(ui_userIdLabel, "%s",alipayInfo.userID);
			lv_obj_align_to(ui_userIdLabel, ui_aliSettingScreen, LV_ALIGN_TOP_MID, 0, 112);	
			
			lv_label_set_text_fmt(ui_nickNameLabel, "%s",alipayInfo.nickName);
			lv_obj_align_to(ui_nickNameLabel, ui_aliSettingScreen, LV_ALIGN_TOP_MID, 0, 88);	
			_ui_screen_change(ui_alipayScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 8*1000);
		}
		break;
		case E_ALI_STA_BINDIND_FAIL:
		{
			lv_anim_del_all();
			lv_obj_add_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN);
			lv_obj_set_style_bg_color(ui_bindCodeScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
			//lv_anim_del(ui_bindingImg,NULL);
			lv_img_set_src(ui_bindingImg,"D:/fail_logo.png");
			lv_label_set_text(bindingTextLabel, "绑定失败");
			lv_obj_align_to(bindingTextLabel, ui_bindingImg, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
		}
		break;
		default:
			break;
	}
	return;
}

void ui_aliSetting_callback(void *arg)
{
	(void)arg;
	lv_label_set_text_fmt(ui_nickNameLabel, "%s",alipayInfo.nickName);
	lv_label_set_text_fmt(ui_userIdLabel, "%s",alipayInfo.userID);
	return;
}

void ui_payment_callback(void *arg)
{
	(void)arg;
	
	switch(alipayInfo.paymentStatus)
	{
		case ALIPAY_PAYMENT_STATUS_SUCCESS:
		{
			lv_img_set_src(ui_paymentImg,"D:/success_logo.png");
			/*支付信息*/
			extern lv_font_t *pFontExtern_48;

			lv_label_set_text_fmt(ui_paymentText,"￥ %s",alipayInfo.amount);
			lv_obj_set_style_text_font(ui_paymentText, pFontExtern_48, LV_PART_MAIN | LV_STATE_DEFAULT);
			lv_label_set_text(ui_paymentLabel, "确定");	
			lv_obj_remove_event_cb(ui_paymentLabel,ui_event_payment_unbind_select);
			lv_obj_add_event_cb(ui_paymentPanel, ui_event_payment_select, LV_EVENT_CLICKED, NULL);
			_ui_screen_change(ui_paymentScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);	
			alipay_get_paycode(alipayInfo.payCode,&alipayInfo.payCodeLen);
			ui_alipay_qr_update(ui_alipayPaycodeQr, alipayInfo.payCode, alipayInfo.payCodeLen);
		}
		break;
		case ALIPAY_PAYMENT_STATUS_UNBIND_BY_USER:
			lv_img_set_src(ui_paymentImg,"D:/fail_logo.png");
			
			lv_label_set_text(ui_paymentText,"该设备已经解绑，请先用手\n机支付扫码绑定");
			lv_obj_set_style_text_font(ui_paymentText, LV_FONT_DEFAULT, LV_STATE_DEFAULT);
			lv_label_set_text(ui_paymentLabel, "开始绑定");
			lv_obj_remove_event_cb(ui_paymentPanel,ui_event_payment_select);
			lv_obj_add_event_cb(ui_paymentPanel, ui_event_payment_unbind_select, LV_EVENT_CLICKED, NULL);
			_ui_screen_change(ui_paymentScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
		break;
		case ALIPAY_PAYMENT_STATUS_DISABLED_BY_USER:
			lv_img_set_src(ui_paymentImg,"D:/fail_logo.png");
			lv_obj_set_style_text_font(ui_paymentText, LV_FONT_DEFAULT, LV_STATE_DEFAULT);
			lv_label_set_text(ui_paymentLabel, "确定");
			lv_obj_remove_event_cb(ui_paymentLabel,ui_event_payment_unbind_select);
			lv_obj_add_event_cb(ui_paymentPanel, ui_event_payment_select, LV_EVENT_CLICKED, NULL);
			_ui_screen_change(ui_paymentScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
			alipay_get_paycode(alipayInfo.payCode,&alipayInfo.payCodeLen);
			ui_alipay_qr_update(ui_alipayPaycodeQr, alipayInfo.payCode, alipayInfo.payCodeLen);
		break;
		case ALIPAY_PAYMENT_STATUS_NOTHING:
		break;
		default:
		break;
	}
	alipayInfo.paymentStatus = ALIPAY_PAYMENT_STATUS_NOTHING;
	return;
}

void ui_paymnet_state_show(uint8_t result, uint8_t status)
{
	//ui_alipay_qr_update(ui_alipayPaycodeQr, alipayInfo.payCode, alipayInfo.payCodeLen);
	lv_obj_t * curr_act = lv_scr_act();
	if((curr_act == ui_payCodeScreen) || (curr_act == ui_paymentScreen))
	{
		if(result == ALIPAY_RV_OK)
		{
			switch(status)
			{
				case ALIPAY_PAYMENT_STATUS_SUCCESS:
				{
					lv_img_set_src(ui_paymentImg,"D:/success_logo.png");
					/*支付信息*/
					extern lv_font_t *pFontExtern_48;

					lv_label_set_text_fmt(ui_paymentText,"￥ %s",alipayInfo.amount);
					lv_obj_set_style_text_font(ui_paymentText, pFontExtern_48, LV_PART_MAIN | LV_STATE_DEFAULT);
					lv_label_set_text(ui_paymentLabel, "确定");	
					lv_obj_remove_event_cb(ui_paymentLabel,ui_event_payment_unbind_select);
					lv_obj_add_event_cb(ui_paymentPanel, ui_event_payment_select, LV_EVENT_CLICKED, NULL);
					_ui_screen_change(ui_paymentScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);	
					alipay_get_paycode(alipayInfo.payCode,&alipayInfo.payCodeLen);
	    			ui_alipay_qr_update(ui_alipayPaycodeQr, alipayInfo.payCode, alipayInfo.payCodeLen);
				}
				break;
				case ALIPAY_PAYMENT_STATUS_UNBIND_BY_USER:
					lv_img_set_src(ui_paymentImg,"D:/fail_logo.png");
					
					lv_label_set_text(ui_paymentText,"该设备已经解绑，请先用手\n机支付扫码绑定");
					lv_obj_set_style_text_font(ui_paymentText, LV_FONT_DEFAULT, LV_STATE_DEFAULT);
					lv_label_set_text(ui_paymentLabel, "开始绑定");
					lv_obj_remove_event_cb(ui_paymentPanel,ui_event_payment_select);
					lv_obj_add_event_cb(ui_paymentPanel, ui_event_payment_unbind_select, LV_EVENT_CLICKED, NULL);
					_ui_screen_change(ui_paymentScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
				break;
				case ALIPAY_PAYMENT_STATUS_DISABLED_BY_USER:
					lv_img_set_src(ui_paymentImg,"D:/fail_logo.png");
					lv_obj_set_style_text_font(ui_paymentText, LV_FONT_DEFAULT, LV_STATE_DEFAULT);
					lv_label_set_text(ui_paymentLabel, "确定");
					lv_obj_remove_event_cb(ui_paymentLabel,ui_event_payment_unbind_select);
					lv_obj_add_event_cb(ui_paymentPanel, ui_event_payment_select, LV_EVENT_CLICKED, NULL);
					_ui_screen_change(ui_paymentScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
					alipay_get_paycode(alipayInfo.payCode,&alipayInfo.payCodeLen);
	    			ui_alipay_qr_update(ui_alipayPaycodeQr, alipayInfo.payCode, alipayInfo.payCodeLen);
				break;
				case ALIPAY_PAYMENT_STATUS_NOTHING:
				break;
				default:
				break;
			}
		}
		else if((result == 99) || (result == 29) || (result == 7))
		{
			return;
		}
		else
		{
			lv_img_set_src(ui_paymentImg,"D:/fail_logo.png");
			lv_obj_set_style_text_font(ui_paymentText, LV_FONT_DEFAULT, LV_STATE_DEFAULT);
			lv_label_set_text_fmt(ui_paymentText,"网络不稳定_%d,请\n在手机支付宝上或与商家确\n认支付结果",result);
			lv_label_set_text(ui_paymentLabel, "确认");
			lv_obj_remove_event_cb(ui_paymentLabel,ui_event_payment_unbind_select);
			lv_obj_add_event_cb(ui_paymentPanel, ui_event_payment_select, LV_EVENT_CLICKED, NULL);
			_ui_screen_change(ui_paymentScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0);
		}
	}
	return;
}

void ui_binding_stat_show(uint8_t status)
{
	static BOOL bcCnt = FALSE;
	switch(status)
	{
		case E_ALI_STA_UNBOUND:
		{
			if(alipayInfo.bindCode && (strlen(alipayInfo.bindCode) > 0))
			{
				if(bcCnt == FALSE)
					ui_alipay_qr_update(ui_alipayBindcodeQr,alipayInfo.bindCode,alipayInfo.bindCodeLen);
				bcCnt = TRUE;
				if(lv_obj_has_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN))
					lv_obj_clear_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				lv_obj_add_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			}
			lv_obj_set_style_bg_color(ui_bindCodeScreen, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
			lv_obj_add_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN);
			
		}
		break;
		case E_ALI_STA_BINDIND:
		{
			lv_obj_add_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN);

			if(lv_obj_has_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN);
			
			lv_obj_set_style_bg_color(ui_bindCodeScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
			lv_img_set_src(ui_bindingImg,"D:/load_logo.png");
			lv_img_set_angle(ui_bindingImg,0);
			bind_Animation(ui_bindingImg,0,3600);
			lv_label_set_text(bindingTextLabel, "正在绑定");
			lv_obj_align_to(bindingTextLabel, ui_bindingImg, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
		}
		break;
		case E_ALI_STA_BINDIND_SUCCESS:
		{
			lv_anim_del_all();
			lv_obj_add_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN);
			lv_obj_set_style_bg_color(ui_bindCodeScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
			//lv_anim_del(ui_bindingImg,NULL);
			lv_img_set_src(ui_bindingImg,"D:/success_logo.png");
			
			lv_img_set_angle(ui_bindingImg,0);
			lv_label_set_text(bindingTextLabel, "绑定成功");
			lv_obj_align_to(bindingTextLabel, ui_bindingImg, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

			lv_label_set_text(ui_aliSettingTitle, "设置");
			lv_label_set_text_fmt(ui_userIdLabel, "%s",alipayInfo.userID);
			lv_obj_align_to(ui_userIdLabel, ui_aliSettingScreen, LV_ALIGN_TOP_MID, 0, 112);	
			
			lv_label_set_text_fmt(ui_nickNameLabel, "%s",alipayInfo.nickName);
			lv_obj_align_to(ui_nickNameLabel, ui_aliSettingScreen, LV_ALIGN_TOP_MID, 0, 88);	
			_ui_screen_change(ui_alipayScreen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 8*1000);
		}
		break;
		case E_ALI_STA_BINDIND_FAIL:
		{
			lv_anim_del_all();
			lv_obj_add_flag(ui_alipayBindcodeQr,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(ui_bindingImg,LV_OBJ_FLAG_HIDDEN);
			if(lv_obj_has_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN))
				lv_obj_clear_flag(bindingTextLabel,LV_OBJ_FLAG_HIDDEN);
			lv_obj_set_style_bg_color(ui_bindCodeScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
			//lv_anim_del(ui_bindingImg,NULL);
			lv_img_set_src(ui_bindingImg,"D:/fail_logo.png");
			lv_label_set_text(bindingTextLabel, "绑定失败");
			lv_obj_align_to(bindingTextLabel, ui_bindingImg, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
		}
		break;
		default:
			break;
	}
}

void ui_alipay_init(void)
{	
	/*绑定码页面*/
	ui_bindCodeScreen = lv_obj_create(NULL);
	lv_obj_clear_flag(ui_bindCodeScreen, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_bindCodeScreen, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_bindCodeScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_alipayBindcodeQr = lv_qrcode_create(ui_bindCodeScreen,lv_obj_get_width(ui_bindCodeScreen)/2,lv_color_hex3(0x000),lv_color_hex3(0xfff));
	
	lv_obj_t * title = lv_label_create(ui_bindCodeScreen);
	lv_obj_t * chg_label = lv_label_create(ui_bindCodeScreen);
	lv_obj_add_event_cb(ui_bindCodeScreen, ui_event_alipayScreen, LV_EVENT_ALL, NULL);

	//lv_qrcode_update(ui_alipayBindcodeQr,alipayInfo.bindCode,alipayInfo.bindCodeLen);
    lv_obj_center(ui_alipayBindcodeQr);
	lv_obj_add_flag(ui_alipayBindcodeQr, LV_OBJ_FLAG_HIDDEN); /// Flags
    lv_label_set_recolor(title,true);
    lv_label_set_text(title, "#000000 使用支付宝扫码绑定#");
    lv_obj_set_x(title, 50);
    lv_obj_set_y(title, 40);

	ui_bindingImg = lv_img_create(ui_bindCodeScreen);
	lv_obj_center(ui_bindingImg);
	bindingTextLabel = lv_label_create(ui_bindCodeScreen);
		
	lv_obj_add_event_cb(ui_bindCodeScreen,ui_event_bindCodeScreen,LV_EVENT_ALL,NULL);
	
	/*设置选项页面*/
	ui_alipayScreen = lv_obj_create(NULL);
	lv_obj_clear_flag(ui_alipayScreen, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_alipayScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_alipayScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_paycodePanel = lv_obj_create(ui_alipayScreen);
    lv_obj_set_width(ui_paycodePanel, lv_pct(90));
    lv_obj_set_height(ui_paycodePanel, lv_pct(30));
	lv_obj_clear_flag(ui_paycodePanel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_paycodePanel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_x(ui_paycodePanel, 8);
    lv_obj_set_y(ui_paycodePanel, 8);
	
	lv_obj_t *ui_paycodeImg = lv_img_create(ui_paycodePanel);
	lv_img_set_src(ui_paycodeImg,"D:/ali_paycode.png");
	lv_obj_align_to(ui_paycodeImg, ui_paycodePanel, LV_ALIGN_TOP_LEFT, 0, 0);
	
    ui_alipayPaycode = lv_label_create(ui_paycodePanel);
    lv_obj_set_width(ui_alipayPaycode, 139);
    lv_obj_set_height(ui_alipayPaycode, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_alipayPaycode, 43);
    lv_obj_set_y(ui_alipayPaycode, 8);
    lv_label_set_text(ui_alipayPaycode, "付款码");
	lv_obj_add_flag(ui_paycodePanel,LV_OBJ_FLAG_CLICKABLE);
	
	ui_alipyaSetPanel = lv_obj_create(ui_alipayScreen);
    lv_obj_set_width(ui_alipyaSetPanel, lv_pct(90));
    lv_obj_set_height(ui_alipyaSetPanel, lv_pct(30));
	lv_obj_clear_flag(ui_alipyaSetPanel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_alipyaSetPanel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
	//lv_obj_align_to(ui_paycodePanel, ui_alipayScreen, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_x(ui_alipyaSetPanel, 8);
    lv_obj_set_y(ui_alipyaSetPanel, 80);

	lv_obj_t *ui_setImg = lv_img_create(ui_alipyaSetPanel);
	lv_img_set_src(ui_setImg,"D:/ali_set.png");
	lv_obj_align_to(ui_setImg, ui_alipyaSetPanel, LV_ALIGN_TOP_LEFT, 0, 0);
	
    ui_alipaySet = lv_label_create(ui_alipyaSetPanel);
    lv_obj_set_width(ui_alipaySet, 139);
    lv_obj_set_height(ui_alipaySet, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_alipaySet, 43);
    lv_obj_set_y(ui_alipaySet, 8);
    lv_label_set_text(ui_alipaySet, "设置");
	lv_obj_add_flag(ui_alipyaSetPanel,LV_OBJ_FLAG_CLICKABLE);
	
	lv_obj_add_event_cb(ui_paycodePanel, ui_event_paycode_select, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_alipyaSetPanel, ui_event_set_select, LV_EVENT_ALL , NULL);
	lv_obj_add_event_cb(ui_alipayScreen,ui_event_alipayScreen,LV_EVENT_ALL,NULL);

	/*支付码页面*/
	ui_payCodeScreen = lv_obj_create(NULL);
	lv_obj_clear_flag(ui_payCodeScreen, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_payCodeScreen, lv_color_hex(0x1677ff), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_payCodeScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_alipayPaycodeQr = lv_qrcode_create(ui_payCodeScreen,lv_obj_get_width(ui_payCodeScreen)/2,lv_color_hex3(0x000),lv_color_hex3(0xfff));
    lv_obj_center(ui_alipayPaycodeQr);
	lv_obj_add_flag(ui_alipayPaycodeQr,LV_OBJ_FLAG_CLICKABLE);
	
    lv_obj_t * title1 = lv_label_create(ui_payCodeScreen);	
    lv_label_set_text(title1, "付款码");
    lv_obj_align_to(title1, ui_payCodeScreen, LV_ALIGN_TOP_MID, 0, 16);

	ui_payCodeChgLabel = lv_label_create(ui_payCodeScreen);
	lv_label_set_text(ui_payCodeChgLabel, "切换条形码");
    lv_obj_align_to(ui_payCodeChgLabel, ui_payCodeScreen, LV_ALIGN_BOTTOM_MID, 0, -16);
	lv_obj_add_flag(ui_payCodeChgLabel,LV_OBJ_FLAG_CLICKABLE);

	lv_obj_add_event_cb(ui_payCodeScreen,ui_event_payCodeScreen,LV_EVENT_ALL,NULL);
	lv_obj_add_event_cb(ui_payCodeChgLabel,ui_event_payCodeScreen,LV_EVENT_ALL,NULL);
	lv_obj_add_event_cb(ui_alipayPaycodeQr,ui_event_payCodeChgScreen,LV_EVENT_ALL,NULL);

	/*设置页面*/
	ui_aliSettingScreen = lv_obj_create(NULL);
	lv_obj_clear_flag(ui_aliSettingScreen, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_aliSettingScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_aliSettingScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_aliSettingTitle = lv_label_create(ui_aliSettingScreen);	
	
    lv_label_set_text(ui_aliSettingTitle, "设置");
    lv_obj_align_to(ui_aliSettingTitle, ui_aliSettingScreen, LV_ALIGN_TOP_MID, 0, 16);

	//add name and count
	
	ui_nickNameLabel = lv_label_create(ui_aliSettingScreen);
	lv_label_set_text_fmt(ui_nickNameLabel, "%s",alipayInfo.nickName);
	lv_obj_align_to(ui_nickNameLabel, ui_aliSettingScreen, LV_ALIGN_TOP_MID, 0, 88);

	ui_userIdLabel = lv_label_create(ui_aliSettingScreen);
	lv_label_set_text_fmt(ui_userIdLabel, "%s",alipayInfo.userID);
	lv_obj_align_to(ui_userIdLabel, ui_aliSettingScreen, LV_ALIGN_TOP_MID, 0, 112);			
	
	ui_unBindPanel = lv_obj_create(ui_aliSettingScreen);
    lv_obj_set_width(ui_unBindPanel, 168);
    lv_obj_set_height(ui_unBindPanel, 48); /// 1
	lv_obj_set_style_bg_color(ui_unBindPanel, lv_color_hex(0x1677ff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(ui_unBindPanel, ui_aliSettingScreen, LV_ALIGN_BOTTOM_MID, 0, -10);
	lv_obj_set_style_radius(ui_unBindPanel,lv_obj_get_height(ui_unBindPanel) / 2,LV_STATE_DEFAULT);
	
	ui_unBindLabel = lv_label_create(ui_unBindPanel);
	lv_obj_center(ui_unBindLabel);
    lv_label_set_text(ui_unBindLabel, "解除绑定");

	lv_obj_add_flag(ui_unBindPanel,LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ui_aliSettingScreen,ui_event_payCodeScreen,LV_EVENT_ALL,NULL);
	lv_obj_add_event_cb(ui_unBindPanel, ui_event_unbind_select, LV_EVENT_ALL, NULL);
	
	/*解除绑定页面*/
	ui_unbindScreen = lv_obj_create(NULL);
	lv_obj_clear_flag(ui_unbindScreen, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_unbindScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_unbindScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_unbindTitle = lv_label_create(ui_unbindScreen);
	lv_obj_center(ui_unbindTitle);
	//lv_obj_set_x(ui_unbindTitle, 43);
    //lv_obj_set_y(ui_unbindTitle, 8);
    lv_obj_align_to(ui_unbindTitle, ui_unbindScreen, LV_ALIGN_TOP_MID, -16, 16);
    lv_label_set_text(ui_unbindTitle, "解除绑定");
	
	lv_obj_t *ui_queryText = lv_label_create(ui_unbindScreen);
	lv_obj_center(ui_queryText);
	//lv_label_set_long_mode(ui_queryText, LV_LABEL_LONG_WRAP); 
	lv_label_set_text(ui_queryText, "解绑后将无法使用支付宝\n各项功能，确认解绑?");
	
	ui_aliQueryUnbindPanel = lv_obj_create(ui_unbindScreen);
    lv_obj_set_width(ui_aliQueryUnbindPanel, 84);
    lv_obj_set_height(ui_aliQueryUnbindPanel, 48); /// 1
	lv_obj_set_style_bg_color(ui_aliQueryUnbindPanel, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(ui_aliQueryUnbindPanel, ui_unbindScreen, LV_ALIGN_BOTTOM_LEFT, 20, -20);
	lv_obj_set_style_radius(ui_aliQueryUnbindPanel,lv_obj_get_height(ui_aliQueryUnbindPanel) / 2,LV_STATE_DEFAULT);

	ui_aliQueryUnbindLabel = lv_label_create(ui_aliQueryUnbindPanel);
	lv_obj_center(ui_aliQueryUnbindLabel);
	lv_label_set_recolor(ui_aliQueryUnbindLabel,true);
    lv_label_set_text(ui_aliQueryUnbindLabel, "#000000 确认#");
	
	lv_obj_add_flag(ui_aliQueryUnbindPanel, LV_OBJ_FLAG_CLICKABLE);

	ui_aliCancelPanel = lv_obj_create(ui_unbindScreen);
    lv_obj_set_width(ui_aliCancelPanel, 84);
    lv_obj_set_height(ui_aliCancelPanel, 48); /// 1
	lv_obj_set_style_bg_color(ui_aliCancelPanel, lv_color_hex(0x1677ff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(ui_aliCancelPanel, ui_unbindScreen, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
	lv_obj_set_style_radius(ui_aliCancelPanel,lv_obj_get_height(ui_aliCancelPanel) / 2,LV_STATE_DEFAULT);

	ui_aliCancelLabel = lv_label_create(ui_aliCancelPanel);
	lv_obj_center(ui_aliCancelLabel);
    lv_label_set_text(ui_aliCancelLabel, "取消");	
	lv_obj_add_flag(ui_aliCancelPanel, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_add_event_cb(ui_unbindScreen,ui_event_unbindScreen,LV_EVENT_ALL,NULL);	
	lv_obj_add_event_cb(ui_aliQueryUnbindPanel, ui_event_query_unbind_select, LV_EVENT_ALL, NULL);		
	lv_obj_add_event_cb(ui_aliCancelPanel, ui_event_cancel_select, LV_EVENT_ALL, NULL);

	/*解绑成功页面*/
	ui_unbindSuccScreen = lv_obj_create(NULL);
	lv_obj_clear_flag(ui_unbindSuccScreen, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_unbindSuccScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_unbindSuccScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t *ubsTitle = lv_label_create(ui_unbindSuccScreen);
    lv_obj_align_to(ubsTitle, ui_unbindSuccScreen, LV_ALIGN_TOP_MID, -16, 16);
    lv_label_set_text(ubsTitle, "解绑成功");
	
	lv_obj_t *ui_usbText = lv_label_create(ui_unbindSuccScreen);
	lv_obj_center(ui_usbText);
	lv_label_set_text(ui_usbText, "手表侧已解绑，请到手机支\n付宝搜索进入“智能设备”\n小程序查看，若手机侧未解\n绑，则需操作解绑");

	lv_obj_t *ubsPanel = lv_obj_create(ui_unbindSuccScreen);
    lv_obj_set_width(ubsPanel, 84);
    lv_obj_set_height(ubsPanel, 48); /// 1
	lv_obj_set_style_bg_color(ubsPanel, lv_color_hex(0x1677ff), LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_align_to(ubsPanel, ui_unbindSuccScreen, LV_ALIGN_BOTTOM_MID, 0, -16);
	lv_obj_set_style_radius(ubsPanel,lv_obj_get_height(ubsPanel) / 2,LV_STATE_DEFAULT);

	lv_obj_t *ubsLabel = lv_label_create(ubsPanel);
	lv_obj_center(ubsLabel);
    lv_label_set_text(ubsLabel, "确认");	
	
	lv_obj_add_flag(ubsPanel, LV_OBJ_FLAG_CLICKABLE);	
	lv_obj_add_event_cb(ubsPanel, ui_event_query_unbind_success_select, LV_EVENT_CLICKED, NULL);

	/*支付页面*/
	ui_paymentScreen = lv_obj_create(NULL);
	lv_obj_clear_flag(ui_paymentScreen, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_color(ui_paymentScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_paymentScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_paymentImg = lv_img_create(ui_paymentScreen);
	lv_obj_align_to(ui_paymentImg,ui_paymentScreen,LV_ALIGN_TOP_MID,-24,20);

	ui_paymentText = lv_label_create(ui_paymentScreen);	
	//lv_obj_align_to(ui_paymentText,ui_paymentScreen,LV_ALIGN_TOP_MID,0,120);
	lv_obj_center(ui_paymentText);

	ui_paymentPanel = lv_obj_create(ui_paymentScreen);
    lv_obj_set_width(ui_paymentPanel, 168);
    lv_obj_set_height(ui_paymentPanel, 48); /// 1
	lv_obj_set_style_bg_color(ui_paymentPanel, lv_color_hex(0x1677ff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(ui_paymentPanel, ui_paymentScreen, LV_ALIGN_BOTTOM_MID, 0, -40);
	lv_obj_set_style_radius(ui_paymentPanel,lv_obj_get_height(ui_paymentPanel) / 2,LV_STATE_DEFAULT);
	
	ui_paymentLabel = lv_label_create(ui_paymentPanel);
	lv_obj_center(ui_paymentLabel);

	lv_obj_add_flag(ui_paymentPanel,LV_OBJ_FLAG_CLICKABLE);
	//lv_obj_add_event_cb(ui_paymentPanel, ui_event_payment_select, LV_EVENT_CLICKED, NULL);
	task_list_add(ui_bindCodeScreen,ui_bindScreen_callback,NULL,20);
	task_list_add(ui_payCodeScreen,ui_payment_callback,NULL,20);
	task_list_add(ui_paymentScreen,ui_payment_callback,NULL,20);
	task_list_add(ui_aliSettingScreen,ui_aliSetting_callback,NULL,20);

}


