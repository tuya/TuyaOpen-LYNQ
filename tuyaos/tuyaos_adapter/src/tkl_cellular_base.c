#include <stdio.h>
#include <string.h>
#include "tkl_timer.h"
#include "tkl_cellular_comm.h"
#include "tkl_cellular_base.h"
#include "tkl_cellular_mds.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "cmips.h"
#include "ps_lib_api.h"
#include "ps_event_callback.h"
#include "ol_nw_api.h"
#include "ol_time_api.h"
#include "ol_at_api.h"

#include "hal_pwrkey.h"
#include "vlog.h"
#include "ol_sys_api.h"
#include "plat_config.h"
#include "tkl_system.h"

#define PDP_REACTIVE_TIME_INTERVAL   (10 * 1000 * 1000)  // 10s

static uint8_t simcard_hotplug_enable = 0;          //0: 不开启热插拔  1：开启  2：开启并已识别到插入卡后的电平状态
STATIC TKL_SIM_NOTIFY gSimNotify = NULL;
STATIC TKL_REGISTION_NOTIFY gRegNotify = NULL;
TKL_SIM_STATE_E gSimStatus = 0xff;
TUYA_CELLULAR_MDS_STATUS_E gRegStatus = TUYA_CELLULAR_MDS_STATUS_UNKNOWN;

extern void enable_simcard_hotplug(void);
extern OPERATE_RET tkl_cellular_mds_init(UINT8_T sim_id);
extern int get_mds_changed_cid(void);
extern void update_mds_net_status(int cid, TUYA_CELLULAR_MDS_NET_STATUS_E status);

static void set_sim_state(TKL_SIM_STATE_E state)
{
    if (state != gSimStatus) {
        gSimStatus = state;
        LOGI("set sim state: %d", state);

        if (gSimNotify) {
            gSimNotify(state);
        }
    }
}

void update_sim_status(void)
{
    GetPinStateType type = QUERY_CPIN;
    GetPinStateCnfParams param;
    CmsRetId ret;

    ret = appGetPINStateSync(type, &param);
    if (0 != ret) {
        LOGE("update sim status failed, ret: %d", ret);
        if(gSimStatus > TKL_SIM_WAIT_PUK)
            set_sim_state(TKL_NO_SIM);
        return;
    }

    switch (param.cpinCode) {
        case CPIN_READY:
            set_sim_state(TKL_SIM_READY); break;
        case CPIN_SIM_PIN:
        case CPIN_SIM_PUK:
        case CPIN_SIM_NOT_READY:
            set_sim_state(TKL_SIM_INIT); break;
        case CPIN_SIM_UNKNOWN:
        default:
            set_sim_state(TKL_NO_SIM); break;
    }
}

void update_reg_status(CmiCeregStateEnum state)
{
    TUYA_CELLULAR_MDS_STATUS_E lastStatus = gRegStatus;
    switch (state) {
        case CMI_PS_REG_HOME:
        case CMI_PS_REG_ROAMING:
            gRegStatus = TUYA_CELLULAR_MDS_STATUS_REG; break;
        case CMI_PS_REG_DENIED:
            gRegStatus = TUYA_CELLULAR_MDS_STATUS_CAMPED; break;
        case CMI_PS_NOT_REG:
        case CMI_PS_NOT_REG_SEARCHING:
        case CMI_PS_REG_SMS_ONLY_HOME:
        case CMI_PS_REG_SMS_ONLY_ROAMING:
        case CMI_PS_REG_EMERGENCY:
        case CMI_PS_REG_CSFB_NOT_PREFER_HOME:
        case CMI_PS_REG_CSFB_NOT_PREFER_ROAMING:
            gRegStatus = TUYA_CELLULAR_MDS_STATUS_IDLE; break;
        case CMI_PS_REG_UNKNOWN:
        default:
            gRegStatus = TUYA_CELLULAR_MDS_STATUS_UNKNOWN; break;
    }

    LOGI("update reg status, last: %d, new: %d", lastStatus, gRegStatus);
    if (lastStatus != gRegStatus && gRegNotify) {
        gRegNotify(0, gRegStatus);
        LOGI("reg notify status: %d", gRegStatus);
    }
}

TUYA_CELLULAR_MDS_STATUS_E get_reg_status(void)
{
    return gRegStatus;
}

static void app_netPSEventCallback(ol_ps_event_id eventID, void* param, UINT32 paramLen)
{
    CmiSimImsiStr* imsi = NULL;
    CmiPsCeregInd* creg = NULL;
    NmAtiNetInfoInd* netif = NULL;
    UINT8 cid = 0;
    char *event_str = " ";
    switch (eventID)
    {
    case OL_PS_ID_PS_BEARER_ACTED:
        event_str = "activated";
        memcpy(&cid, param, 1);
        LOGI("bearer actived, cid: %d", cid);
        update_mds_net_status(cid, TUYA_CELLULAR_MDS_NET_CONNECT);
    break;
    case OL_PS_ID_PS_BEARER_DEACTED:
        event_str = "deactivated";
        memcpy(&cid, param, 1);
        LOGI("bearer deactived, cid: %d", cid);
        update_mds_net_status(cid, TUYA_CELLULAR_MDS_NET_DISCONNECT);
    break;
    case OL_PS_ID_PS_CEREG_CHANGED:
        event_str = "cereg changed";
        creg = (CmiPsCeregInd*)param;
        LOGI("CREG message act:%d cellId:%d locPresent:%d state:%d, changetype: %d", creg->act, creg->cellId, creg->locPresent, creg->state, creg->changedType);
        if (CMI_CEREG_STATE_CHANGED == creg->changedType) {
            update_reg_status(creg->state);
        }
    break;
    case OL_PS_ID_PS_NETINFO:
        netif = (NmAtiNetInfoInd*) param;
        LOGI("netStatus:%d netifType:%d ipType:%d",netif->netifInfo.netStatus, netif->netifInfo.netifType, netif->netifInfo.ipType);
        // NW_IP_S ip;
        // tkl_cellular_mds_adv_get_ip(0, 1, &ip);
        // LOGI("IP:%s", ip.ip);
    break;
    case OL_PS_ID_SIM_READY:
        event_str = "SIM card is ready";
        set_sim_state(TKL_SIM_READY);
        imsi = (CmiSimImsiStr*)param;
        LOGI("SIM ready IMSI: %s", imsi->contents);
    break;
    case OL_PS_ID_SIM_REMOVED:
        event_str = "SIM card removed";
        set_sim_state(TKL_NO_SIM);
        LOGI("SIM removed");
    break;
    default:
        break;
    }
    LOGI("netPSEventCallback eventID: %d %s", eventID, event_str);
}

OPERATE_RET tkl_cellular_base_init(TKL_CELL_INIT_PARAM_T* param)
{
    static bool init = false;
    if (init) {
        return OPRT_OK;
    }

    ol_ps_event_register(app_netPSEventCallback);
    if(gSimStatus > TKL_SIM_WAIT_PUK)
        update_sim_status();

    if(TKL_SIM_READY == gSimStatus) {
        CeregGetStateParams param_info;
        int ret = appGetCeregStateSync(&param_info);
        update_reg_status(param_info.state);
        LOGI("appGetCeregStateSync ret = %d, rereg_state = %d", ret, param_info.state);
    }
    init = true;
    LOGI("tkl cellular base init success");
    return OPRT_OK;
}

/**
 * @brief 获取当前设备的通讯能力
 * @param ability @TKL_CELLULAR_ABILITY_E 类型
 * @return 0 成功  其它 失败
 */
OPERATE_RET tkl_cellular_base_get_ability(TKL_CELLULAR_ABILITY_E* ability)
{
    if (ability == NULL) {
        LOGE("get ability failed, invalid param");
        return OPRT_INVALID_PARM;
    }

    *ability = SINGLE_SIM_SINGLE_CHANNEL;
    return OPRT_OK;
}

/**
 * @brief 切换当前使能的SIM卡。
 * @param simid SIM卡ID.(0~1)
 * @return 0 成功  其它 失败
 */
OPERATE_RET tkl_cellular_base_switch_sim(UINT8_T sim_id)
{
    LOGE("switch sim(%d) not support", sim_id);
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief 注册SIM状态变化通知函数
 * @param fun 状态变化通知函数
 * @return 0 成功  其它 失败
 */
OPERATE_RET tkl_cellular_base_register_sim_state_notify(UINT8_T simd_id, TKL_SIM_NOTIFY fun)
{
    gSimNotify = fun;
    return OPRT_OK;
}

/**
 * @brief 使能或禁止sim卡热拔插
 *
 * @param simId sim卡ID
 * @param enable TRUE 使能 FALSE 禁止
 *
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_enable_sim_hotplug(UINT8_T sim_id, BOOL_T enable)
{
    extern void simcard_hotplug_ctl(bool enable);
    simcard_hotplug_enable = enable;
    if(false == enable) {
        simcard_hotplug_ctl(enable);
    } else {
        enable_simcard_hotplug();
    }
    return OPRT_OK;
}

bool tkl_cellular_get_sim_hotplug_status(UINT8_T sim_id)
{
    return (simcard_hotplug_enable == 0) ? false : true;
}

/**
 * @brief 获取SIM卡的状态
 * @param simId sim卡ID
 * @param state 1：正常，0：异常，2：初始化中
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_sim_get_status(UINT8_T sim_id, UCHAR_T* state)
{
    if (state == NULL) {
        LOGE("sim get status failed, invalid param");
        return OPRT_OS_ADAPTER_INVALID_PARM;
    }

    *state = gSimStatus;
    LOGI("get sim status: %d", *state);
    return OPRT_OK;
}

/**
 * @brief 获取蜂窝设备当前的通信功能设置
 *
 * @param cfun 获取的通信功能
 *
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_get_cfun_mode(UINT8_T sim_id, PINT_T cfun)
{
    if (cfun == NULL) {
        LOGE("get cfun failed, invalid param");
        return OPRT_INVALID_PARM;
    }

    UINT8 val;
    CmsRetId ret = appGetCFUN(&val);
    if (0 != ret) {
        LOGE("get cfun failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    *cfun = (INT_T)val;
    LOGI("get cfun success, cfun: %d", *cfun);
    return OPRT_OK;
}

/**
 * @brief 设置蜂窝设备的通信功能模式
 *
 * @param cfun 通信功能，取值含义如下：
 *            1：全功能模式
 *            4：飞行模式
 *
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_set_cfun_mode(UINT8_T sim_id, INT_T cfun)
{
    UINT8 val = (UINT8)cfun;
    CmsRetId ret = appSetCFUN(val);
    if (0 == ret) {
        LOGI("set cfun success, cfun: %d", cfun);
        return OPRT_OK;
    } else {
        LOGE("set cfun failed, cfun: %d", cfun);
        return OPRT_COM_ERROR;
    }
}

/**
 * @brief 获取SIM卡中的国际移动用户识别码
 *
 * @param simid,SIM卡id号(0,1,2...)
 * @param imsi识别码，为16字节的字符串
 *
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_get_imsi(UINT8_T sim_id, CHAR_T imsi[15 + 1])
{
    if (!imsi) {
        LOGE("get imsi failed, invalid param");
        return OPRT_INVALID_PARM;
    }

    memset(imsi, 0, 16);
    CmsRetId ret = appGetImsiNumSync(imsi);
    if (0 != ret) {
        LOGE("get imsi failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/**
 * @brief 获取SIM卡的ICCID
 * @param simid
 * @param ICCID识别码，为20字节的字符串
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_get_iccid(UINT8_T sim_id, CHAR_T iccid[20 + 1])
{
    if (!iccid) {
        LOGE("get iccid failed, invalid param");
        return OPRT_INVALID_PARM;
    }

    memset(iccid, 0, 21);
    CmsRetId ret = appGetIccidNumSync(iccid);
    if (0 != ret) {
        LOGE("get iccid failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/**
 * @brief 获取SIM卡所在通道设备的IMEI号
 * @param simid
 * @param IMEI识别码，为15字节的字符串
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_get_imei(UINT8_T sim_id, CHAR_T imei[15 + 1])
{
    if(sim_id != 0) {
        return OPRT_INVALID_PARM;
    }

    if (!imei) {
        LOGE("get imei failed, invalid param");
        return OPRT_INVALID_PARM;
    }

    memset(imei, 0, 16);
    CmsRetId ret = appGetImeiNumSync(imei);
    if (0 != ret) {
        LOGE("get imei failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/**
 * @brief 设置设备的IMEI号
 * @note 该函数目前的适配方法只为适应涂鸦产测写IMEI流程，实际NL668模组的
 *       IMEI号在广和通出厂阶段已完成写入，且广和通不支持外部修改。
 * @param simid
 * @param IMEI识别码，为15字节的字符串
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_set_imei(UINT8_T sim_id, CHAR_T imei[15 + 1])
{
    LOGE("set imei not supported, sim_id: %d, imei: %s", sim_id, imei);
    return OPRT_OK;
}

OPERATE_RET tkl_cellular_base_get_singal_quality(UINT8_T sim_id, cellular_signal_quality_t *quality)
{
    if(quality == NULL) {
        return OPRT_INVALID_PARM;
    }

    UINT8 csq;
    INT8 snr, rsrp, rsrq;
    CmsRetId ret = appGetSignalQualitySync(&csq, &snr, &rsrp, &rsrq);
    if(0 != ret) {
        LOGE("get signal quality failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    quality->csq = csq;
    quality->snr = snr;
    if(rsrp <= 0) {
        quality->rsrp = (INT_T)rsrp - 140;
    } else {
        quality->rsrp = (INT_T)rsrp - 141;
    }

    LOGI("rsrq: %f", rsrq);
    if(rsrq <= 0) {
        quality->rsrq = rsrq * 0.5 - 19.5;
    } else {
        quality->rsrq = rsrq * 0.5 - 20;
    }
    return OPRT_OK;
}

/**
 * @brief 获取SIM卡所在通道蜂窝设备的信号接收功率——单位dbm
 * @param simid
 * @param rsrp 返回实际的信号强度(dbm)
 * @return 0 成功 其它 失败
 */

OPERATE_RET tkl_cellular_base_get_rsrp(UINT8_T sim_id, INT_T* rsrp)
{
    if (rsrp == NULL) {
        LOGE("get rsrp failed, invalid param");
        return OPRT_INVALID_PARM;
    }

    UINT8 csq;
    INT8 snr, trsrp, rsrq;
    CmsRetId ret = appGetSignalQualitySync(&csq, &snr, &trsrp, &rsrq);
    if (0 != ret) {
        LOGE("get rsrp failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    if(trsrp <= 0) {
        *rsrp = (INT_T)trsrp - 140;
    } else {
        *rsrp = (INT_T)trsrp - 141;
    }

    LOGI("csq:%d snr:%d rsrp:%d rsrq:%d", csq, snr, trsrp, rsrq);
    return OPRT_OK;
}

/**
 * @brief 获取蜂窝设备SIM卡所在通道的信号噪声比及误码率
 * @param simid
 * @param sinr 信噪比 （GSM网络无效）
 * @param bit_error (0~7,99) 99无网络
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_get_sinr(UINT8_T sim_id, INT_T* sinr, INT_T* bit_error)
{
    if (!sinr || !bit_error) {
        LOGE("get sinr failed, invalid param");
        return OPRT_INVALID_PARM;
    }

    UINT8 csq;
    INT8 snr, rsrp, rsrq;
    CmsRetId ret = appGetSignalQualitySync(&csq, &snr, &rsrp, &rsrq);
    if (0 != ret) {
        LOGE("get sinr failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    *sinr = (INT_T)snr;
    *bit_error = 0;
    return OPRT_OK;
}

/**
 * @brief SIM卡所在通道LBS的基站信息)
 * @param simid
 * @param lbs 返回基站信息
 * @param neighbour 是否搜索临近基站信息
 * @param timeout 搜索临近基站信息超时时间(一般需要4秒左右)
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_base_get_lbs(UINT8_T sim_id, TKL_LBS_INFO_T* lbs, BOOL_T neighbour, INT_T timeout)
{
    if (lbs == NULL) {
        LOGE("get lbs failed, param lbs NULL");
        return OPRT_INVALID_PARM;
    }

    int cfun;
    OPERATE_RET opret = tkl_cellular_base_get_cfun_mode(0, &cfun);
    if ((OPRT_OK == opret) && ((0 == cfun) || (4 == cfun))) {
        LOGE("cfun 0 or 4 mode, not support");
        return OPRT_COM_ERROR;
    }

    BasicCellListInfo cellinfo;
    int ret = ol_get_cellinfo(&cellinfo);
    if (0 != ret || !cellinfo.sCellPresent) {
        LOGE("get lbs info failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    memset(lbs, 0, sizeof(TKL_LBS_INFO_T));

    char temp[5];
    snprintf(temp, 5, "%03x", cellinfo.sCellInfo.plmn.mcc);
    lbs->mcc[0] = temp[0] - '0';
    lbs->mcc[1] = temp[1] - '0';
    lbs->mcc[2] = temp[2] - '0';
    // LOGI("get lbs, sCell, mcc/%d%d%d", lbs->mcc[0], lbs->mcc[1], lbs->mcc[2]);
    if (cellinfo.sCellInfo.plmn.mncWithAddInfo & 0xf000) {
        snprintf(temp, 5, "%02x", cellinfo.sCellInfo.plmn.mncWithAddInfo & 0x00ff);
        lbs->mnc[0] = temp[0] - '0';
        lbs->mnc[1] = temp[1] - '0';
        lbs->mnc[2] = 0;
        // LOGI("get lbs, sCell, mnc/%d%d", lbs->mnc[0], lbs->mnc[1]);
    } else {
        snprintf(temp, 5, "%03x", cellinfo.sCellInfo.plmn.mncWithAddInfo & 0x0fff);
        lbs->mnc[0] = temp[0] - '0';
        lbs->mnc[1] = temp[1] - '0';
        lbs->mnc[2] = temp[2] - '0';
        // LOGI("get lbs, sCell, mnc/%d%d%d", lbs->mnc[0], lbs->mnc[1], lbs->mnc[2]);
    }

    // service cell
    memcpy(lbs->main.mcc, lbs->mcc, 3);
    memcpy(lbs->main.mnc, lbs->mnc, 3);
    lbs->main.lac = cellinfo.sCellInfo.tac;
    lbs->main.rx_pwr = cellinfo.sCellInfo.rsrp;
    lbs->main.cellid = cellinfo.sCellInfo.cellId;
    // LOGI("get lbs, sCell, lac/%d, rx_pwr/%d, cellId/%d", lbs->main.lac, lbs->main.rx_pwr, lbs->main.cellid);

    if (!neighbour) goto EXIT;

    // neighbour cell
    int neighbour_num = (NEIGHBOUR_NUM < cellinfo.nCellNum) ? NEIGHBOUR_NUM : cellinfo.nCellNum;
    // LOGI("get lbs, neighbour_num/%d", neighbour_num);

    for (int i = 0; i < neighbour_num; i++) {
        TKL_CELL_INFO_T* nbr = &(lbs->neighbour[i]);
        snprintf(temp, 5, "%03x", cellinfo.nCellList[i].plmn.mcc);
        nbr->mcc[0] = temp[0] - '0';
        nbr->mcc[1] = temp[1] - '0';
        nbr->mcc[2] = temp[2] - '0';
        // LOGI("get lbs, nCell, mcc/%d%d%d", nbr->mcc[0], nbr->mcc[1], nbr->mcc[2]);
        if (cellinfo.nCellList[i].plmn.mncWithAddInfo & 0xf000) {
            snprintf(temp, 5, "%02x", cellinfo.nCellList[i].plmn.mncWithAddInfo & 0x00ff);
            nbr->mnc[0] = temp[0] - '0';
            nbr->mnc[1] = temp[1] - '0';
            nbr->mnc[2] = 0;
            // LOGI("get lbs, nCell, mnc/%d%d", nbr->mnc[0], nbr->mnc[1]);
        } else {
            snprintf(temp, 5, "%03x", cellinfo.nCellList[i].plmn.mncWithAddInfo & 0x0fff);
            nbr->mnc[0] = temp[0] - '0';
            nbr->mnc[1] = temp[1] - '0';
            nbr->mnc[2] = temp[2] - '0';
            // LOGI("get lbs, nCell, mnc/%d%d%d", nbr->mnc[0], nbr->mnc[1], nbr->mnc[2]);
        }
        nbr->lac = cellinfo.nCellList[i].tac;
        nbr->rx_pwr = cellinfo.nCellList[i].rsrp;
        nbr->cellid = cellinfo.nCellList[i].cellId;
        // LOGI("get lbs, nCell, lac/%d, rx_pwr/%d, cellId/%d", nbr->lac, nbr->rx_pwr, nbr->cellid);
    }

EXIT:
    return OPRT_OK;
}

/**
 * @brief 获取当前设备的射频校准状态
 * @param
 * @return TRUE正常，FALSE异常
 */

BOOL_T tkl_cellular_base_rf_calibrated(VOID_T)
{
    return TRUE;
}

BOOL_T tkl_cellular_base_enable_sim_detect(UINT8_T simid, BOOL_T enable)
{
    appSetSIMHotSwapNotify(enable);
    return TRUE;
}

INT8_T tkl_cellular_base_get_default_simid(VOID)
{
    return 0;
}

OPERATE_RET tkl_cellular_base_ioctl(INT_T cmd, VOID* argv)
{
    LOGI("base ioctl, cmd: %d", cmd);
    OPERATE_RET ret = OPRT_OK;
    switch (cmd)
    {
    case CELL_IOCTL_SET_PWRKEY_SHUTDOWN_TIME:
    {
        uint32_t timeout = *(uint32_t *)argv;
        if(timeout == 0) {
            pwrKeyDeinit(false);
        } 
    }
    break;
    case CELL_IOCTL_CLOSE_WAKEUP_MODULE:
    {
        extern void close_wakeup_pin(void);
        close_wakeup_pin();
    }
    break;
#if defined(ENABLE_CATX_CALL) && (ENABLE_CATX_CALL == 1)
    case CELL_IOCTL_SET_PA_PIN:
    {
        extern void tkl_audio_pa_pin_set(int pin, bool enable);
        CELL_IOCTL_SET_PA_PIN_T *pin_info = (CELL_IOCTL_SET_PA_PIN_T *)argv;
        tkl_audio_pa_pin_set(pin_info->pin, pin_info->enable_level);
    }
    break;
#endif
    case CELL_IOCTL_GET_SINGAL_QUALITY:
    {
        ret = tkl_cellular_base_get_singal_quality(tkl_cellular_base_get_default_simid(), (cellular_signal_quality_t *)argv);
    }
    break;
    case CELL_IOCTL_GET_CURRENT_OPERATOR:
    {
        GetCurrentOperatorInfo oper_info;
        char *operator_info = (char *)argv;
        if (operator_info == NULL) {
            ret = OPRT_INVALID_PARM;
            break;
        }

        memset(&oper_info, 0, sizeof(oper_info));
        if (0 != appGetCurrentOperatorInfo(&oper_info)) {
            ret = OPRT_COM_ERROR;
            break;
        }
        strncpy(operator_info, (CHAR_T *)oper_info.longPlmn, 32);
    }
    break;
    case CELL_IOCTL_SET_LOGSEL:
    {
        /* AT+ECPCFG="logPortSel",x  x=0->usb  x=1->uart  x=2->usb+uart */
        int new_logsel = *(int *)argv;
        if (new_logsel >= 0 && new_logsel <= 2) {
            int logsel = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_LOG_PORT_SEL);
            if (new_logsel != logsel) {
                LOGD("Set logsel %d", new_logsel);
                BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_LOG_PORT_SEL, new_logsel);
                BSP_SavePlatConfigToFs();
                BSP_SavePlatConfigToRawFlash();
                tkl_system_sleep(300);
                tkl_system_reset();
            }
        } else {
            ret = OPRT_INVALID_PARM;
        }
    }
    break;
    default:
        break;
    }
    return ret;
}

void tuya_cniot_get_sysfw_info(char fw_name[64], char fw_ver[11])
{
    extern void LYNQ_MODULE_GET_NAME(char *name);
    LYNQ_MODULE_GET_NAME(fw_name);
    fw_name[strlen(fw_name)] = 0;
    // strncpy(fw_name, MODULE_NAME, 63);
    // fw_name[63] = 0;

    char *version = ol_get_sw_version();
    memset(fw_ver, 0, 11);
    int ver[3] = {0};
    sscanf(strstr(version, "Y7PM_V") + 6, "%d.%db%d", &ver[0], &ver[1], &ver[2]);
    sprintf(fw_ver, "%d.%d.%d", ver[0], ver[1], ver[2]);
}

VOID tuya_cniot_init(VOID)
{
    char fw_name[64];
    char fw_ver[11];

    tuya_cniot_get_sysfw_info(fw_name, fw_ver);
    LOGI("[SYSVER]        Name: %s", fw_name);
    LOGI("[SYSVER]     Version: %s  %s", ol_get_sw_version(), fw_ver);
    LOGI("[SYSVER] CompileDate: %s %s", __DATE__, __TIME__);

    tkl_cellular_mds_init(0);
    tkl_cellular_base_init(NULL);
}

OPERATE_RET tkl_cellular_base_register_dev_register_notify(UINT8_T simd_id, TKL_REGISTION_NOTIFY fun)
{
    gRegNotify = fun;
    return OPRT_OK;
}
