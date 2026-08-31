#ifndef __IMI_CC_H__
#define __IMI_CC_H__

/*******************************************************************************
 Copyright:      - 2023- Copyrights of EigenComm Ltd.
 File name:      - imicc.h
 Description:    - IMS CC interface
 History:        - 2023/04/10, Original created
******************************************************************************/

/*********************************************************************************
*****************************************************************************
* Includes
*****************************************************************************
*********************************************************************************/
#include "imicomm.h"

/*********************************************************************************
*****************************************************************************
* Macros
*****************************************************************************
*********************************************************************************/
#define IMI_CC_MAX_DIAL_NUMBER_STR_LEN    80//TS24.008 10.5.4.7 called party BCD number digit max 40 octets
#define IMI_CC_MAX_SUB_ADDR_LEN    20//TS24.008 10.5.4.8
#define IMI_CC_MAX_ALPHA_ID_STR_LEN     32
#define IMI_CC_MAX_CURRENT_CALLS    6
#define IMI_CC_MAX_SINGLE_EMG_NUM_STR_LEN  12   //must be equel to IMS_MAX_SINGLE_EMG_NUM_STR_LEN
#define IMI_CC_MAX_EMG_NUMS  20                 //must be equel to IMS_MAX_EMG_NUMS
#define IMI_CC_MAX_TOTAL_EMG_NUM_STR_LEN  256   //must be equel to IMS_EMG_NUM_MAX_STR_LEN
#define IMI_CC_MAX_CALL_NUMBER_STR_LEN    16    //current defined for white list number string length
#define IMI_CC_MAX_CAL_NUM_LIST_SIZE      30    //current defined for white list number size
/******************************************************************************
 *****************************************************************************
 * IMI enum
 *****************************************************************************
******************************************************************************/
typedef enum IMI_CC_PRIM_ID_TAG
{
    IMI_CC_PRIM_BASE = 0,

    IMI_CC_MAKE_CALL_REQ,               /* ImiCcMakeCallReq, ATD<number>[;] */
    IMI_CC_MAKE_CALL_CNF,               /* ImiCcMakeCallCnf */
    IMI_CC_ANSWER_CALL_REQ,             /* ImiCcAnswerCallReq, ATA */
    IMI_CC_ANSWER_CALL_CNF,             /* ImiCcAnswerCallCnf */
    IMI_CC_HANGUP_CALL_REQ,             /* ImiCcHangupCallReq, ATH/AT+CHUP */
    IMI_CC_HANGUP_CALL_CNF,             /* ImiCcHangupCallCnf */
    IMI_CC_LIST_CURRENT_CALL_REQ,       /* ImiCcListCurrentCallReq, AT+CLCC */
    IMI_CC_LIST_CURRENT_CALL_CNF,       /* ImiCcListCurrentCallCnf */

    IMI_CC_SEND_DTMF_STR_REQ,           /* ImiCcSendDtmfStrReq, AT+VTS=<DTMF>[,<duration>] */
    IMI_CC_SEND_DTMF_STR_CNF,           /* ImiCcSendDtmfStrCnf */
    IMI_CC_SET_DTMF_DURATION_REQ,       /* ImiCcSetDtmfDurationReq, AT+VTD=<n> */
    IMI_CC_SET_DTMF_DURATION_CNF,       /* ImiCcSetDtmfDurationCnf */
    IMI_CC_GET_DTMF_DURATION_REQ,       /* ImiCcGetDtmfDurationReq, AT+VTD? */
    IMI_CC_GET_DTMF_DURATION_CNF,       /* ImiCcGetDtmfDurationCnf */

    IMI_CC_SET_EMG_NUM_REQ,
    IMI_CC_SET_EMG_NUM_CNF,
    IMI_CC_GET_EMG_NUM_REQ,
    IMI_CC_GET_EMG_NUM_CNF,

    IMI_CC_SET_WHITE_LIST_NUM_REQ,
    IMI_CC_SET_WHITE_LIST_NUM_CNF,

    /*unsolicited indication*/
    IMI_CC_INCOMING_CALL_IND,           /*ImiCcIncomingCallInd, RING/+CRING*/
    IMI_CC_CONN_LINE_ID_PRE_IND,        /*ImiCcConnLineIdPreInd, +COLP, MO call display remote number*/
    IMI_CC_CALL_LINE_ID_PRE_IND,        /*ImiCcCallLineIdPreInd, +CLIP, MT call display remote number*/
    IMI_CC_CALL_CONNECT_IND,
    IMI_CC_CALL_DISCONNECT_IND,         /*ImiCcCallDisconnectInd, NO CARRIER */
    IMI_CC_CALL_WAITING_IND,            /*ImiCcCallWaitingInd, +CCWA*/
    IMI_CC_CALL_SS_IND,                 /*ImiCcCallSsInd, +ECCSSN*/

    /*internal Ind*/
    IMI_CC_CALL_STATE_IND,

    IMI_CC_PRIM_END    = 0x01ff
}IMI_CC_PRIM_ID;

typedef enum ImiCcResultCodeTag
{
    IMI_CC_SUCC                  = 0, //
    IMI_CC_NO_CARRIER,
    IMI_CC_BUSY
    //IMI_CC_NO_ANSWER ?
}
ImiCcResultCode;

typedef enum ImiCcCallDirectionTag
{
    MO_CALL     = 0,
    MT_CALL     = 1
}
ImiCcCallDirection;

typedef enum ImiCcCallStateTag
{
    IMI_CC_CALL_ACTIVE          = 0,
    IMI_CC_CALL_HELD            = 1,
    IMI_CC_CALL_DIALING_MO      = 2,
    IMI_CC_CALL_ALERTING_MO     = 3,
    IMI_CC_CALL_INCOMING        = 4,
    IMI_CC_CALL_WAITING         = 5,
    IMI_CC_CALL_INVALID         = 0xFF
}
ImiCcCallState;

typedef enum ImiCcCallModeTag
{
    IMI_CC_MODE_VOICE           = 0,
    IMI_CC_MODE_DATA            = 1,
    IMI_CC_MODE_FAX             = 2
}
ImiCcCallMode;

typedef enum ImiCcRingTypeTag
{
    IMI_CC_RING_TYPE_ASYNC                = 0,
    IMI_CC_RING_TYPE_SYNC                 = 1,
    IMI_CC_RING_TYPE_REL_ASYNC            = 2,
    IMI_CC_RING_TYPE_REL_SYNC             = 3,
    IMI_CC_RING_TYPE_FAX                  = 4,
    IMI_CC_RING_TYPE_VOICE                = 5,
    IMI_CC_RING_TYPE_VOICE_VIDEO          = 6
}
ImiCcRingType;

/*
* call number type refer to TS24.008 10.5.4.7
*/
typedef enum ImiCcCallNumTypeTag
{
    IMI_CC_NUM_TYPE_UNKNOWN                 = 129,//unknown type
    IMI_CC_NUM_TYPE_INTER_NUMBER            = 145,//international number
    IMI_CC_NUM_TYPE_NATIONAL_NUMBER         = 161 //national number
}
ImiCcCallNumType;

typedef enum ImiCcCallHangupTypeTag
{
    IMI_CC_HANGUP_ALL               = 0,
    IMI_CC_HANGUP_ALL_HOLD_WAIT     = 1,
    IMI_CC_HANGUP_ONGOING           = 2
}
ImiCcCallHangupType;

typedef enum ImiCcCliValidityTag
{
    IMI_CC_CLI_VALID                = 0,
    IMI_CC_CLI_WITHHELD_BY_ORIG     = 1,//CLI has been withheld by the originator
    IMI_CC_CLI_INVALID_BY_NW        = 2,
    IMI_CC_CLI_INVALID_MO_PAYPHONE  = 3,
    IMI_CC_CLI_INVALID_OTHER        = 4
}
ImiCcCliValidity;

typedef enum ImiCcEmgNumModeTag
{
    IMI_CC_EMG_NUM_QUERY             = 0,
    IMI_CC_EMG_NUM_ADD               = 1,
    IMI_CC_EMG_NUM_DEL               = 2,
}
ImiCcEmgNumMode;

typedef enum ImiCcEmgNumCategoryTag
{
    IMI_CC_EMG_CATEGORY_GENERIC              = 0,
    IMI_CC_EMG_CATEGORY_AMBULANCE            = 1,
    IMI_CC_EMG_CATEGORY_POLICE               = 2,
    IMI_CC_EMG_CATEGORY_FIRE                 = 3,
    IMI_CC_EMG_CATEGORY_MARINE               = 4,
    IMI_CC_EMG_CATEGORY_MOUNTAIN             = 5,
}
ImiCcEmgNumCategory;

typedef enum ImiCcWhiteListNumModeTag
{
    IMI_CC_WL_NUM_DEL               = 0,
    IMI_CC_WL_NUM_ADD               = 1,
    IMI_CC_WL_NUM_QUERY             = 2,
    IMI_CC_WL_NUM_UNKNOWN           = 0xFF
}
ImiCcWhiteListNumMode;

/******************************************************************************
 *****************************************************************************
 * IMI STRUCT
 *****************************************************************************
******************************************************************************/

typedef struct ImiCcCallInfoTag
{
    UINT8                   callId;//call Id number, 1-N, current 1-3?
    UINT8                   direction;//ImiCcCallDirection
    UINT8                   state;//ImiCcCallState
    UINT8                   mode;//ImiCcCallMode
    BOOL                    bMpty;// whether the call is one of multiparty (conference) call parties
    UINT8                   type;//ImiCcCallNumType
    UINT8                   resvd;
    UINT8                   dialNumStrLen;
    CHAR                    dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
}
ImiCcCallInfo;

typedef struct ImiCcCallNumTag
{
    UINT8                   callNumStrLen;
    UINT8                   rsvd1;
    UINT16                  rsvd2;
    CHAR                    callNumStr[IMI_CC_MAX_CALL_NUMBER_STR_LEN];
}
ImiCcCallNum;

/******************************************************************************
 * IMI_CC_MAKE_CALL_REQ
 * ATD<number>
******************************************************************************/
typedef struct ImiCcMakeCallReqTag
{
    UINT8               dialNumStrLen;
    UINT8               clir;
    UINT16              resvd2;
    CHAR                dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN + 1];
}
ImiCcMakeCallReq;

//IMI_CC_MAKE_CALL_CNF
typedef struct ImiCcMakeCallCnfTag
{
    UINT8                   ccRc;//ImiCcResultCode
    UINT8                   resvd1;
    UINT16                  resvd2;
}
ImiCcMakeCallCnf;

/******************************************************************************
 * IMI_CC_ANSWER_CALL_REQ
 * ATA
******************************************************************************/
typedef ImiEmptySig ImiCcAnswerCallReq;

//IMI_CC_ANSWER_CALL_CNF
typedef struct ImiCcAnswerCallCnfTag
{
    UINT8               ccRc;//ImiCcResultCode
    UINT8               resvd1;
    UINT16              resvd2;
}
ImiCcAnswerCallCnf;

/******************************************************************************
 * IMI_CC_HANGUP_CALL_REQ
 * ATH/AT+CHUP
******************************************************************************/
typedef struct ImiCcHangupCallReqTag
{
    UINT8               type;//
    UINT8               resvd1;
    UINT16              resvd2;
}
ImiCcHangupCallReq;

//IMI_CC_HANGUP_CALL_CNF
typedef ImiEmptySig ImiCcHangupCallCnf;

/******************************************************************************
 * IMI_CC_LIST_CURRENT_CALL_REQ
 * AT+CLCC
******************************************************************************/
typedef ImiEmptySig ImiCcListCurrCallReq;

//IMI_CC_LIST_CURRENT_CALL_CNF
typedef struct ImiCcListCurrCallCnfTag
{
    ImiCcCallInfo                   callInfoList[IMI_CC_MAX_CURRENT_CALLS];
}
ImiCcListCurrCallCnf;


/**
 * IMI_CC_SEND_DTMF_STR_REQ
*/
typedef struct ImiCcSendDtmfStrReqTag
{
    UINT16              duration;//ms
    BOOL                durationPresent;
    UINT8               dtmfStrLen;
    UINT8               dtmfStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
}
ImiCcSendDtmfStrReq;

/**
 * IMI_CC_SEND_DTMF_STR_CNF
*/
typedef  ImiEmptySig       ImiCcSendDtmfStrCnf;


/**
 * IMI_CC_SET_DTMF_DURATION_REQ
*/
typedef struct ImiCcSetDtmfDurationReqTag
{
    UINT16              duration;//ms
    UINT16              resvd;
}
ImiCcSetDtmfDurationReq;

/**
 * IMI_CC_SET_DTMF_DURATION_CNF
*/
typedef  ImiEmptySig       ImiCcSetDtmfDurationCnf;

/**
 * IMI_CC_GET_DTMF_DURATION_REQ
*/
typedef  ImiEmptySig       ImiCcGetDtmfDurationReq;

/**
 * IMI_CC_GET_DTMF_DURATION_CNF
*/
typedef struct ImiCcGetDtmfDurationCnfTag
{
    UINT16              duration;//ms
    UINT16              resvd;
}
ImiCcGetDtmfDurationCnf;




/**
 * IMI_CC_INCOMING_CALL_IND
*/
typedef struct ImiCcIncomingCallIndTag
{
    UINT8                   type;//ImiCcRingType
    UINT8                   rsvd1;
    UINT16                  rsvd2;
}
ImiCcIncomingCallInd;


/**
 * IMI_CC_CONN_LINE_ID_PRE_IND
 * +COLP
*/
typedef struct ImiCcConnLineIdPreIndTag
{
    UINT8                   type;//ImiCcCallNumType
    UINT8                   dialNumStrLen;
    UINT8                   saType;
    UINT8                   subAddrStrLen;
    UINT8                   alphaStrLen;
    UINT8                   rsvd1;
    UINT16                  rsvd2;
    CHAR                    dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
    CHAR                    subAddrStr[IMI_CC_MAX_SUB_ADDR_LEN];
    CHAR                    alphaStr[IMI_CC_MAX_ALPHA_ID_STR_LEN];
}
ImiCcConnLineIdPreInd;

/**
 * IMI_CC_CALL_LINE_ID_PRE_IND
 * +CLIP
*/
typedef struct ImiCcCallLineIdPreIndTag
{
    UINT8                   type;//ImiCcCallNumType
    UINT8                   dialNumStrLen;
    UINT8                   saType;
    UINT8                   subAddrStrLen;
    UINT8                   alphaStrLen;
    UINT8                   cliValidity;//ImiCcCliValidity
    UINT16                  rsvd;
    CHAR                    dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
    CHAR                    subAddrStr[IMI_CC_MAX_SUB_ADDR_LEN];
    CHAR                    alphaStr[IMI_CC_MAX_ALPHA_ID_STR_LEN];
}
ImiCcCallLineIdPreInd;

/**
 * IMI_CC_CALL_DISCONNECT_IND
*/
typedef ImiEmptySig ImiCcCallDisconnectInd;

/**
 * IMI_CC_CALL_WAITING_IND
 * +CCWA
*/
typedef struct ImiCcCallWaitingIndTag
{
    BOOL                    bFirstRpt;//whether the first report, call waiting urc is not repeat but RING repeat if call waiting URC disabled
    UINT8                   type;//ImiCcCallNumType
    UINT8                   voiceClass;
    UINT8                   cliValidity;
    UINT8                   dialNumStrLen;
    UINT8                   alphaStrLen;
    UINT16                  resvd2;
    CHAR                    dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
    CHAR                    alphaStr[IMI_CC_MAX_ALPHA_ID_STR_LEN];
}
ImiCcCallWaitingInd;

typedef struct ImiCcSsIndTag
{
    UINT8       callId;
    UINT8       ssId;
}
ImiCcSsInd;

typedef struct ImiCcSetEmgNumReqTag
{
    UINT8               mode;   //ImiCcEmgNumMode
    UINT8               type;
    UINT8               category;
    UINT8               emgNumCount;
    CHAR                emgNum[IMI_CC_MAX_EMG_NUMS][IMI_CC_MAX_SINGLE_EMG_NUM_STR_LEN + 1];
}
ImiCcSetEmgNumReq;

typedef struct ImiCcSetEmgNumCnfTag
{
    UINT8                   ccRc;//ImiCcResultCode
    UINT8                   mode;
    UINT8                   type;
    UINT8                   emgNumCount;
    UINT8                   category[IMI_CC_MAX_EMG_NUMS];
    CHAR                    emgNum[IMI_CC_MAX_EMG_NUMS][IMI_CC_MAX_SINGLE_EMG_NUM_STR_LEN + 1];
}
ImiCcSetEmgNumCnf;

typedef struct ImiCcGetEmgNumReqTag
{
    UINT32              resvd;
}
ImiCcGetEmgNumReq;

typedef struct ImiCcGetEmgNumCnfTag
{
    UINT8                   ccRc;//ImiCcResultCode
    UINT8                   resvd;
    UINT8                   emgNumCount;
    UINT8                   emgNumCountNoSim;
    UINT8                   category[IMI_CC_MAX_EMG_NUMS];
    UINT8                   categoryNoSim[IMI_CC_MAX_EMG_NUMS];
    CHAR                    emgNum[IMI_CC_MAX_EMG_NUMS][IMI_CC_MAX_SINGLE_EMG_NUM_STR_LEN + 1];
    CHAR                    emgNumNoSim[IMI_CC_MAX_EMG_NUMS][IMI_CC_MAX_SINGLE_EMG_NUM_STR_LEN + 1];
}
ImiCcGetEmgNumCnf;

/**
 * IMI_CC_SET_WHITE_LIST_NUM_REQ
*/
typedef struct ImiCcSetWhiteListNumReqTag
{
    UINT8                   mode;   //ImiCcWhiteListNumMode
    UINT8                   callNumCnt; //if mode is QUERY, cnt shall be 0; if mode is DELETE and cnt is 0, delete all number list
    UINT16                  resvd;
    ImiCcCallNum            callNumList[IMI_CC_MAX_CAL_NUM_LIST_SIZE];
}
ImiCcSetWhiteListNumReq;

/**
 * IMI_CC_SET_WHITE_LIST_NUM_CNF
*/
typedef struct ImiCcSetWhiteListNumCnfTag
{
    UINT8                   mode;   //ImiCcWhiteListNumMode
    UINT8                   callNumCnt; //if mode is QUERY, shall return valid value
    UINT16                  resvd;
    ImiCcCallNum            callNumList[IMI_CC_MAX_CAL_NUM_LIST_SIZE];
}
ImiCcSetWhiteListNumCnf;

#endif

