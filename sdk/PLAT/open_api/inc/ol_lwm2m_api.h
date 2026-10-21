#ifndef __OL_LWM2M_API__
#define __OL_LWM2M_API__

#ifdef MBTK_OPENCPU_LWM2M

#define CTM2MSETPM_1_PORT_MIN                 1
#define CTM2MSETPM_1_PORT_MAX                 65535
#define CTM2MSETPM_1_PORT_DEF                 5683
#define CTM2MSETPM_2_LIFETIME_MIN             300
#define CTM2MSETPM_2_LIFETIME_MAX             0x7FFFFFFF
#define CTM2MSETPM_2_LIFETIME_DEF             86400

typedef void (*ol_at_urc_cb)(const char *urc_str, unsigned long strlen);

typedef struct
{
    char serverIP[256+1];
    signed long lifeTime;
    signed long port;
} ol_lwm2m_config_type;

typedef enum
{
    OL_SENDMODE_CON,
    OL_SENDMODE_NON,
    OL_SENDMODE_NON_REL,
    OL_SENDMODE_CON_REL,
}ol_lwm2m_send_mode;

typedef enum
{
    OL_RECVMODE_NO_INDICATE,            //cache data but not indicate
    OL_RECVMODE_REPORT,                 //report immediately
    OL_RECVMODE__WITH_INDICATE,         //cache data and indicate
}ol_lwm2m_recv_mode;


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to register the callback of the lwm2m urc report
 *
 * PARAMETERS
 *    cb_func     : [IN] the callback function
 *
 * RETURN
 *    =0 : success
 *    <0 : fail
*****************************************************************************/
extern int ol_lwm2m_urc_register(ol_at_urc_cb cb_func);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to set the config about the lwm2m connection parameters
 *
 * PARAMETERS
 *    config     : [IN] see the struct ol_lwm2m_config_type
 *
 * RETURN
 *    =0 : success
 *    <0 : fail
*****************************************************************************/
extern int ol_lwm2m_config(ol_lwm2m_config_type config);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to send the registration request
 *
 * PARAMETERS
 *      NONE
 *
 * RETURN
 *    =0 : success
 *    <0 : fail
*****************************************************************************/
extern int ol_lwm2m_reg(void);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to send the data to the AEP plate
 *
 * PARAMETERS
 *    data     : [IN] the send data
 *    mode     : [IN] send mode,see ol_lwm2m_send_mode
 *
 * RETURN
 *    =0 : success
 *    <0 : fail
*****************************************************************************/
extern int ol_lwm2m_send(char * data,ol_lwm2m_send_mode mode);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to set the recv mode
 *
 * PARAMETERS
 *    rmode     : [IN] send mode,see ol_lwm2m_recv_mode
 *
 * RETURN
 *    =0 : success
 *    <0 : fail
*****************************************************************************/
extern int ol_lwm2m_set_recv_mode(ol_lwm2m_recv_mode rmode);


/*****************************************************************************
 * DESCRIPTION
 *    This API is used to set the recv mode
 *
 * PARAMETERS
 *    datalen     : [OUT]the read length
 *    data        : [OUT]the read data
 *
 * RETURN
 *    =0 : success
 *    <0 : fail
*****************************************************************************/
extern int ol_lwm2m_read(int *datalen,unsigned char **data);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to send the dereg request
 *
 * PARAMETERS
 *      NULL
 *
 * RETURN
 *    =0 : success
 *    <0 : fail
*****************************************************************************/
extern int ol_lwm2m_dereg(void);

/*****************************************************************************
 * DESCRIPTION
 *    This API is used to send update request
 *
 * PARAMETERS
 *      NULL
 *
 * RETURN
 *    =0 : success
 *    <0 : fail
*****************************************************************************/
extern int ol_lwm2m_update(void);


#endif //MBTK_OPENCPU_LWM2M
#endif //__OL_LWM2M_API__
