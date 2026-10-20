/**
* @file tkl_init_cellular.c
* @brief Common process - tkl init cellular description
* @version 0.1
* @date 2022-01-14
*
* @copyright Copyright 2022 Tuya Inc. All Rights Reserved.
*
*/
#include "tkl_init_cellular.h"
#include "tkl_cellular.h"

static const TKL_CELLULAR_DESC_T c_legacy_cell_desc = {
    .init = tkl_cellular_init,
    .get_status = tkl_cellular_get_status,
    .set_status_cb = tkl_cellular_set_status_cb,
    .get_ip = tkl_cellular_get_ip,
    .get_ipv6 = tkl_cellular_get_ipv6,
    .get_ccid = tkl_cellular_get_ccid,
    .get_rssi = tkl_cellular_get_rssi,
    .get_volt = tkl_cellular_get_volt,
    .get_imei = tkl_cellular_get_imei,
    .get_sn = tkl_cellular_get_sn,
    .get_sw_ver = tkl_cellular_get_sw_ver,
};

TKL_CELLULAR_DESC_T *tkl_cellular_desc_get(void)
{
    return (TKL_CELLULAR_DESC_T *)&c_legacy_cell_desc;
}

TKL_CELL_BASE_INTF_T c_cell_desc = {
    .base_init                  = tkl_cellular_base_init,
    .get_ability                = tkl_cellular_base_get_ability,
    .switch_sim                 = tkl_cellular_base_switch_sim,
    .register_sim_state_notify  = tkl_cellular_base_register_sim_state_notify,
    .sim_hotplug_enable         = tkl_cellular_base_enable_sim_hotplug,
    .sim_get_status             = tkl_cellular_base_sim_get_status,
    .get_cfun_mode              = tkl_cellular_base_get_cfun_mode,
    .set_cfun_mode              = tkl_cellular_base_set_cfun_mode,
    .get_imsi                   = tkl_cellular_base_get_imsi,
    .get_iccid                  = tkl_cellular_base_get_iccid,
    .get_imei                   = tkl_cellular_base_get_imei,
    .set_imei                   = tkl_cellular_base_set_imei,
    .get_rsrp                   = tkl_cellular_base_get_rsrp,
    .get_sinr                   = tkl_cellular_base_get_sinr,
    .get_lbs                    = tkl_cellular_base_get_lbs,
    .get_default_simid          = tkl_cellular_base_get_default_simid,
    .ioctl                      = tkl_cellular_base_ioctl,
    .get_epoch_sec              = tkl_cellular_comm_get_epoch_time,
    .get_rssidbm                = tkl_cellular_comm_get_rssi_dBm,
    .get_rssi                   = tkl_cellular_comm_get_rssi,
    .get_nettype                = tkl_cellular_comm_get_nettype,
    .get_localtime              = tkl_cellular_comm_ctrl_get_localtime,
    .get_timezone               = tkl_cellular_comm_get_timezone,
    .get_sn                     = tkl_cellular_comm_get_sn,
    .register_dev_reg_notify    = tkl_cellular_register_dev_reg_notify,
};

TKL_CELL_BASE_INTF_T* tkl_cellular_base_desc_get()
{
    return &c_cell_desc;
}

TKL_CELL_MDS_INTF_T c_cell_mds_desc = {
    .mds_init                       = tkl_cellular_mds_init,
    .get_mds_status                 = tkl_cellular_mds_get_status,
    .pdp_active                     = tkl_cellular_mds_pdp_active,
    .adv_pdp_active                 = tkl_cellular_mds_adv_pdp_active,
    .pdp_deactive                   = tkl_cellular_mds_pdp_deactive,
    .adv_pdp_deactive               = tkl_cellular_mds_adv_pdp_deactive,
    .pdp_auto_reactive_enable       = tkl_cellular_mds_pdp_auto_reactive,
    .registr_mds_net_notify         = tkl_cellular_mds_register_state_notify,
    .get_ip                         = tkl_cellular_mds_get_ip
};
TKL_CELL_MDS_INTF_T* tkl_cellular_mds_desc_get()
{
    return  &c_cell_mds_desc;
}

#if defined(ENABLE_CELLULAR_CALL) && (ENABLE_CELLULAR_CALL == 1)
TKL_CELL_CALL_INTF_T c_cell_call_desc = {
    .call_service_available = tkl_cellular_call_service_available,
    .call                   = tkl_cellular_call,
    .answer                 = tkl_cellular_call_answer,
    .hungup                 = tkl_cellular_call_hungup,
    .cb_register            = tkl_cellular_call_cb_register,
    .set_callin_mute        = tkl_cellular_call_set_callin_mute,
    .set_volte              = tkl_cellular_call_set_volte,
    .set_voice_mute         = tkl_cellular_call_set_voice_mute,
    .get_voice_mute         = tkl_cellular_call_get_voice_mute,
    .set_voice_vol          = tkl_cellular_call_set_voice_vol,
    .get_voice_vol          = tkl_cellular_call_get_voice_vol,
    .play_tone              = tkl_cellular_call_play_tone,
    .stop_tone              = tkl_cellular_call_stop_tone,
    .dtmf2voice             = tkl_cellular_call_dtmf2voice,
    .reg_KTDetect           = tkl_cellular_call_reg_KTDetect,
    .ctrl_KTDetect          = tkl_cellular_call_ctrl_KTDetect
    .ioctl                  = tkl_cellular_call_ioctl,
};
#else
TKL_CELL_CALL_INTF_T c_cell_call_desc = {
    .call_service_available = NULL,
    .call                   = NULL,
    .answer                 = NULL,
    .hungup                 = NULL,
    .cb_register            = NULL,
    .set_callin_mute        = NULL,
    .set_volte              = NULL,
    .set_voice_mute         = NULL,
    .get_voice_mute         = NULL,
    .set_voice_vol          = NULL,
    .get_voice_vol          = NULL,
    .play_tone              = NULL,
    .stop_tone              = NULL,
    .dtmf2voice             = NULL,
    .reg_KTDetect           = NULL,
    .ctrl_KTDetect          = NULL,
    .ioctl                  = NULL,
};
#endif
TKL_CELL_CALL_INTF_T* tkl_cellular_call_desc_get()
{
    return &c_cell_call_desc;
}

#if defined(ENABLE_CELLULAR_SMS) && (ENABLE_CELLULAR_SMS == 1)
TKL_CELL_SMS_INTF_T c_cell_sms_desc = {
    .send               =  tkl_cellular_sms_send ,
    .recv_cb_register   =  tkl_cellular_sms_recv_cb_register,
    .sms_mute           =  tkl_cellular_sms_mute,
    .convert_str        =  tkl_cellular_sms_convert_str,
    .set_charactor      =  tkl_cellular_sms_set_charactor,
};
#else
TKL_CELL_SMS_INTF_T c_cell_sms_desc = {
    .send               =  NULL ,
    .recv_cb_register   =  NULL,
    .sms_mute           =  NULL,
    .convert_str        =  NULL,
    .set_charactor      =  NULL,
};
#endif
TKL_CELL_SMS_INTF_T* tkl_cellular_sms_desc_get()
{
    return &c_cell_sms_desc;
}
#if defined(ENABLE_CELLULAR_VBAT) && (ENABLE_CELLULAR_VBAT == 1)
TKL_CELL_VBAT_INTF_T c_cell_vbat_desc = {
    .get_rsoc               = tkl_cellular_vbat_get_rsoc,
    .ntc_enable             = tkl_cellular_vbat_ntc_enable,
    .set_charge_current     = tkl_cellular_vbat_set_charge_current,
    .get_charger_state      = tkl_cellular_vbat_get_charger_state,
    .charge_cb_register     = tkl_cellular_vbat_cb_register,
    .get_voltage            = tkl_cellular_vbat_get_voltage,
    .low_volt_poweroff_enable   = tkl_cellular_vbat_low_volt_poweroff_enable
};
#else
TKL_CELL_VBAT_INTF_T c_cell_vbat_desc = {
    .get_rsoc               = NULL,
    .ntc_enable             = NULL,
    .set_charge_current     = NULL,
    .get_charger_state      = NULL,
    .charge_cb_register     = NULL,
    .get_voltage            = NULL,
    .low_volt_poweroff_enable   = NULL
};
#endif
TKL_CELL_VBAT_INTF_T* tkl_cellular_vbat_desc_get()
{
    return &c_cell_vbat_desc;
}

#if defined(ENABLE_CELLULAR_KEYPAD) && (ENABLE_CELLULAR_KEYPAD == 1)
TKL_CELL_KEYPAD_INTF_T  c_cell_keypad_desc = {
    .init                   = tkl_cellular_keypad_init,
    .key_listener_add       = tkl_cellular_keypad_key_listener_add,
    .key_listener_delete    = tkl_cellular_keypad_key_listener_delete,
    .key_state_get          = tkl_cellular_keypad_key_state_get,
    .key_ioctl              = tkl_cellular_keypad_key_ioctl
};
#else
TKL_CELL_KEYPAD_INTF_T  c_cell_keypad_desc = {
    .init                   = NULL,
    .key_listener_add       = NULL,
    .key_listener_delete    = NULL,
    .key_state_get          = NULL,
    .key_ioctl              = NULL
};
#endif

TKL_CELL_KEYPAD_INTF_T* tkl_cellular_keypad_desc_get()
{
    return &c_cell_keypad_desc;
}