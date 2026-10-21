#ifndef __OL_LBS_API__
#define __OL_LBS_API__

typedef unsigned long            uint32_t;

/*****************************************************************************
 * DESCRIPTION
 *    This API is to get Location Based Service(LBS) data
 *    Note: api function cannot be accessed repeatedly within 10 minutes 
 *          after the last successful attempt
 *
 * PARAMETERS
 *    hostname       : [IN]  the hostname of the server
 *    port           : [IN]  the server port
 *    info           : [OUT] ponit to output lbs string info "lbs: longitude: xxx, latitude: xxx"
 *    timeout_sec    : [IN]  Timeout waiting for server return lbs info
 *
 * RETURN
 *    =0 : success
 *    <0 : fails
 *****************************************************************************/
extern int ol_get_lbs_info(char * hostname, uint32_t port, char *info, uint32_t timeout_sec);


/*****************************************************************************
 * DESCRIPTION
 *    This API is to get Location Based Service(LBS) data through the amap key
 *    Note: api function cannot be accessed repeatedly within 10 minutes 
 *          after the last successful attempt
 *
 * PARAMETERS
 *    key               : [IN] The key applied from Amap
 *    pIndBuf           : [OUT] ponit to output lbs string info "lbs: longitude: xxx, latitude: xxx"
 *
 * RETURN
 *    =0 : success
 *    <0 : fails
 *****************************************************************************/
extern int ol_lbs_gdhttp_info(char *key, char *pIndBuf);

#endif
