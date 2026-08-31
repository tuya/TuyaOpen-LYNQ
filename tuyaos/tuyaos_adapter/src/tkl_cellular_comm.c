#include <string.h>
#include "tkl_cellular_comm.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ps_lib_api.h"
#include "ol_open_api.h"
#include "ol_nw_api.h"
#include "ol_time_api.h"
#include "ol_at_api.h"
#include "vlog.h"

static TKL_VIRTAT_RESP gVirtAtCb;
static TKL_GENERAL_CALLBACK gGenNotify;

extern void tuya_cniot_get_sysfw_info(char fw_name[64], char fw_ver[11]);

//控制AP LOG输出
OPERATE_RET tkl_cellular_comm_ctrl_ap_trace(BOOL_T enable)
{
    return OPRT_NOT_SUPPORTED;
}

//电源键关机功能使能控制
OPERATE_RET tkl_cellular_comm_ctrl_powerkey(BOOL_T enable)
{
    return OPRT_NOT_SUPPORTED;
}

// 控制UART的AT命令服务
OPERATE_RET tkl_cellular_comm_ctrl_at_port(BOOL_T enable)
{
    return OPRT_NOT_SUPPORTED;
}

// 获取RSSI dBm值
OPERATE_RET tkl_cellular_comm_get_rssi_dBm(UINT8_T sim_id, INT_T *rssi_dBm)
{
		if (rssi_dBm == NULL) {
        return OPRT_INVALID_PARM;
    }

	UINT8 csq;
	INT8 snr, rsrp, rsrq;
	CmsRetId ret = appGetSignalQualitySync(&csq, &snr, &rsrp, &rsrq);
    if (ret == 0) {
        if (csq == 31) {
            *rssi_dBm = -50;
        }
        else if (csq > 0 && csq < 31) {
            *rssi_dBm = (csq*2-113);
        }
        else {
            *rssi_dBm = -115;
        }
    }
    return (ret==0)?OPRT_OK:OPRT_NETWORK_ERROR;
}

//获取当前蜂窝设备所在的网络类型
OPERATE_RET tkl_cellular_comm_get_nettype(TUYA_CELLULAR_RAT_E *net_type)
{
	if (!net_type) {
		LOGE("get nettype failed, invalid param");
		return OPRT_INVALID_PARM;
	}

	CeregGetStateParams param;
	CmsRetId ret = appGetCeregStateSync(&param);
	if (0 != ret) {
		LOGE("get nettype failed, ret: %d", ret);
		return OPRT_COM_ERROR;
	}

	if (CMI_PS_REG_HOME == param.state || CMI_PS_REG_ROAMING == param.state) {
		switch (param.act) {
			case CMI_PS_GSM:
			case CMI_PS_GSM_COMPACT:
				*net_type = TUYA_CELL_NET_TYPE_GSM; break;
			case CMI_PS_UMTS:
			case CMI_PS_GSM_EGPRS:
			case CMI_PS_HSDPA:
			case CMI_PS_HSUPA:
			case CMI_PS_HSDPA_HSUPA:
				*net_type = TUYA_CELL_NET_TYPE_3G; break;
			case CMI_PS_LTE:
				*net_type = TUYA_CELL_NET_TYPE_LTE; break;
			case CMI_PS_EC_GSM:
				*net_type = TUYA_CELL_NET_TYPE_CATM; break;
			case CMI_PS_NB_IOT:
				*net_type = TUYA_CELL_NET_TYPE_NB; break;
			default:
				*net_type = TUYA_CELL_NET_TYPE_UNKNOWN; break;
		}
	} else {
		*net_type = TUYA_CELL_NET_TYPE_UNKNOWN;
	}
	LOGI("get nettype  %d", *net_type);
	return OPRT_OK;
}

/*
获取当前蜂窝设备的参考信号质量，参考AT+CSQ
<rssi>:
    0 113 dBm or less
    1 111 dBm
    2. . . 30 109. . . 53 dBm
    31 51 dBm or greater
    99 not known or not detectable
*/
OPERATE_RET tkl_cellular_comm_get_rssi(UINT8_T sim_id, INT_T *rssi)
{
    if (rssi == NULL) {
		LOGE("get rssi failed, invalid param");
        return OPRT_INVALID_PARM;
    }
	
	UINT8 csq;
	INT8 snr, rsrp, rsrq;
	CmsRetId ret = appGetSignalQualitySync(&csq, &snr, &rsrp, &rsrq);
	if (0 != ret) {
		LOGE("get rssi failed, ret: %d", ret);
		return OPRT_COM_ERROR;
	}

	*rssi = (INT_T)csq;

    return OPRT_OK;
}

// 模组进行自检
OPERATE_RET tkl_cellular_comm_selfcheck(SELFTEST_TYPE type, VOID *pdata)
{
    return OPRT_NOT_SUPPORTED;
}

// 获取系统的Epoch 时间，单位秒
OPERATE_RET tkl_cellular_comm_get_epoch_time(INT64_T *epoch_sec)
{
    if (epoch_sec == NULL) {
		LOGE("get epoch time failed, invalid param");
        return OPRT_INVALID_PARM;
    }
	
	*epoch_sec = (INT64_T)ol_time(NULL);
    return OPRT_OK;
}

// 获取本次系统上电后的累计时间，单位ms
OPERATE_RET tkl_cellular_comm_get_poweron_time(INT64_T *up_ms)
{
    return OPRT_NOT_SUPPORTED;
}

// 获取本次系统上电后的累计时间,单位us
OPERATE_RET tkl_cellular_comm_get_poweron_time_us(INT64_T *up_us)
{
    return OPRT_NOT_SUPPORTED;
}

//根据基站同步的时间，获取蜂窝的设备的本地时间
OPERATE_RET tkl_cellular_comm_ctrl_get_localtime(struct tm *local_tm)
{
    if (local_tm == NULL) {
		LOGE("get local time failed, invalid param");
        return OPRT_INVALID_PARM;
    }

	time_t oltime = ol_time(NULL);
	struct tm* ltime = ol_localtime(&oltime);
	memcpy(local_tm, ltime, sizeof(struct tm));

    return OPRT_OK;
}

// 获取底层蜂窝数据上下行统计
OPERATE_RET tkl_cellular_comm_get_data_statics(TKL_CELL_DATA_STATISTICS *stat)
{
    return OPRT_NOT_SUPPORTED;
}

// 控制底层USB
OPERATE_RET tkl_cellular_comm_ctrl_usb(TUYA_CELL_USB_CTRL *ctl)
{
    return OPRT_NOT_SUPPORTED;
}

//根据基站同步的时间，获取时区信息
OPERATE_RET tkl_cellular_comm_get_timezone(INT_T *timezone)
{
    if (timezone == NULL) {
		LOGE("get timezone failed, invalid param");
        return OPRT_INVALID_PARM;
    }
	
	ol_ntp_time_t time;
	int ret = ol_get_time(&time);
	if (0 != ret) {
		LOGE("get timezone failed, ret: %d", ret);
		return OPRT_COM_ERROR;
	}
	
    *timezone = time.timezone / 4;
    LOGI("get time zone: %d", *timezone);
    return OPRT_OK;
}

//获取蜂窝SN号
OPERATE_RET tkl_cellular_comm_get_sn(UINT8_T sim_id, CHAR_T sn[25])
{
	memset(sn, 0, 25);
    BOOL ret = appGetSNNumSync(sn);
	if (0 == ret) {
		LOGE("get sn failed");
		return OPRT_COM_ERROR;
	}
    return OPRT_OK;
}

//设置蜂窝SN号
OPERATE_RET tkl_cellular_comm_set_sn(UINT8_T sim_id, CHAR_T sn[25])
{
	LOGE("set sn not supported");
    return OPRT_OK;
}

//获取基础固件的版本号
OPERATE_RET tkl_cellular_comm_get_sysfw_ver(CHAR_T ver[32])
{
    CHAR_T module[32]={0};
    tuya_cniot_get_sysfw_info(module, ver);

    return OPRT_OK;
}

//获取模组型号
OPERATE_RET tkl_cellular_comm_get_module(CHAR_T module[32])
{
    strncpy(module, "L511-Y7", 10);
    return OPRT_OK;
}

//平台支持的系统备份功能
OPERATE_RET tkl_cellular_comm_sysbackup(VOID)
{
    return OPRT_NOT_SUPPORTED;
}

//设置PLMN
OPERATE_RET tkl_cellular_comm_set_plmn(CHAR_T *plmn)
{
    if (plmn == NULL) {
		LOGE("set plmn failed, invalid param");
        return OPRT_INVALID_PARM;
    }

    CmsRetId ret = appManualPlmnSelect(4, plmn);
	if (0 != ret) {
		LOGE("set plmn failed, ret: %d", ret);
		return OPRT_COM_ERROR;
	}
    return OPRT_OK;
}

OPERATE_RET tkl_cellular_comm_set_virtual_at_recv_cb(TKL_VIRTAT_RESP resp_callback)
{
    gVirtAtCb = resp_callback;
    return OPRT_OK;
}

OPERATE_RET tkl_cellular_comm_send_virtual_at(CHAR_T *at_cmd)
{
	char buf[1024] = {0};
	int ret = ol_at_send_wait_resp(at_cmd, 60, buf, 1024);
	if (0 != ret) {
		LOGE("send virtual at failed, ret: %d", ret);
		return OPRT_COM_ERROR;
	}

	if (gVirtAtCb) {
		gVirtAtCb(buf, strlen(buf));
	}
    return OPRT_OK;
}

OPERATE_RET tkl_cellular_comm_set_gtact(TKL_GTACT_T *gtact)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_cellular_comm_set_general_cb(TKL_GENERAL_CALLBACK general_cb)
{
    gGenNotify = general_cb;
    return OPRT_OK;
}

OPERATE_RET tkl_cellular_comm_get_current_plmn_list(PLMN_LIST_T *plmn_list)
{
    return OPRT_NOT_SUPPORTED;
}
