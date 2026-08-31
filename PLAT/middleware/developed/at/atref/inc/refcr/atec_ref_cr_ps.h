/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ref_cr_ps.h
*
*  Description: Process protocol related AT CMD
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_REF_CR_PS_H_
#define _ATEC_REF_CR_PS_H_

#include "at_util.h"
#include "cmidev.h"

/* AT+MLOCKFREQ */
#define ATC_MFREQ_0_NW_MODE_VAL_MIN                0
#define ATC_MFREQ_0_NW_MODE_VAL_MAX                1
#define ATC_MFREQ_0_NW_MODE_VAL_DEFAULT            0
#define ATC_MFREQ_1_EARFCN_VAL_MIN                 0
#define ATC_MFREQ_1_EARFCN_VAL_MAX                 0x7FFFFFFF
#define ATC_MFREQ_1_EARFCN_VAL_DEFAULT             0
#define ATC_MFREQ_2_PHYCELL_VAL_MIN                0
#define ATC_MFREQ_2_PHYCELL_VAL_MAX                503
#define ATC_MFREQ_2_PHYCELL_VAL_DEFAULT            0

/* AT+MCSEARFCN */
#define ATC_MCSEARFCN_0_NW_MODE_VAL_MIN            0
#define ATC_MCSEARFCN_0_NW_MODE_VAL_MAX            0
#define ATC_MCSEARFCN_0_NW_MODE_VAL_DEFAULT        0


/* AT+MBAND */
#define ATC_MBAND_1_BAND_VAL_MIN                   0
#define ATC_MBAND_1_BAND_VAL_MAX                   255
#define ATC_MBAND_1_BAND_VAL_DEFAULT               0

#define ATC_MUESTATS_0_MAX_PARM_STR_LEN            16
#define ATC_MUESTATS_0_MAX_PARM_STR_DEFAULT        NULL


/* AT+MIPCALL */
#define ATC_MIPCALL_0_STATE_VAL_MIN                 0
#define ATC_MIPCALL_0_STATE_VAL_MAX                 1
#define ATC_MIPCALL_0_STATE_VAL_DEFAULT             1  /* activated */
#define ATC_MIPCALL_1_CID_VAL_MIN                   1
#define ATC_MIPCALL_1_CID_VAL_MAX                   15
#define ATC_MIPCALL_1_CID_VAL_DEFAULT               1

/* AT+MUECONFIG */
#define ATC_MUECONFIG_0_MAX_PARM_STR_LEN            16
#define ATC_MUECONFIG_0_MAX_PARM_STR_DEFAULT        NULL
#define ATC_MUECONFIG_1_AUTOCONN_VAL_MIN            0
#define ATC_MUECONFIG_1_AUTOCONN_VAL_MAX            1
#define ATC_MUECONFIG_1_AUTOCONN_VAL_DEFAULT        0


/*AT+MWIFISCAN */

#define ATC_MWIFISCANCFG_0_MAX_PARM_STR_LEN       16
#define ATC_MWIFISCANCFG_0_MAX_PARM_STR_DEFAULT   NULL

#define ATC_MWIFISCANCFG_1_PRIORITY_VAL_MIN       0   /* lTE Preferred */
#define ATC_MWIFISCANCFG_1_PRIORITY_VAL_MAX       1   /* WIFI Preferred */
#define ATC_MWIFISCANCFG_1_PRIORITY_VAL_DEF       0

#define ATC_MWIFISCANCFG_2_MAX_VAL_MIN            4     //max WIFI scan channel
#define ATC_MWIFISCANCFG_2_MAX_VAL_MAX            10
#define ATC_MWIFISCANCFG_2_MAX_VAL_DEF            5

#define ATC_MWIFISCANCFG_3_CHANNELTIMEOUT_VAL_DEF 280


#define ATC_MWIFISCAN_0_ROUND_VAL_MIN             1 //max WIFI scan round
#define ATC_MWIFISCAN_0_ROUND_VAL_MAX             3
#define ATC_MWIFISCAN_0_ROUND_VAL_DEF             3

#define ATC_MWIFISCAN_1_MAXTIMEOUT_VAL_MIN        10  //WIFI scan  timeout for***N
#define ATC_MWIFISCAN_1_MAXTIMEOUT_VAL_MAX        60
#define ATC_MWIFISCAN_1_MAXTIMEOUT_VAL_DEF        25


#define CMI_REF_CR_MAX_WIFI_BSSID_NUM             ATC_MWIFISCANCFG_2_MAX_VAL_MAX

#define CMI_REF_CR_MAX_SSID_HEX_LENGTH            32

typedef struct CmiRefCrWifiScanRslt_Tag
{
    UINT8   bssidNum;                                   // valid bssid number reported
    //UINT8   ssidHexLen[CMI_REF_CR_MAX_WIFI_BSSID_NUM];        //the length of Wifi SSID Name hex data
    //UINT8   ssidName[CMI_REF_CR_MAX_WIFI_BSSID_NUM][CMI_REF_CR_MAX_SSID_HEX_LENGTH]; //the hex data of WiFi SSID Name
    INT8    rssi[CMI_REF_CR_MAX_WIFI_BSSID_NUM];           // rssi value of scanned bssid
    UINT8   channel[CMI_REF_CR_MAX_WIFI_BSSID_NUM];        // record channel index of bssid, 2412MHz ~ 2472MHz correspoding to 1 ~ 13
    UINT8   bssid[CMI_REF_CR_MAX_WIFI_BSSID_NUM][6];       // mac address is fixed to 6 digits
} CmiRefCrWifiScanRslt;




#ifdef FEATURE_PS_REF_CR_EPS_AT_ENABLE
CmsRetId  refCrPsMCCID(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsICCID(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMLOCKFREQ(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMCSEARFCN(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMBAND(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMSTATS(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMIPCALL(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMWIFISCANCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMWIFISCANSTART(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMWIFISCANSTOP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMWIFISCANQUERY(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMEID(const AtCmdInputContext *pAtCmdReq);
CmsRetId  refCrPsMUECONFIG(const AtCmdInputContext *pAtCmdReq);

#endif

#endif




