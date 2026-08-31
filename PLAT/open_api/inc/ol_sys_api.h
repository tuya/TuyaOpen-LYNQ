/****************************************************************************
 *
 ****************************************************************************/
#ifndef __OL_SYS_API__
#define __OL_SYS_API__


typedef enum
{
    OL_DUMP_FLASH_EPAT_LOOP,      /*0 -- dump full exception info to flash and EPAT tool then trapped in endless loop(while(1))*/
    OL_DUMP_PRINT_RESET,          /*print necessary exception info, and then reset*/
    OL_DUMP_FLASH_RESET,          /*dump full exception info to flash, and then reset*/
    OL_DUMP_FLASH_EPAT_RESET,     /*dump full exception info to flash and EPAT tool, and then reset*/
    OL_DUMP_SILENT_RESET,         /*reset directly*/
    OL_DUMP_MAX
}ol_dump_mode;

typedef enum
{
    OL_PS_ID_START = 0,

    OL_PS_ID_MM_SIGQ,                   /* param -> CmiMmCesqInd */
    OL_PS_ID_MM_TAU_EXPIRED,            /* param -> NULL*/
    OL_PS_ID_MM_BLOCK_STATE_CHANGED,    /* param -> UINT8(CmiMmEmmTimerStateInd->timerState) */
    OL_PS_ID_MM_CERES_CHANGED,          /* param -> UINT8(CmiMmCEStatusInd->ceLevel) */
    OL_PS_ID_MM_NITZ_REPORT = 5,        /* param -> CmiMmNITZInd*/

    OL_PS_ID_MM_PSM_STATE_CHANGED,      /* reserve, not used */

    OL_PS_ID_PS_BEARER_ACTED = 7,       /* param -> UINT8(CmiPsBearerActedInd->cid)*/
    OL_PS_ID_PS_BEARER_DEACTED,         /* param -> UINT8(CmiPsBearerActedInd->cid)*/
    OL_PS_ID_PS_CEREG_CHANGED,          /* param -> CmiPsCeregInd*/
    OL_PS_ID_PS_NETINFO,                /* param -> NmAtiNetInfoInd*/
    OL_PS_ID_PS_PKG_DATA_TRANS_STATE_CHANGED, /* param -> CmiPsPkgDataTransStateInd*/

    OL_PS_ID_SIM_READY,                 /* param -> CmiSimImsiStr*/
    OL_PS_ID_SIM_REMOVED,               /* param -> NULL*/

    OL_PS_ID_END,
}ol_ps_event_id;

typedef enum
{
    MBTK_SIM_READY_NETLIGHT,
    MBTK_SIM_BLOCK_NETLIGHT,
    MBTK_SIM_REMOVE_NETLIGHT,
    MBTK_SIM_MAX_NETLIGHT,
}mbtk_sim_status_netlight;


typedef void (*ol_ps_event_cb_f)(ol_ps_event_id eventId, void *param, UINT32 paramLen);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to get the software version
 * PARAMETERS
 *      NULL
 * RETURN
 *      Return a pointer to the software version string
 *
 *****************************************************************************/
extern char *ol_get_sw_version(void);


/*****************************************************************************
 *
 * DESCRIPTION
 *      Get chip version string
 * PARAMETERS
 *      NULL
 * RETURN
 *      Pointer to chip id string, e.g. "EC718PM" / "EC718PVM" / "EC718P"
 *      Return "ERROR" if unrecognized
 *
 *****************************************************************************/
extern char *ol_get_chipid(void);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to power reset module
 * PARAMETERS
 *      NONE
 * RETURN
 *      NONE
 *
 *****************************************************************************/
extern void ol_power_reset(void);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to power off module
 * PARAMETERS
 *      NONE
 * RETURN
 *      NONE
 *
 *****************************************************************************/
extern void ol_power_off(void);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to set dump mode
 * PARAMETERS
 *      flag : enum ol_fump_mode
 * RETURN
 *      0  : success
 *      -1 : fail
 * 
 *****************************************************************************/
extern int ol_set_dump_flag(ol_dump_mode flag);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to get dump mode
 * PARAMETERS
 *      NONE
 * RETURN
 *      dump mode value, enum ol_fump_mode
 *
 *****************************************************************************/
extern ol_dump_mode ol_get_dump_flag(void);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to register functions for specific PS event callback
 * PARAMETERS
 *      callback: Callback function to be registered
 * RETURN
 *      0  : success
 *      -1 : fail
 *
 *****************************************************************************/
int ol_ps_event_register(ol_ps_event_cb_f callback);


/*****************************************************************************
 *
 * DESCRIPTION
 *      This API is used to deregister specific PS event callback
 * PARAMETERS
 *      NONE
 * RETURN
 *      0  : success
 *      -1 : fail
 *
 *****************************************************************************/
int ol_ps_event_deregister();


#endif /*__OL_SYS_API__*/
