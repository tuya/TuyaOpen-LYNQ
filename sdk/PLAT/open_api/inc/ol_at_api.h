#ifndef __OL_AT_IFACE_API_H__
#define __OL_AT_IFACE_API_H__

typedef enum
{
    OL_AT_OK = 0,
    OL_AT_CME_ERROR,
    OL_AT_INVALID_PARAM,
    OL_AT_TIME_OUT,
    OL_AT_SYS_ERROR,
}ol_at_err_num;

typedef void (*ol_at_urc_cb)(const CHAR *urc_str, UINT32 strlen);


/*****************************************************************************
 *
 * DESCRIPTION
 *  This API is to send one at command to modem and read the response
 *
 * PARAMETERS
 *      cmd         : [IN]  AT string want send to modem
 *      timeout     : [IN]  timeout to read the response, Unit is seconds
 *      resp        : [OUT] AT command response
 *      resp_len    : [OUT] AT command response length
 *
 * RETURN
 *      return value define in ol_at_err_num
 *
 *****************************************************************************/
extern int ol_at_send_wait_resp(char *cmd, int timeout, char *resp, unsigned long resp_len);


/*****************************************************************************
 *
 * DESCRIPTION
 *  This API is to register the callback of the at urc
 *
 * PARAMETERS
 *      cb_fun      :[IN]the callball function
 *
 * RETURN
 *      0 : success
 *      -1: error
 *
 *****************************************************************************/
extern int ol_at_urc_register(ol_at_urc_cb cb_func);


/*****************************************************************************
 *
 * DESCRIPTION
 *  This API is to register the callback of the at urc
 *
 * PARAMETERS
 *      cb_fun      :[IN]the callball function
 *
 * RETURN
 *      0 : success
 *      -1: error
 *
 *****************************************************************************/
extern int ol_at_urc_deregister(void);


#endif/*__OL_AT_IFACE_API_H__*/
