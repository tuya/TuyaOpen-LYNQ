/****************************************************************************
 *
 * File name:    app.c
 * Description:  open app demo entry source file
 *
 ****************************************************************************/
#include "string.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "os_common.h"
#include "ostask.h"
#include "cmsis_os2.h"
#include "plat_config.h"
#include "ps_lib_api.h"
#include "ps_event_callback.h"
#include "slpman.h"
#include "reset.h"
#include "app.h"
#include "ol_sys_api.h"
#include "ol_log.h"


void app_netPSEventCallback(ol_ps_event_id eventID, void* param, UINT32 paramLen)
{
    CmiSimImsiStr* imsi = NULL;
    CmiPsCeregInd* creg = NULL;
    NmAtiNetInfoInd* netif = NULL;
    UINT8 cid = 0;

    OL_LOG_PRINTF("app netPS cb eventID: %d", eventID);

    if(OL_PS_ID_PS_CEREG_CHANGED == eventID) //UE PLMN register state changed
    {
        creg = (CmiPsCeregInd*)param;
        OL_LOG_PRINTF("app cereg act:%d, celId:%d, state:%d", creg->act, creg->cellId, creg->state);
    }
    else if(OL_PS_ID_PS_NETINFO == eventID) //WAN net base info, such as ipv4/ipv6 info
    {
        netif = (NmAtiNetInfoInd*)param;
        OL_LOG_PRINTF("app netStatus:%d, ipType:%d",netif->netifInfo.netStatus, netif->netifInfo.ipType);
    }
    else if(OL_PS_ID_SIM_READY == eventID) //SIM card is ready
    {
        imsi = (CmiSimImsiStr*)param;
        OL_LOG_PRINTF("app SIM Ready IMSI: %s", imsi->contents);
    }
    else if(OL_PS_ID_PS_BEARER_ACTED == eventID)
    {
        memcpy(&cid, param, paramLen);
        OL_LOG_PRINTF("app Act cid: %d", cid);
    }
    else if(OL_PS_ID_PS_BEARER_DEACTED == eventID)
    {
        memcpy(&cid, param, paramLen);
        OL_LOG_PRINTF("app Deact cid: %d", cid);
    }
}


/*
  App init entry function. Customer do init or create tasks based on their needs
*/
static void app_init(void *arg)
{
    LastResetState_e ap_state;
    LastResetState_e cp_state;
    slpManSlpState_t slpstate;

    osDelay(500);

    //register network change event callback
    ol_ps_event_register(app_netPSEventCallback);

    OL_LOG_PRINTF("App demo init: %s", ol_get_sw_version());

    ResetStateGet(&ap_state, &cp_state);
    OL_LOG_PRINTF("Check last reset reason: ap_state = %d, cp_state = %d",ap_state, cp_state);

    //check wakeup from deep sleep or power on
    slpstate = slpManGetLastSlpState();
    OL_LOG_PRINTF("Check last sleep state = %d",slpstate);

    if(1)
    {
        usb_test_demo_menu(); //create usb test task
    }
    else
    {
        uart_test_demo_menu(); //create uart test task
    }
}

/**
  \fn          int main_entry(void)
  \brief       main entry function.
  \return
*/
void main_entry(void)
{
    BSP_CommonInit();

    slpManAONIOPowerOn();
    mbtkInitENGpio();

    if(0)
    {
        //IO voltage default is 1.8V
        //Modify the IO voltage to 3.3V or other voltage define in IOVoltageSel_t as follows
        slpManAONIOVoltSet(IOVOLT_3_30V);
        slpManNormalIOVoltSet(IOVOLT_3_30V);
    }

    osKernelInitialize();
    registerAppEntry(app_init, NULL);
    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }
    while(1);

}
