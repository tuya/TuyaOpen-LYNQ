#include <stdio.h>
#include <string.h>
#include "tkl_cellular_base.h"
#include "tkl_cellular_mds.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ps_lib_api.h"
#include "ps_event_callback.h"
#include "ol_open_api.h"
#include "ol_nw_api.h"
#include "ol_time_api.h"
#include "ol_at_api.h"
#include "vlog.h"
#include "tkl_system.h"
#include "slpman.h"

#define MAX_CID     7
#define APN_LEN_MAX       64
#define USERNAME_LEN_MAX  64
#define PASSWORD_LEN_MAX  64

#define PDP_REQUEST_ACTIVE    1
#define PDP_REQUEST_DEACTIVE  2
#define PDP_ACTIVE_NOTIFY     3
#define PDP_DEACTIVE_NOTIFY   4
#define SIMCARD_REMOVE        5
#define SIMCARD_INSERT        6
#define SIMCARD_HOTPULG_ENABLE 7

#define CHECK_CID_AND_RETURN(cid)    \
    do { \
        if (cid < 1 || cid > MAX_CID - 1) { \
            return OPRT_INVALID_PARM; \
        } \
    } while (0)

typedef struct {
    bool actived;
    bool need_active;
    TUYA_MDS_PDP_TYPE_E pdp;
    char apn[APN_LEN_MAX];
    char username[USERNAME_LEN_MAX];
    char password[PASSWORD_LEN_MAX];
} profile_t;

typedef struct {
    int cmd;
    uint8_t cid;
    TUYA_MDS_PDP_TYPE_E pdp_type;
    char apn[APN_LEN_MAX];
    char username[USERNAME_LEN_MAX];
    char password[PASSWORD_LEN_MAX];
} pdp_request_t;

typedef enum {
    CID_STATUS_IDLE = 0,
    CID_STATUS_ACTIVATING,
    CID_STATUS_ACTIVED,
    CID_STATUS_DEACTIVATING,
    CID_STATUS_DEACTIVED,
} cid_status_e;

TKL_MDS_NOTIFY gMdsNotify = NULL;
BOOL_T gPdpAutoReactive = TRUE;
static osMessageQueueId_t gPdpQueue;
TUYA_CELLULAR_MDS_NET_STATUS_E gMdsNetStatus[MAX_CID];
STATIC profile_t gProfile[MAX_CID] = {0};
STATIC cid_status_e cid_status[MAX_CID] = {0};
extern TKL_SIM_STATE_E gSimStatus;

extern TUYA_CELLULAR_MDS_STATUS_E get_reg_status(void);
static OPERATE_RET pdp_active_proc(UINT8_T cid, TUYA_MDS_PDP_TYPE_E pdp_type,
                            PCHAR_T apn, PCHAR_T username, PCHAR_T password);
static OPERATE_RET pdp_deactive_proc(UINT8_T cid);
OPERATE_RET tkl_cellular_mds_adv_pdp_active(UINT8_T sim_id, UINT8_T cid,
                                            TUYA_MDS_PDP_TYPE_E pdp_type,
                                            PCHAR_T apn, PCHAR_T username,
                                            PCHAR_T password);


extern void simcard_hotplug_ctl(bool enable);
extern void update_sim_status(void);
void simcard_plug_handle(bool state)
{
    pdp_request_t msg;
    if(state) {
        msg.cmd = SIMCARD_INSERT;
    } else {
        msg.cmd = SIMCARD_REMOVE;
    }
    LOGI("simcard state %d", state);
    osMessageQueuePut(gPdpQueue, &msg, 0, 0);
}

void enable_simcard_hotplug(void)
{
    pdp_request_t msg;
    msg.cmd = SIMCARD_HOTPULG_ENABLE;
    osMessageQueuePut(gPdpQueue, &msg, 0, 0);
}

int get_mds_changed_cid(void)
{
    for (int i = 1; i < MAX_CID; i++) {
        ol_cid_index_enum olCid = i - 1 + OL_CID_INDEX_1;
        ol_cid_active_state_enum act;
        int ret = ol_get_cid_status(olCid, &act);
        if (0 != ret) {
            LOGE("get cid status failed, cid: %d", olCid);
            continue;
        }

        LOGI("get cid status, cid/%d, act/%d, actived/%d", i, act, gProfile[i].actived);
        if (OL_CID_ACTIVE == act && false == gProfile[i].actived) {
            return i;
        }

        if (OL_CID_DEACTIVE == act && true == gProfile[i].actived) {
            return i;
        }
    }

    LOGE("get mds changed failed");
    return -1;
}

void update_mds_net_status(int cid, TUYA_CELLULAR_MDS_NET_STATUS_E status)
{
    if (cid < 1 || cid > MAX_CID - 1) {
        LOGE("update mds net status failed, invalid cid: %d", cid);
        return;
    }
#if 0
    gMdsNetStatus[cid] = status;
    if (1 == cid && gMdsNotify) {
        gMdsNotify(0, gMdsNetStatus[1]);
    }

    if (TUYA_CELLULAR_MDS_NET_CONNECT == status) {
        gProfile[cid].actived = true;
    } else {
        gProfile[cid].actived = false;
        gProfile[cid].need_active = gPdpAutoReactive ? true : false;
    }
    LOGI("update mds net status, cid/%d, actived/%d", cid, gProfile[cid].actived);
#else
    pdp_request_t msg;
    msg.cmd = TUYA_CELLULAR_MDS_NET_CONNECT == status ? PDP_ACTIVE_NOTIFY : PDP_DEACTIVE_NOTIFY;
    msg.cid = cid;

    if (osOK != osMessageQueuePut(gPdpQueue, &msg, 0, 0)) {
        LOGE("update_mds_net_status %d notify failed", status);
    }
#endif
}

TUYA_CELLULAR_MDS_NET_STATUS_E get_mds_net_status(int cid)
{
    if (cid < 1 || cid > MAX_CID - 1) {
        LOGE("get mds net status failed, invalid cid: %d", cid);
        return TUYA_CELLULAR_MDS_NET_DISCONNECT;
    }
    
    return gMdsNetStatus[cid];
}

void pdp_active_deactve(void *arg)
{
    pdp_request_t msg;

    while (1) {
        osStatus_t status = osMessageQueueGet(gPdpQueue, &msg, NULL, osWaitForever);
        if (osOK == status) {
            switch(msg.cmd)
            {
                case PDP_REQUEST_ACTIVE:
                    pdp_active_proc(msg.cid, msg.pdp_type, msg.apn, msg.username, msg.password);
                break;
                case PDP_REQUEST_DEACTIVE:
                    pdp_deactive_proc(msg.cid);
                break;
                case PDP_ACTIVE_NOTIFY:
                {
                    //驻网事件上报前会先判断是否已pdp激活，若已激活，则会忽略 驻网事件上报，知道驻网事件没更新，在这里进行强制上报
                    extern void update_reg_status(CmiCeregStateEnum state);
                    TUYA_CELLULAR_MDS_STATUS_E reg_status = get_reg_status();
                    if(TUYA_CELLULAR_MDS_STATUS_REG != reg_status) {
                        LOGI("force to set reg status");
                        update_reg_status(CMI_PS_REG_HOME);
                    }

                    ol_set_dns(1,"114.114.114.114","8.8.8.8");
                    gMdsNetStatus[msg.cid] = TUYA_CELLULAR_MDS_NET_CONNECT;
                    gProfile[msg.cid].actived = true;
                    LOGI("update mds net status, cid/%d, actived/%d", msg.cid, gProfile[msg.cid].actived);
                    if (1 == msg.cid && gMdsNotify) {
                        gMdsNotify(0, gMdsNetStatus[1]);
                    }
                }
                break;
                case PDP_DEACTIVE_NOTIFY:
                    gMdsNetStatus[msg.cid] = TUYA_CELLULAR_MDS_NET_DISCONNECT;
                    gProfile[msg.cid].actived = false;
                    if (1 == msg.cid && gMdsNotify) {
                        gMdsNotify(0, gMdsNetStatus[1]);
                    }
                break;
                case SIMCARD_REMOVE:
                    appSetSIMHotSwapNotify(false);
                    tkl_system_sleep(200);
                    simcard_hotplug_ctl(true);
                break;
                case SIMCARD_INSERT:
                    appSetSIMHotSwapNotify(true);
                    tkl_system_sleep(200);
                    simcard_hotplug_ctl(true);
                break;
                case SIMCARD_HOTPULG_ENABLE:
                {
                    extern void set_sim_insert_level(bool state);
                    bool level = (bool)((slpManGetWakeupPinValue() >> 1) & 0x1);     //获取当前sim det引脚状态
                    update_sim_status();
                    if(gSimStatus == TKL_NO_SIM) {
                        set_sim_insert_level(!level);
                    } else {
                        set_sim_insert_level(level);
                    }
                    LOGI("sim hotplug %d gSimStatus:%d sim_det:%d", true, gSimStatus, level);    
                    simcard_hotplug_ctl(true);
                }
                break;
            }
        }
    }
}

void pdp_reactive(void *arg)
{
    while (1) {
        for(int i = 0;i<3;i++) {
            osDelay(10 * 1000);
            if(gMdsNetStatus[1] != TUYA_CELLULAR_MDS_NET_CONNECT) {
                NW_IP_S ip;
                if(tkl_cellular_mds_adv_get_ip(tkl_cellular_base_get_default_simid(), 1, &ip) == OPRT_OK) {
                    if (ip.ip[0] != '\0' && strcmp(ip.ip, "0.0.0.0") != 0) {
                        update_mds_net_status(1, TUYA_CELLULAR_MDS_NET_CONNECT);
                    }
                }   
            }
        }
        
        for (int i = 1; i < MAX_CID; i++) {
            if (false == gProfile[i].actived && gProfile[i].need_active) {
                LOGI("pdp reactive, cid: %d", i);
                tkl_cellular_mds_adv_pdp_active(0, i, gProfile[i].pdp, gProfile[i].apn, gProfile[i].username, gProfile[i].password);
            }
        }
    }
}

/**
 * @brief 初始化蜂窝移动数据服务
 *
 * @param simId sim卡ID
 *
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_mds_init(UINT8_T sim_id)
{
    static bool init = false;

    if (init) {
        return OPRT_OK;
    }

    for (int i = 0; i < MAX_CID; i++) {
        gMdsNetStatus[i] = TUYA_CELLULAR_MDS_NET_DISCONNECT;
        gProfile[i].actived = false;
        gProfile[i].need_active = false;
        gProfile[i].pdp = TUYA_MDS_PDP_IPV4;
        gProfile[i].apn[0] = 0;
        gProfile[i].username[0] = 0;
        gProfile[i].password[0] = 0;
    }

    for (int i = 0; i < MAX_CID; i++) {
        cid_status[i] = CID_STATUS_IDLE;
    }

    gPdpQueue = osMessageQueueNew(5, (uint32_t)sizeof(pdp_request_t), NULL);
    if (NULL == gPdpQueue) {
        LOGE("create pdp queue failed");
        return OPRT_COM_ERROR;
    }

    osThreadAttr_t task_attr;
    memset(&task_attr, 0, sizeof(task_attr));
    task_attr.name = "pdp task";
    task_attr.stack_size = 4 * 1024;
    task_attr.priority = osPriorityNormal;
    osThreadNew(pdp_active_deactve, NULL, &task_attr);

    memset(&task_attr, 0, sizeof(task_attr));
    task_attr.name = "pdp reactive";
    task_attr.stack_size = 4 * 1024;
    task_attr.priority = osPriorityNormal;
    osThreadNew(pdp_reactive, NULL, &task_attr);

    init = true;
    LOGI("mds init success");
    return OPRT_OK;
}

/**
 * @brief 获取蜂窝移动数据服务的鉴权状态
 *
 * @param simId sim卡ID
 *
 * @return 蜂窝移动数据鉴权状态，查看 @TUYA_CELLULAR_MDS_STATUS_E 定义
 */
TUYA_CELLULAR_MDS_STATUS_E tkl_cellular_mds_adv_get_status(UINT8_T sim_id, UINT8_T cid)
{
    TUYA_CELLULAR_MDS_STATUS_E reg_status = get_reg_status();
    TUYA_CELLULAR_MDS_NET_STATUS_E net_status = get_mds_net_status(cid);

    if ((TUYA_CELLULAR_MDS_STATUS_REG == reg_status) &&
        (TUYA_CELLULAR_MDS_NET_CONNECT == net_status)) {
            return TUYA_CELLULAR_MDS_STATUS_ACTIVE;
    }

    return reg_status;
}

/**
 * @brief 获取蜂窝移动数据服务的鉴权状态
 *
 * @param simId sim卡ID
 *
 * @return 蜂窝移动数据鉴权状态，查看 @TUYA_CELLULAR_MDS_STATUS_E 定义
 */
TUYA_CELLULAR_MDS_STATUS_E tkl_cellular_mds_get_status(UINT8_T sim_id)
{
    return tkl_cellular_mds_adv_get_status(sim_id, 1);
}

static OPERATE_RET pdp_active_proc(UINT8_T cid, TUYA_MDS_PDP_TYPE_E pdp_type, PCHAR_T apn, PCHAR_T username, PCHAR_T password)
{
    int ret;
    ol_cid_index_enum olCid;
    ol_iptype_enum olType;
    char* olApn = apn;
    char* olUsername = username;
    char* olPassword = password;

    CHECK_CID_AND_RETURN(cid);

    if (CID_STATUS_ACTIVATING == cid_status[cid] || CID_STATUS_DEACTIVATING == cid_status[cid]) {
        LOGE("pdp active proc exit, cid/%d", cid);
        return OPRT_COM_ERROR;
    }
    cid_status[cid] = CID_STATUS_ACTIVATING;

    LOGI("pdp active proc, cid: %d, apn: %s, username: %s, password: %s", 
          cid, 
          (NULL == apn ? "(null)" : apn),
          (NULL == username ? "(null)" : username),
          (NULL == password ? "(null)" : password));

    olCid = cid - 1 + OL_CID_INDEX_1;

    gProfile[cid].pdp = pdp_type;
    strncpy(gProfile[cid].apn, olApn, APN_LEN_MAX);
    strncpy(gProfile[cid].username, olUsername, USERNAME_LEN_MAX);
    strncpy(gProfile[cid].password, olPassword, PASSWORD_LEN_MAX);

    char old_apn[32];
    memset(old_apn, 0, sizeof(old_apn));

    ret = ol_cid_get(olCid, &olType, old_apn);
    LOGD("ol_cid_get: %d ret:%d old_apn:%s olType;%d", olCid, ret, old_apn, olType);
    if(!ret) {
        if(strlen(olApn) == 0 ){
            if(strlen(old_apn) == 0) {
                return OPRT_OK;
            }
        } else {
            if(strncmp(olApn, old_apn, strlen(olApn)) == 0) {
                return OPRT_OK;
            }
        }
    }

    if (OL_CID_INDEX_1 != olCid) {
        ret = ol_set_cid_status(olCid, OL_CID_DEACTIVE);
        LOGD("active ol_set_cid_status cid:%d ret = %d",olCid, ret);
    }

    /* 
     *模组默认工作在全功能模式，该模式下系统自动激活CID1，
     *需先配置apn，在设置最小工作模式，然后恢复
     */
    olType = pdp_type + OL_IPTYPE_V4;
    LOGD("call ol_cid_set, cid:%d, pdp:%d, apn:%s", olCid, olType, olApn);
    ret = ol_cid_set(olCid, olType, olApn);
    if (0 != ret) {
        LOGE("pdp active failed, set apn ret: %d", ret);
        ret = OPRT_COM_ERROR;
        goto EXIT;
    }

    if(olUsername && strlen(olUsername) && olPassword && strlen(olPassword) ) {
        LOGD("call ol_auth_set, cid:%d, user:%s, password:%s", olCid, olUsername, olPassword);
        ret = ol_auth_set(olCid, OL_AUTH_PAP_CHAP, olUsername, olPassword);
        if (0 != ret) {
            LOGE("pdp active failed, set auth ret: %d", ret);
            ret = OPRT_COM_ERROR;
            goto EXIT;
        }
    }
    
    if (OL_CID_INDEX_1 == olCid) {
        LOGD("call appSetCFUN");
        appSetCFUN(CMI_DEV_MIN_FUNC);
        osDelay(2000);
        appSetCFUN(CMI_DEV_FULL_FUNC);
    } else {
        ret = ol_set_cid_status(olCid, OL_CID_ACTIVE);
        LOGD("active ol_set_cid_status cid:%d ret = %d",olCid, ret);
    }

    cid_status[cid] = CID_STATUS_ACTIVED;
    LOGI("pdp active success, cid:%d, type:%d, apn:%s, username:%s, password:%s", cid, pdp_type, olApn, olUsername, olPassword);
    return OPRT_OK;

EXIT:
    cid_status[cid] = CID_STATUS_IDLE;
    LOGE("pdp active failed, cid:%d, type:%d, apn:%s", cid, pdp_type, olApn);
    return ret;
}

/**
 * @brief 蜂窝移动数据指定CID PDP激活
 *
 * @param simId sim卡ID
 * @param cid Specify the PDP Context Identifier
 * @param apn 运营商APN设置
 * @param username 用户名
 * @param password 密码
 *
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_mds_adv_pdp_active(UINT8_T sim_id, UINT8_T cid,
                                            TUYA_MDS_PDP_TYPE_E pdp_type,
                                            PCHAR_T apn, PCHAR_T username,
                                            PCHAR_T password)
{
    pdp_request_t msg;
    msg.cmd = PDP_REQUEST_ACTIVE;
    msg.cid = cid;
    msg.pdp_type = pdp_type;
    if (NULL == apn || 0 == strlen(apn)) {
        msg.apn[0] = 0;
    } else {
        strncpy(msg.apn, apn, APN_LEN_MAX - 1);
        msg.apn[APN_LEN_MAX - 1] = 0;
    }

    if (NULL == username || 0 == strlen(username)) {
        msg.username[0] = 0;
    } else {
        strncpy(msg.username, username, USERNAME_LEN_MAX - 1);
        msg.username[USERNAME_LEN_MAX - 1] = 0;
    }

    if (NULL == password || 0 == strlen(password)) {
        msg.password[0] = 0;
    } else {
        strncpy(msg.password, password, PASSWORD_LEN_MAX - 1);
        msg.password[PASSWORD_LEN_MAX - 1] = 0;
    }

    osStatus_t status = osMessageQueuePut(gPdpQueue, &msg, 0, 0);
    if (osOK != status) {
        LOGE("pdp_active failed");
        return OPRT_COM_ERROR;
    }

    LOGI("mds pdp active, cid: %d, apn: %s, username: %s, password: %s", 
          cid, 
          (NULL == apn ? "(null)" : apn),
          (NULL == username ? "(null)" : username),
          (NULL == password ? "(null)" : password));

    return OPRT_OK;
}

/**
 * @brief 蜂窝移动数据PDP激活，默认使用CID为1
 *
 * @param simId sim卡ID
 * @param apn 运营商APN设置
 * @param username 用户名
 * @param password 密码
 *
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_mds_pdp_active(UINT8_T sim_id, PCHAR_T apn, PCHAR_T username, PCHAR_T password)
{
    return tkl_cellular_mds_adv_pdp_active(sim_id, 1, TUYA_MDS_PDP_IPV4, apn, username, password);
}

static OPERATE_RET pdp_deactive_proc(UINT8_T cid)
{
    CmsRetId cmsRet;
    int ret;
    UINT8 orgCfun = CMI_DEV_FULL_FUNC;

    CHECK_CID_AND_RETURN(cid);

    if (CID_STATUS_ACTIVATING == cid_status[cid] ||
        CID_STATUS_DEACTIVATING == cid_status[cid]) {
        LOGE("pdp deactive proc exit, cid/%d", cid);
        return OPRT_COM_ERROR;
    }
    cid_status[cid] = CID_STATUS_DEACTIVATING;

    LOGI("pdp deactive proc, cid: %d", cid);

    ol_cid_index_enum olCid = cid - 1 + OL_CID_INDEX_1;
    ol_cid_active_state_enum act;

    /*
     *模组默认工作在全功能模式，该模式下系统自动激活CID1，
     *去激活CID1需要先设置最小功能模式，否则去激活会失败
     */
    if (OL_CID_INDEX_1 == olCid) {
        LOGD("call appGetCFUN");
        cmsRet = appGetCFUN(&orgCfun);
        if (0 != cmsRet) {
            cid_status[cid] = CID_STATUS_IDLE;
            LOGE("get original cfun failed: %d", cmsRet);
            return OPRT_COM_ERROR;
        }

        LOGD("call appSetCFUN: CMI_DEV_MIN_FUNC");
        cmsRet = appSetCFUN(CMI_DEV_MIN_FUNC);
        if (0 != cmsRet) {
            cid_status[cid] = CID_STATUS_IDLE;
            LOGE("set cfun to CMI_DEV_MIN_FUNC failed, ret: %d", cmsRet);
            return OPRT_COM_ERROR;
        }
    }

    LOGD("call ol_get_cid_status, cid/%d", olCid);
    ret = ol_get_cid_status(olCid, &act);
    if (0 != ret) {
        LOGE("pdp deactive failed, get cid(%d) status ret: %d", cid, ret);
        ret = OPRT_COM_ERROR;
        goto EXIT;
    }

    if (OL_CID_DEACTIVE == act) {
        LOGI("pdp already deactived, cid: %d", cid);
        ret = OPRT_OK;
        goto EXIT;
    }

    LOGD("call ol_set_cid_status, cid/%d, OL_CID_DEACTIVE", olCid);
    ret = ol_set_cid_status(olCid, OL_CID_DEACTIVE);
    if (0 != ret) {
        LOGE("pdp deactive failed, ret: %d", ret);
        ret = OPRT_COM_ERROR;
        goto EXIT;
    }

    ret = OPRT_OK;
    LOGI("pdp deactive success, cid/%d", cid);

EXIT:
    cid_status[cid] = (OPRT_OK == ret) ? CID_STATUS_DEACTIVED : CID_STATUS_IDLE;

    if (OL_CID_INDEX_1 == olCid) {
        LOGD("call appSetCFUN, orgCfun/%d", orgCfun);
        cmsRet = appSetCFUN(orgCfun);
        if (0 != cmsRet) {
            LOGE("set cfun to %d failed, ret: %d", orgCfun, cmsRet);
            return OPRT_COM_ERROR;
        }
    }
    return ret;
}

/**
 * @brief 蜂窝移动数据指定CID PDP去激活
 *
 * @param simId sim卡ID
 * @param cid Specify the PDP Context Identifier
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_mds_adv_pdp_deactive(UINT8_T sim_id, UINT8_T cid)
{
    pdp_request_t msg;
    msg.cmd = PDP_REQUEST_DEACTIVE;
    msg.cid = cid;

    osStatus_t status = osMessageQueuePut(gPdpQueue, &msg, 0, 0);
    if (osOK != status) {
        LOGE("pdp_deactive failed");
        return OPRT_COM_ERROR;
    }

    LOGI("mds pdp deactive, cid: %d", cid);
    return OPRT_OK;
}

/**
 * @brief 蜂窝移动数据PDP去激活，默认使用CID为1
 *
 * @param simId sim卡ID
 *
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_mds_pdp_deactive(UINT8_T sim_id)
{
    return  tkl_cellular_mds_adv_pdp_deactive(sim_id, 1);
}

/**
 * @brief 蜂窝移动数据PDP自动重激活设置
 *
 * @param simId sim卡ID
 * @param enable TRUE 开启自动重新激活 FALSE 关闭自动重新激活
 *
 * @return 0 成功 其它 失败
 */
OPERATE_RET tkl_cellular_mds_pdp_auto_reactive(UINT8_T sim_id, BOOL_T enable)
{
    gPdpAutoReactive = enable;
    LOGI("set pdp auto reactive, sim/%d, enable/%d", sim_id, enable);
    return OPRT_OK;
}

/**
 * @brief 注册蜂窝数据服务状态变化通知函数
 * @param fun 状态变化通知函数
 * @return 0 成功  其它 失败
 */
OPERATE_RET tkl_cellular_mds_register_state_notify(UINT8_T sim_id,
                                                   TKL_MDS_NOTIFY fun)
{
    gMdsNotify = fun;
    gMdsNotify(tkl_cellular_base_get_default_simid(), gMdsNetStatus[1]);
    return OPRT_OK;
}

/**
 * @brief   Get device ip address.
 * @param   ip: The type of NW_IP_S
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_cellular_mds_adv_get_ip(UINT8_T sim_id, UINT8_T cid, NW_IP_S* ip)
{
    CHECK_CID_AND_RETURN(cid);

    if (!ip) {
        LOGE("get ip failed, invalid param");
        return OPRT_INVALID_PARM;
    }

    ol_cid_info_struct cidInfo;
    ol_cid_index_enum olCid = cid - 1 + OL_CID_INDEX_1;
    int ret = ol_get_cid_info(olCid, &cidInfo);
    if (0 != ret) {
        LOGE("get ip failed, ret: %d", ret);
        return OPRT_COM_ERROR;
    }

    if (OL_IPTYPE_V4 != cidInfo.iptype && OL_IPTYPE_V4V6 != cidInfo.iptype) {
        LOGE("get ip failed, not ipv4");
        return OPRT_COM_ERROR;
    }

    memset(ip, 0, sizeof(NW_IP_S));
    strncpy(ip->ip, cidInfo.v4_addr, 16);
    ip->ip[15] = 0;

    return OPRT_OK;
}

/**
 * @brief   Get device ip address.
 * @param   ip: The type of NW_IP_S
 * @return  OPRT_OK: success  Other: fail
 */
OPERATE_RET tkl_cellular_mds_get_ip(UINT8_T sim_id, NW_IP_S* ip)
{
    return tkl_cellular_mds_adv_get_ip(0, 1, ip);
}


#if 0
void tkl_mds_apn_test(void)
{
    int ret = 0;
    osDelay(20000);

    ret = ol_cid_set(1,OL_IPTYPE_V4,"ltemobile.apn");
    OL_LOG_INFO("ol_cid_set ret = %d",ret);

    ret = ol_auth_set(1, OL_AUTH_PAP_CHAP, NULL, NULL);

    appSetCFUN(CMI_DEV_MIN_FUNC);
    osDelay(5000);
    appSetCFUN(CMI_DEV_FULL_FUNC);

    while(1)
    {
        osDelay(5000);
    }
}   
#endif
