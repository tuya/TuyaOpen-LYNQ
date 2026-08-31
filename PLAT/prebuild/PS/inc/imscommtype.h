#ifndef __IMS_COMM_TYPE_H__
#define __IMS_COMM_TYPE_H__
/*******************************************************************************
 Copyright:      - 2023- Copyrights of EigenComm Ltd.
 File name:      - imscommtype.h
 Description:    - IMS common struct/marco header
 History:        - 2023/04/23, Original created
******************************************************************************/
#include "commontypedef.h"

/******************************************************************************
 *****************************************************************************
 * Marco/Enum
 *****************************************************************************
******************************************************************************/


/*
 * IMS string format, max len: xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
 *  contain last: '\0'
*/
#define IMS_IPV6_STR_LEN            40
#define IMS_IP_MAX_STR_LEN          IMS_IPV6_STR_LEN
#define IMS_REALM_MAX_LEN           48
#define IMS_USER_NAME_MAX_LEN       64     /*impi/nai, eg: 46001000000009@ims.mnc001.mcc460.3gppnework.org*/
#define IMS_IMSI_MAX_LEN            16
#define IMS_IMEI_MAX_LEN            16
#define IMS_PRODUCT_NAME_MAX_LEN    64      /*user-agent header, eg: "SIP/2.0 (EigenComm Ltd)" */
#define IMS_APN_MAX_STR_LEN         100
#define IMS_CELL_ID_MAX_STR_LEN     22
#define IMS_XCAP_URI_MAX_LEN        64
#define IMS_BSF_URI_MAX_LEN         64
#define IMS_MNC_MAX_LEN             4
#define IMS_MCC_MAX_LEN             4

#define IMS_XCAP_ROOT_MAX_LEN        64
#define IMS_XCAP_AUID_MAX_LEN        64
#define IMS_XCAP_CLIENTNAME_MAX_LEN  32

#define IMS_DIGEST_PWD_MAX_LEN      32


#define IMS_USER_IDENTITY_MAX_LEN   128

#define IMS_EMG_NUM_MAX_STR_LEN     256     //must be equel to IMI_CC_MAX_TOTAL_EMG_NUM_STR_LEN
#define IMS_MAX_EMG_NUMS            20       //must be equel to IMI_CC_MAX_EMG_NUMS
#define IMS_MAX_SINGLE_EMG_NUM_STR_LEN  12   //must be equel to IMI_CC_MAX_SINGLE_EMG_NUM_STR_LEN

#define IMS_WHITE_LIST_JSON_MAX_STR_LEN   512
#define IMS_WHITE_LIST_MAX_SIZE        30
#define IMS_WHITE_LIST_NUM_MAX_LEN     16


/*
 * AKA auth related marco
*/
#define IMS_AKA_RAND_LEN    16
#define IMS_AKA_AUTN_LEN    16
#define IMS_AKA_IK_LEN      16
#define IMS_AKA_CK_LEN      16
#define IMS_AKA_RES_LEN     16
#define IMS_AKA_AUTS_LEN    14



#define  IMS_NOR_BR_DEF_CID   (15)
#define  IMS_EMC_BR_DEF_CID   (14)


/**
 * IMS IP type
*/
typedef enum _EPAT_ImsIpType_enum
{
    IMS_IPV4,
    IMS_IPV6,
    IMS_IPV4V6_V4_perfer,   /* used in some IP selection case */
    IMS_IPV4V6_V6_perfer,   /* used in some IP selection case */
}ImsIpType;


/**
 * IMS transport type
*/
typedef enum ImsTpType_enum
{
    IMS_UDP,
    IMS_TCP,
    IMS_UDP_perfer          /* if pkg > MTU size, using TCP, else using UDP */
}ImsTpType;



/**
 * IMS network acctype
*/
typedef enum ImsAccNetTpType_enum
{
    IMS_NET_TDD_TYPE,
    IMS_NET_FDD_TYPE,
}msAccNetTpType;


typedef enum ImsUtAuthType_enum
{
    IMS_UT_AUTH_GBA_DEF      = 0,
    IMS_UT_AUTH_GBA_ME       = IMS_UT_AUTH_GBA_DEF,
    IMS_UT_AUTH_GBA_UICC     = 1,
    IMS_UT_AUTH_GBA_DIGEST   = 2,
    IMS_UT_AUTH_DIGEST       = 3,
    IMS_UT_AUTH_NULL         = 4,
}ImsUtAuthType;


/**
 * PLMN oper marco
*/
#define IMS_GET_PURE_MNC(MNCWITHADDINFO)    ((MNCWITHADDINFO)&0xFFF)
#define IMS_IS_2_DIGIT_MNC(MNCWITHADDINFO)  (((MNCWITHADDINFO)>>12) == 0x0F)
#define IMS_SET_MNC_WITH_ADD_INFO(TWODIGType, pureMNC)  ((TWODIGType)?((pureMNC)|0xF000):((pureMNC)&0xFFF))
#define IMS_IS_VALID_PLMN(LPLMN)            (((LPLMN).mcc != 0) ? TRUE : FALSE)
#define IMS_IS_SAME_PLMN(PLMN1, PLMN2)      (((PLMN1).mcc == (PLMN2).mcc) && ((PLMN1).mncWithAddInfo == (PLMN2).mncWithAddInfo))


/******************************************************************************
 *****************************************************************************
 * Struct
 *****************************************************************************
******************************************************************************/

/**
 * Numberic PLMN format, 4 bytes,
 * example: CMCC: 46000; mcc = 0x0460, mnc = 0xf000
*/
typedef struct ImsPlmn_Tag
{
    UINT16 mcc;
    UINT16 mncWithAddInfo;  // if 2-digit MNC type, the 4 MSB bits should set to 'F',
}ImsPlmn;




/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************
******************************************************************************/

#endif

