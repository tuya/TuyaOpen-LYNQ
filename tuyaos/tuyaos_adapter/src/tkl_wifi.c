#include <stdlib.h>
#include "tkl_wifi.h"
#include "tkl_memory.h"
#include "vlog.h"
#include "ps_lib_api.h"

/**
 * @brief set wifi interface work channel
 *
 * @param[in]       chan        the channel to set
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_wifi_set_cur_channel(CONST UCHAR_T chan)
{
    // Set chan when current channel and the requested channel are not equal
    return OPRT_OK;
}

/**
 * @brief get wifi interface work channel
 *
 * @param[out]      chan        the channel wifi works
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_wifi_get_cur_channel(UCHAR_T* chan)
{
    return OPRT_OK;
}

/**
 * @brief enable / disable wifi sniffer mode.
 *        if wifi sniffer mode is enabled, wifi recv from
 *        packages from the air, and user shoud send these
 *        packages to tuya-sdk with callback <cb>.
 *
 * @param[in]       en          enable or disable
 * @param[in]       cb          notify callback
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_wifi_set_sniffer(CONST BOOL_T en, CONST SNIFFER_CALLBACK cb)
{
    return OPRT_OK;
}

/*
 * Note:
 *   8910 平台下wifi与蓝牙在硬件上共用资源，二者是二选一关系，MAC地址共用
 */

/**
 * @brief get wifi mac info.when wifi works in
 *        ap+station mode, wifi has two macs.
 *
 * @param[in]       wf          wifi function type
 * @param[out]      mac         the mac info
 * @return  OPRT_OK: success  Other: fail
 */
int tkl_wifi_get_mac(const WF_IF_E wf, NW_MAC_S* mac)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief set wifi mac info.when wifi works in
 *        ap+station mode, wifi has two macs.
 *
 * @param[in]       wf          wifi function type
 * @param[in]       mac         the mac info
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_wifi_set_mac(CONST WF_IF_E wf, CONST NW_MAC_S* mac)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief set wifi work mode
 *
 * @param[in]       mode        wifi work mode
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_wifi_set_work_mode(CONST WF_WK_MD_E mode)
{
    return OPRT_OK;
}

/**
 * @brief get wifi work mode
 *
 * @param[out]      mode        wifi work mode
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_wifi_get_work_mode(WF_WK_MD_E* mode)
{
    return OPRT_OK;
}

/**
 * @brief set wifi country code
 *
 * @param[in]       ccode  country code
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_wifi_set_country_code(CONST COUNTRY_CODE_E ccode)
{
    return OPRT_OK;
}

/**
 * @brief send wifi management
 *
 * @param[in]       buf         pointer to buffer
 * @param[in]       len         length of buffer
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_wifi_send_mgnt(CONST UCHAR_T* buf, CONST UINT_T len)
{
    return OPRT_OK;
}

/**
 * @brief register receive wifi management callback
 *
 * @param[in]       enable
 * @param[in]       recv_cb     receive callback
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_wifi_register_recv_mgnt_callback(CONST BOOL_T enable,
        CONST WIFI_REV_MGNT_CB recv_cb)
{
    return OPRT_OK;
}

/**
 * @brief set wifi lowerpower mode
 *
 * @param[in]       en
 * @param[in]       dtim
 * @return  OPRT_OK: success  Other: fail
 */
int tkl_wifi_set_lp_mode(const BOOL_T en, const unsigned char dtim)
{
    return OPRT_OK;
}

/**
 * @brief get wifi rf param exist or not
 *
 * @param[in]       none
 * @return  true: rf param exist  Other: fail
 */
BOOL_T tkl_wifi_set_rf_calibrated(VOID_T)
{
    return OPRT_OK;
}

OPERATE_RET tkl_wifi_init(WIFI_EVENT_CB cb)
{
    return 0;
}

/**
 * @brief get wifi bssid
 *
 * @param[out]      mac         uplink mac
 * @return  OPRT_OK: success  Other: fail
 */
int tkl_wifi_get_bssid(unsigned char* mac)
{
    return 0;
}

OPERATE_RET tkl_wifi_station_connect(CONST SCHAR_T* ssid, CONST SCHAR_T* passwd)
{
    return 0;
}

OPERATE_RET tkl_wifi_station_disconnect(VOID_T)
{
    return 0;
}

OPERATE_RET tkl_wifi_station_get_status(WF_STATION_STAT_E* stat)
{
    return 0;
}

// 判断bssid是否有效用于定位
static bool is_bssid_valid_for_location(const uint8_t *m) {
    // 全 0 / 全 FF
    static const uint8_t zeros[6] = {0};
    static const uint8_t ffs[6]   = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    if (!memcmp(m, zeros, 6) || !memcmp(m, ffs, 6)) return false;

    // 组播位 (I/G) —— BSSID 不可能是组播
    if (m[0] & 0x01) return false;

    // 本地管理位 (U/L) —— 手机热点 / 随机化 MAC,不可用于定位
    if (m[0] & 0x02) return false;

    // IANA 保留段 00:00:5E:xx:xx:xx (VRRP 等虚拟 MAC)
    if (m[0] == 0x00 && m[1] == 0x00 && m[2] == 0x5E) return false;

    return true;
}

OPERATE_RET tkl_wifi_all_ap_scan(AP_IF_S** ap_ary, UINT_T* num)
{
	SetWifiScanParams wifiscanreq = {0};
    GetWifiScanInfo *pWifiScanInfo = NULL;
    AP_IF_S* apinfo = NULL;
    int i = 0;
	OPERATE_RET ret = OPRT_OK;

    // 初始化输出参数
    if (ap_ary) *ap_ary = NULL;
    if (num) *num = 0;

    wifiscanreq.maxTimeOut      = 12000;
    wifiscanreq.round           = 1;
    wifiscanreq.maxBssidNum     = 5;
    wifiscanreq.scanTimeOut     = 5;
    wifiscanreq.wifiPriority    = 0;
    wifiscanreq.channelRecLen   = 280;
    wifiscanreq.channelCount    = 1;

    pWifiScanInfo = (GetWifiScanInfo *)malloc(sizeof(GetWifiScanInfo));
    if(!pWifiScanInfo)
    {
        LOGE("alloc pWifiScanInfo error");
        return OPRT_COM_ERROR;
    }

    appGetWifiScanInfo(&wifiscanreq, pWifiScanInfo);
	if (0 == pWifiScanInfo->bssidNum) {
		LOGE("ap scan result: 0");
		ret = OPRT_COM_ERROR;
		goto EXIT;
	}

	apinfo = (AP_IF_S*)malloc(pWifiScanInfo->bssidNum * sizeof(AP_IF_S));
	if (!apinfo) {
		LOGE("alloc apinfo failed");
		ret = OPRT_COM_ERROR;
		goto EXIT;
	}

	memset(apinfo, 0, pWifiScanInfo->bssidNum * sizeof(AP_IF_S));
	char ssid[128];
    uint8_t valid_ap_num = 0;
    for (i = 0; i < pWifiScanInfo->bssidNum; i++) {
        if(is_bssid_valid_for_location(pWifiScanInfo->bssid[i]) == false) {
            LOGD("invalid ap, ignore it");
            continue;
        }

		memset(ssid, 0, sizeof(ssid));
        apinfo[valid_ap_num].channel = pWifiScanInfo->channel[i];
		apinfo[valid_ap_num].rssi = pWifiScanInfo->rssi[i];
        //应用兼容其他平台，这里bssid取反		
        // memcpy(apinfo[i].bssid, pWifiScanInfo->bssid[i], 6);
        apinfo[valid_ap_num].bssid[0] = pWifiScanInfo->bssid[i][5];
        apinfo[valid_ap_num].bssid[1] = pWifiScanInfo->bssid[i][4];
        apinfo[valid_ap_num].bssid[2] = pWifiScanInfo->bssid[i][3];
        apinfo[valid_ap_num].bssid[3] = pWifiScanInfo->bssid[i][2];
        apinfo[valid_ap_num].bssid[4] = pWifiScanInfo->bssid[i][1];
        apinfo[valid_ap_num].bssid[5] = pWifiScanInfo->bssid[i][0];
		memcpy(apinfo[valid_ap_num].ssid, pWifiScanInfo->ssidHex[i], pWifiScanInfo->ssidHexLen[i]);
		apinfo[valid_ap_num].s_len = pWifiScanInfo->ssidHexLen[i];
		
        // 安全地复制SSID用于日志输出，防止缓冲区溢出
        int copy_len = (apinfo[valid_ap_num].s_len < sizeof(ssid) - 1) ? apinfo[valid_ap_num].s_len : sizeof(ssid) - 1;
        strncpy(ssid, (char*)apinfo[valid_ap_num].ssid, copy_len);
        ssid[copy_len] = '\0';  // 确保字符串结束
        
        LOGI("wifi scan(%d): channel/%d, rssi/%d, ssid/%s", valid_ap_num, apinfo[valid_ap_num].channel, apinfo[valid_ap_num].rssi, ssid);
        LOGI("bssid:%x:%x:%x:%x:%x:%x", apinfo[valid_ap_num].bssid[0], apinfo[valid_ap_num].bssid[1], apinfo[valid_ap_num].bssid[2], apinfo[valid_ap_num].bssid[3], apinfo[valid_ap_num].bssid[4], apinfo[valid_ap_num].bssid[5]);
        valid_ap_num++; 
    }

    if (ret == OPRT_OK) {
        *num = valid_ap_num;
        *ap_ary = apinfo;
        apinfo = NULL; // 防止在EXIT时被释放
    }
EXIT:
    if (pWifiScanInfo) {
        free(pWifiScanInfo);
    }
    if (apinfo) {
        free(apinfo);
    }
    return ret;
}

OPERATE_RET tkl_wifi_get_ip(CONST WF_IF_E wf, NW_IP_S* ip)
{
    return 0;
}

OPERATE_RET tkl_wifi_release_ap(AP_IF_S* ap)
{
    return 0;
}

OPERATE_RET tkl_wifi_start_ap(CONST WF_AP_CFG_IF_S* cfg)
{
    return 0;
}

OPERATE_RET tkl_wifi_stop_ap(VOID_T)
{
    return 0;
}

OPERATE_RET tkl_wifi_get_connected_ap_info(FAST_WF_CONNECTED_AP_INFO_T**
        fast_ap_info)
{
    return 0;
}

OPERATE_RET tkl_wifi_scan_ap(CONST SCHAR_T* ssid, AP_IF_S** ap_ary, UINT_T* num)
{
	LOGI("tkl_wifi_scan_ap call");
    return tkl_wifi_all_ap_scan(ap_ary, num);
}

OPERATE_RET tkl_wifi_station_get_conn_ap_rssi(SCHAR_T* rssi)
{
    return 0;
}

OPERATE_RET tkl_wifi_station_fast_connect(CONST FAST_WF_CONNECTED_AP_INFO_T*
        fast_ap_info)
{
    return 0;
}
