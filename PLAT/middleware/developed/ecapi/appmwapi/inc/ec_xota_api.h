/******************************************************************************
 * (C) Copyright 2018 EIGENCOMM International Ltd.
 * All Rights Reserved
*******************************************************************************
 *  Filename: ec_xota_api.h
 *
 *  Description: API interface implementation header file for adc service
 *
 *  History:
 *
 *  Notes:
 *
******************************************************************************/
#ifndef __EC_XOTA_API_H__
#define __EC_XOTA_API_H__

#include <stdint.h>

#define XOTA_URC_BUF_MAXLEN         128

//errno
typedef enum
{
    REF_CR_FOTA_EUNDEF = 850,
    REF_CR_FOTA_ECONFIG,
    REF_CR_FOTA_EURL,
    REF_CR_FOTA_EFWWR,
    REF_CR_FOTA_EFWUP,
    REF_CR_FOTA_EFWDL,
    REF_CR_FOTA_EMAXTRY,
    REF_CR_FOTA_ENETWORK,

    /* extended */
    REF_CR_FOTA_EPATMODE,
    REF_CR_FOTA_EALGO,
    REF_CR_FOTA_EERASE,
    REF_CR_FOTA_EVERIF
}RefCrFotaErrCode_e;

typedef enum
{
    XOTA_EC_PARAM_INVALID = 1,
    XOTA_EC_OPER_UNSUPP,
    XOTA_EC_PKGSZ_ERROR,
    XOTA_EC_PKGSN_ERROR,
    XOTA_EC_PKGOFFS_ERROR,
    XOTA_EC_CRC8_ERROR,
    XOTA_EC_FLERASE_UNDONE,
    XOTA_EC_FLERASE_ERROR,
    XOTA_EC_FLWRITE_ERROR,
    XOTA_EC_FLREAD_ERROR,
    XOTA_EC_UNDEF_ERROR,

    XOTA_EC_EREOR_MAX
}AtXotaErrCode_e;

#define XOTA_CONVTO_REF_CR_ERRCODE(referr, xerr)  \
        if((xerr) < XOTA_EC_EREOR_MAX)\
        {\
            switch(xerr) \
            {\
                case XOTA_EC_PARAM_INVALID:\
                    referr = REF_CR_FOTA_ECONFIG;\
                    break;\
                case XOTA_EC_OPER_UNSUPP:\
                    referr = XOTA_EC_OPER_UNSUPP;/* keep the same! */\
                    break;\
                case XOTA_EC_PKGSZ_ERROR:\
                case XOTA_EC_PKGSN_ERROR:\
                case XOTA_EC_PKGOFFS_ERROR:\
                    referr = REF_CR_FOTA_EFWDL;\
                    break;\
                case XOTA_EC_FLERASE_UNDONE:\
                case XOTA_EC_FLERASE_ERROR:\
                    referr = REF_CR_FOTA_EERASE;\
                    break;\
                case XOTA_EC_FLWRITE_ERROR:\
                    referr = REF_CR_FOTA_EFWWR;\
                    break;\
                default:\
                    referr = REF_CR_FOTA_EUNDEF;\
                    break;\
            }\
        }\
        else\
        {\
            referr = xerr;/* keep the same! */\
        }\

typedef struct
{
    char* url;
    char* user;
    char* passwd;
    uint32_t packetSize;
    uint8_t sslId;
    uint8_t timeout;
} AtXotaDownloadReq_t;


#endif


