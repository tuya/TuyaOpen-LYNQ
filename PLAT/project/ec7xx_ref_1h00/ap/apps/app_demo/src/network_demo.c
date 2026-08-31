/****************************************************************************
 *
 ****************************************************************************/
#include "stdio.h"
#include "string.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "ps_lib_api.h"
#include "ol_open_api.h"
#include "ol_nw_api.h"
#include "ol_log.h"


void get_cell_info(void)
{
    int i = 0;
    int ret = 0;
    BasicCellListInfo cellinfo;

    memset(&cellinfo, 0x0, sizeof(cellinfo));
    ret = ol_get_cellinfo(&cellinfo);
    if(0 == ret)
    {
        OL_LOG_INFO("Server Cell: mcc=%d, mnc=%d, cellid=%d, tac=%d, phyid=%d", \
                                  cellinfo.sCellInfo.plmn.mcc, \
                                  cellinfo.sCellInfo.plmn.mncWithAddInfo, \
                                  cellinfo.sCellInfo.cellId, \
                                  cellinfo.sCellInfo.tac, \
                                  cellinfo.sCellInfo.phyCellId);

        for(i = 0; i < cellinfo.nCellNum; i++)
        {
            OL_LOG_INFO("Neighbour Cell[%d]: mcc=%d, mnc=%d, cellid=%d, tac=%d, phyid=%d", i, \
                                             cellinfo.nCellList[i].plmn.mcc, \
                                             cellinfo.nCellList[i].plmn.mncWithAddInfo, \
                                             cellinfo.nCellList[i].cellId, \
                                             cellinfo.nCellList[i].tac, \
                                             cellinfo.nCellList[i].phyCellId);
        }
    }
}

void test_get_net_info(void)
{
    int ret = 0;
    char buff[32] = {0};
    CeregGetStateParams param_info;

    OL_LOG_INFO("------test get net info------");

    memset(buff, 0x0, sizeof(buff));
    ret = appGetImeiNumSync(buff);
    OL_LOG_INFO("get IMEI ret=%d, imei=%s", ret, buff);

    memset(buff, 0x0, sizeof(buff));
    ret = appGetIccidNumSync(buff);
    OL_LOG_INFO("get ICCID ret=%d, iccid=%s", ret, buff);

    memset(buff, 0x0, sizeof(buff));
    ret = appGetImsiNumSync(buff);
    OL_LOG_INFO("get IMSI ret=%d, imsi=%s", ret, buff);

    ret = appGetCeregStateSync(&param_info);
    OL_LOG_INFO("get Cereg ret=%d, state=%d", ret, param_info.state);

    get_cell_info();
}

void test_cid_apn_get(ol_cid_index_enum cid)
{
    int ret = 0;
    ol_iptype_enum iptype;
    ol_auth_enum auth_type;
    char apn[32] = {0};
    char usr[64] = {0};
    char pwd[64] = {0};
    ol_cid_info_struct info = {0};
    ol_cid_active_state_enum act_state;

    OL_LOG_INFO("------test cid apn get------");

    ret = ol_get_cid_status(cid, &act_state);
    OL_LOG_INFO("get CID[%d] Active ret=%d, state=%d",cid, ret, act_state);

    if(OL_CID_ACTIVE == act_state)
    {
        ret = ol_cid_get(cid, &iptype, apn);
        OL_LOG_INFO("get APN ret=%d, iptype=%d, apn=%s", ret, iptype, apn);

        ret = ol_get_cid_info(cid, &info);
        OL_LOG_INFO("get IP info ret=%d, iptype=%d, ipv4=%s, ipv6=%s", ret, info.iptype, info.v4_addr, info.v6_addr);

        ret = ol_auth_get(cid, &auth_type, usr, pwd);
        OL_LOG_INFO("get Auth ret=%d, auth_type=%d, usr=%s, pwd=%s",ret, auth_type, usr, pwd);
    }
}

void test_cid_apn_set(ol_cid_index_enum cid)
{
    int ret = 0;
    ol_cid_active_state_enum act_state;

    OL_LOG_INFO("------test cid apn set------");

    ret = ol_cid_set(cid, OL_IPTYPE_V4V6, "test_apn");
    OL_LOG_INFO("set APN ret = %d",ret);

    ret = ol_auth_set(cid, OL_AUTH_PAP_CHAP, "test_usr", "test_pwd");
    OL_LOG_INFO("set Auth ret = %d",ret);

    //change APN need reset network
    appSetCFUN(0);
    osDelay(1000);
    appSetCFUN(1);
    osDelay(3000);

    //If CID is not auto activated, activate manually
    ret = ol_get_cid_status(cid, &act_state);
    if(OL_CID_DEACTIVE == act_state)
    {
        ret = ol_set_cid_status(cid,OL_CID_ACTIVE);
        OL_LOG_INFO("set CID request Active ret = %d",ret);
        osDelay(3000);
    }
}

void test_cfun_restart_no_apn(void)
{
    int ret = 0;
    UINT8 cfun = 0;
    CeregGetStateParams cereg;
    ol_cid_active_state_enum act_state;

    OL_LOG_INFO("------test CFUN restart (no APN rewrite)------");

    ret = appGetCFUN(&cfun);
    OL_LOG_INFO("before CFUN ret=%d, cfun=%d", ret, cfun);

    /* Same sequence as test_cid_apn_set, but do NOT touch APN/auth.
     * Isolates whether attach needs radio restart rather than "test_apn". */
    appSetCFUN(0);
    osDelay(1000);
    appSetCFUN(1);
    osDelay(5000);

    ret = appGetCFUN(&cfun);
    memset(&cereg, 0, sizeof(cereg));
    appGetCeregStateSync(&cereg);
    ret = ol_get_cid_status(OL_CID_INDEX_1, &act_state);
    OL_LOG_INFO("after CFUN cfun=%d cereg=%d cid1=%d/%d", cfun, cereg.state, ret, act_state);

    if (OL_CID_DEACTIVE == act_state) {
        ret = ol_set_cid_status(OL_CID_INDEX_1, OL_CID_ACTIVE);
        OL_LOG_INFO("set CID1 ACTIVE ret=%d", ret);
        osDelay(3000);
        appGetCeregStateSync(&cereg);
        ol_get_cid_status(OL_CID_INDEX_1, &act_state);
        OL_LOG_INFO("after ACTIVE cereg=%d cid1=%d", cereg.state, act_state);
    }
}

void test_dns_server(void)
{
    char pri[128] = {0};
    char sec[128] = {0};

    OL_LOG_INFO("------test dns server------");

    ol_set_dns(OL_IPTYPE_V4,"114.114.114.114","8.8.8.8");
    ol_set_dns(OL_IPTYPE_V6,"240E:56:4000:8000:0:0:0:69","240E:56:4000:0:0:0:0:218");

    ol_get_dns(OL_IPTYPE_V4,pri,sec);
    ol_get_dns(OL_IPTYPE_V6,pri,sec);
    OL_LOG_INFO("DNS IPV4 pri=%s, sec=%s",pri, sec);
    OL_LOG_INFO("DNS IPV6 pri=%s, sec=%s",pri, sec);
}

void wifiscan_demo(void)
{
    SetWifiScanParams wifiscanreq = {0};
    GetWifiScanInfo *pWifiScanInfo = PNULL;
    int i = 0;
    char pRspStr[128] = {0};

    wifiscanreq.maxTimeOut      = 12000;
    wifiscanreq.round           = 1;
    wifiscanreq.maxBssidNum     = 5;
    wifiscanreq.scanTimeOut     = 5;
    wifiscanreq.wifiPriority    = 0;
    wifiscanreq.channelRecLen   = 280;
    wifiscanreq.channelCount    = 1;

    pWifiScanInfo = (GetWifiScanInfo *)OsaAllocZeroMemory(sizeof(GetWifiScanInfo));
    if(pWifiScanInfo == PNULL)
    {
        OL_LOG_INFO("alloc pWifiScanInfo NULL");
        return;
    }

    appGetWifiScanInfo(&wifiscanreq, pWifiScanInfo);
    OL_LOG_INFO("Get Wifi Scan bssidNum: %d",pWifiScanInfo->bssidNum);

    for (i = 0; i < pWifiScanInfo->bssidNum; i++)
    {
        memset(pRspStr, 0, 128);
        snprintf(pRspStr, 128, "WIFISCAN:(-,-,%d,\"%02X:%02X:%02X:%02X:%02X:%02X\",%d)",
             pWifiScanInfo->rssi[i],
             pWifiScanInfo->bssid[i][0],
             pWifiScanInfo->bssid[i][1],
             pWifiScanInfo->bssid[i][2],
             pWifiScanInfo->bssid[i][3],
             pWifiScanInfo->bssid[i][4],
             pWifiScanInfo->bssid[i][5],
             pWifiScanInfo->channel[i]);
        OL_LOG_INFO("%s",pRspStr);
    }

    OsaFreeMemory(&pWifiScanInfo);
    osDelay(2000/portTICK_PERIOD_MS);
}

void network_demo(void)
{
    OL_LOG_INFO("------test network_demo------");

    //Set SIM Notify
    //SIM ready or removed will report by AppPSUrcCallback (in app.c)
    appSetSIMHotSwapNotify(TRUE);
    osDelay(3000);

    test_get_net_info();
    test_cid_apn_get(OL_CID_INDEX_1);

    /* Critical path isolation: CFUN 0->1 without writing test_apn */
    test_cfun_restart_no_apn();

    test_get_net_info();
    test_cid_apn_get(OL_CID_INDEX_1);

    OL_LOG_INFO("------test network_demo END------");
}

