/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History: initiated by xxxx
*
*  Notes:
*
******************************************************************************/

/*----------------------------------------------------------------------------*
 *                    NCLUDES                                                *
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "clock.h"
#include "bsp_custom.h"
#include "slpman.h"
#include "plat_config.h"
#include DEBUG_LOG_HEADER_FILE
#include "exception_process.h"
#include "gpio.h"
#include "timer.h"
#include "reset.h"
#include "apmu_external.h"
#ifdef FEATURE_CCIO_ENABLE
#include "ccio_custom.h"
#include "uart_device.h"
#endif
#include "hal_charge.h"
#include "apmu_external.h"
#if (WDT_FEATURE_ENABLE==1)
#include "wdt.h"
#define WDT_TIMEOUT_VALUE     (20)            // in unit of second, shall be less than 256s
#endif
#include "cmsis_os2.h"
#include "hal_pwrkey.h"
#include "hal_misc.h"
#include "hal_trim.h"
#include "unilog.h"
#include "usb_ext_inc.h"
#include "hal_alarm.h"
#include "usbd_errinfo.h"
#ifdef FEATURE_PLAT_CTLWM2M_AT_ENABLE
#include "at_ctlwm2m_autoupdate.h"
#endif
#ifdef FEATURE_PLAT_ONENET_AT_ENABLE
#include "at_onenet_task.h"
#endif
#include "usb_net_adapt_cust.h"
#include "sys_record.h"

#include "dbversion.h"

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

 #ifndef BSP_USB_BOOT_INIT_EN
//#define BSP_USB_BOOT_INIT_EN 0
#define BSP_USB_BOOT_INIT_EN 1
#endif

#define RESET_REASON_WDT_FLAG  0xECAAEE

/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/

typedef void (*BSP_SetConfigFunc)(void *devConf);
typedef void (*BSP_InitHandlerFunc)(uint8_t devIdx, uint32_t baudRate, uint32_t frameFmt);


/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/



__USED static uint8_t rsvUsrRam[1*1024];

/*
#if defined CHIP_EC716
#ifdef FEATURE_MORERAM_ENABLE
#ifdef FEATURE_IMS_ENABLE
__USED static uint8_t rsvUsrRam[120*1024];
#else
__USED static uint8_t rsvUsrRam[300*1024];
#endif
#else
__USED static uint8_t rsvUsrRam[200*1024];
#endif
#else//718
#ifdef OPEN_CPU_MODE
#ifdef FEATURE_IMS_ENABLE
#ifdef FEATURE_IMS_USE_PSRAM_ENABLE

#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
__USED static uint8_t rsvUsrRam[70*1024];     // 70KB ram reserved for customer
#else
__USED static uint8_t rsvUsrRam[78*1024];     // 78KB ram reserved for customer
#endif

#else

#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
__USED static uint8_t rsvUsrRam[40*1024];     // 40KB ram reserved for customer
#else
__USED static uint8_t rsvUsrRam[48*1024];     // 48KB ram reserved for customer
#endif

#endif
#else// ims disen
#if defined (FEATURE_AMR_CP_ENABLE) && defined (FEATURE_VEM_CP_ENABLE)

#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
__USED static uint8_t rsvUsrRam[337*1024];     // 337KB ram reserved for customer
#else
__USED static uint8_t rsvUsrRam[345*1024];    // 345KB ram reserved for customer
#endif

#else

#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
__USED static uint8_t rsvUsrRam[415*1024];     // 415KB ram reserved for customer
#else
__USED static uint8_t rsvUsrRam[420*1024];    // 420KB ram reserved for customer
#endif
#endif
#endif//ims enable
#else//not open cpu
#if(RTE_CCID_EN == 1)

#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
__USED static uint8_t rsvUsrRam[152*1024];     // ccid feature need more SRAM
#else
__USED static uint8_t rsvUsrRam[160*1024];    // ccid feature need more SRAM
#endif

#else
#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
__USED static uint8_t rsvUsrRam[162*1024];    // 162KB ram reserved for customer
#else
__USED static uint8_t rsvUsrRam[170*1024];    // 174KB ram reserved for customer
#endif

#endif
#endif

#endif
*/

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/

typedef int (*pfn_PadWakeupHook)(uint32_t pad_num);
extern BOOL excepIsInExcephandler(void);
extern void apmuSetCPFastBoot(bool force_on);

extern void usbstack_top_var_clear(void);
extern uint8_t usbstack_clear_ctx_stat(void);
extern uint8_t usbstack_ctx_stat_ison(void);
extern void usbstack_set_ctx_vbus_mode(uint8_t vbus_mode_en, uint8_t vbus_pad_idx);
extern void usbc_call_usbmst_slp1_retothwk_earlyinit(void);
extern UINT32 flashXIPLimit;
extern void ShareInfoAPSetLdoFemVbatType(uint8_t type);

extern int32_t uartDevNotifySerlDtrEvt(uint32_t uartIdx);


/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTIONS                                     *
 *----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/
#ifdef FEATURE_CCIO_ENABLE
/**
  \brief Init uart handler for at/raw data
  \return
 */
static void BSP_InitUartHandler(uint8_t uartIndex, uint32_t baudRate, uint32_t frameFormat, BSP_SetConfigFunc setConfigFn)
{
    atPortFrameFormat_t    uartFrameFmt;
    UartDevConf_t           uartDevConf;
    UartHwConf_t            *uartHwConf = &uartDevConf.hwConf;

    memset(&uartDevConf, 0, sizeof(UartDevConf_t));

    uartHwConf->powerMode   = ARM_POWER_FULL;
    uartHwConf->ctrlSetting = ARM_USART_MODE_ASYNCHRONOUS;
    uartHwConf->baudRate    = baudRate;

    //setUartBaudRate(uartIndex, baudRate);

    uartFrameFmt.wholeValue = frameFormat;
    switch(uartFrameFmt.config.dataBits)
    {
        case 1:
            uartHwConf->ctrlSetting |= ARM_USART_DATA_BITS_7;
            break;
        default:
            uartHwConf->ctrlSetting |= ARM_USART_DATA_BITS_8;
            break;
    }

    switch(uartFrameFmt.config.parity)
    {
        case 1:
            uartHwConf->ctrlSetting |= ARM_USART_PARITY_ODD;
            break;
        case 2:
            uartHwConf->ctrlSetting |= ARM_USART_PARITY_EVEN;
            break;
        default:
            uartHwConf->ctrlSetting |= ARM_USART_PARITY_NONE;
            break;
    }

    switch(uartFrameFmt.config.stopBits)
    {
        case 2:
            uartHwConf->ctrlSetting |= ARM_USART_STOP_BITS_2;
            break;
        default:
            uartHwConf->ctrlSetting |= ARM_USART_STOP_BITS_1;
            break;
    }

    switch(uartFrameFmt.config.flowControl)
    {
        case 3:
            uartHwConf->ctrlSetting |= ARM_USART_FLOW_CONTROL_RTS_CTS;
            break;
        case 2:
            uartHwConf->ctrlSetting |= ARM_USART_FLOW_CONTROL_CTS;
            break;
        case 1:
            uartHwConf->ctrlSetting |= ARM_USART_FLOW_CONTROL_RTS;
            break;
        case 0:
        default:
            uartHwConf->ctrlSetting |= ARM_USART_FLOW_CONTROL_NONE;
            break;
    }

    switch(baudRate)
    {
        case 9600:
        case 4800:
        case 2400:
        case 1200:
        case 600:
            uartHwConf->powerMode = ARM_POWER_LOW;
            break;
        default:
            break;
    }

    if(uartIndex == 0)
    {
    #if (RTE_UART0)
        extern ARM_DRIVER_USART Driver_USART0;
        uartDevConf.drvHandler = &Driver_USART0;
    #endif
    }
    else if(uartIndex == 1)
    {
    #if (RTE_UART1)
        extern ARM_DRIVER_USART Driver_LPUSART1;
        uartDevConf.drvHandler  = &Driver_LPUSART1;
        uartDevConf.isDftAtPort = 1;
    #endif
    }
    else if (uartIndex == 2)
    {
    #if (RTE_UART2)
        extern ARM_DRIVER_USART Driver_USART2;
        uartDevConf.drvHandler = &Driver_USART2;
    #endif
    }
    else if (uartIndex == 3)
    {
#if (RTE_UART3)
        extern ARM_DRIVER_USART Driver_USART3;
        uartDevConf.drvHandler = &Driver_USART3;
#endif
    }

    EC_ASSERT(uartDevConf.drvHandler, uartIndex, setConfigFn, 0);

    if(setConfigFn)
    {
        setConfigFn(&uartDevConf);
    }
    else
    {
        /* TBD: ~~ */
    }

    // open printf throught AT port
    // UsartPrintHandle = uartDevConf.drvHandler;
    uartDevCreate(uartIndex, &uartDevConf);
}

void BSP_SetUart(usart_port_t port, BSP_InitHandlerFunc initHandlerFn)
{
    uint32_t baudRate = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_AT_PORT_BAUDRATE);
    uint32_t frameFormat = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_AT_PORT_FRAME_FORMAT);

    // check if auto baud rate flag is set
    if(baudRate & 0x80000000UL)
    {
        // re-enable auto baud detection on POR, otherwise keep setting if wakeup from other low power modes
        if(slpManGetLastSlpState() == SLP_ACTIVE_STATE)
        {
            // Shall overwrite previous detection value for corner case:
            // previous detection e.g 9600 is recorded, when power on again, the baud rate is going to change to 115200, however device enter
            // lower poer mode before auto detection is done, after wakeup, it'll mess up, so we need to keep this state sync by saving setting
            if(baudRate & 0x7FFFFFFFUL)
            {
                BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_AT_PORT_BAUDRATE, 0x80000000UL);
                BSP_SavePlatConfigToFs();
                /* ensure the baud to be identical! */
                BSP_SavePlatConfigToRawFlash();
            }

            baudRate = 0; // auto baud rate

        }
        else
        {
            baudRate &= 0x7FFFFFFFUL;
        }
    }
    else
    {
        baudRate &= 0x7FFFFFFFUL;
    }

    if(initHandlerFn)
    {
        initHandlerFn(port, baudRate, frameFormat);
    }
    else
    {
        /* TBD: ~~ */
    }
}

/*
 *  unset uart port
 *  Parameter: port: port for atcmd/opaq/unilog
 */
void BSP_UnsetUart(usart_port_t port)
{
    uint32_t uartIdx = 0xff;

    if(port == PORT_USART_0)
    {
    #if (RTE_UART0)
        uartIdx = 0;
    #endif
    }
    else if(port == PORT_USART_1)
    {
    #if (RTE_UART1)
        uartIdx = 1;
    #endif
    }
    else if (port == PORT_USART_2)
    {
    #if (RTE_UART2)
        uartIdx = 2;
    #endif
    }
    else if (port == PORT_USART_3)
    {
#if (RTE_UART3)
        uartIdx = 3;
#endif
    }

    uartDevDestroy(uartIdx);
}

void BSP_SetOpaqUartConf(UartDevConf_t *uartConf)
{
    uartConf->mainUsage    = CSIO_DT_OPAQ;
    uartConf->speedType    = CCIO_ST_HIGH;
    uartConf->bmCreateFlag = CCIO_TASK_FLAG_RX | CCIO_TASK_FLAG_TX;
    uartConf->rxMethod     = UART_DEV_RX_SLIDER;
    uartConf->custFlags    = CSIO_OCF_UARTEST,
    uartConf->rbufFlags    = uartConf->isDftAtPort ? CUST_RBUF_FOR_AT_CALI : CUST_RBUF_FOR_OPAQ /* or CUST_RBUF_FOR_AT_NORM if reused! */;
}

void InitOpaqUartHandler(uint8_t uartIndex, uint32_t baudRate, uint32_t frameFormat)
{
    BSP_InitUartHandler(uartIndex, baudRate, frameFormat, (BSP_SetConfigFunc)BSP_SetOpaqUartConf);
}

void BSP_SetAtUartConf(UartDevConf_t *uartConf)
{
    uartConf->mainUsage    = CSIO_DT_AT;
    uartConf->speedType    = CCIO_ST_SLOW;
    uartConf->bmCreateFlag = CCIO_TASK_FLAG_RX | CCIO_TASK_FLAG_TX;
    uartConf->rxMethod     = UART_DEV_RX_SLIDER;
    uartConf->rbufFlags    = uartConf->isDftAtPort ? CUST_RBUF_FOR_AT_CALI : CUST_RBUF_FOR_AT_NORM;
}

/**
  \brief Init uart handler for AT CMD
  \return
 */
void InitAtUartHandler(uint8_t uartIndex, uint32_t baudRate, uint32_t frameFormat)
{
    BSP_InitUartHandler(uartIndex, baudRate, frameFormat, (BSP_SetConfigFunc)BSP_SetAtUartConf);
}

void BSP_SetMuxUartConf(UartDevConf_t *uartConf)
{
    uartConf->mainUsage    = CSIO_DT_MUX;
    uartConf->speedType    = CCIO_ST_SLOW;
    uartConf->bmCreateFlag = CCIO_TASK_FLAG_RX | CCIO_TASK_FLAG_TX;
    uartConf->rbufFlags    = uartConf->isDftAtPort ? CUST_RBUF_FOR_AT_CALI : CUST_RBUF_FOR_AT_NORM; /* no particular rbuf for mux */
}

/**
  \brief Init uart handler for cmux
  \return
 */
void InitMuxUartHandler(uint8_t uartIndex, uint32_t baudRate, uint32_t frameFormat)
{
    BSP_InitUartHandler(uartIndex, baudRate, frameFormat, (BSP_SetConfigFunc)BSP_SetMuxUartConf);
}

#endif

void SetOpaqUart(usart_port_t port)
{
#ifdef FEATURE_CCIO_ENABLE
    BSP_SetUart(port, InitOpaqUartHandler);
#endif
}

/*
 *  set printf and at command uart port
 *  Parameter:      port: port for at cmd
 */
void SetAtUart(usart_port_t port)
{
#ifdef FEATURE_CCIO_ENABLE
    BSP_SetUart(port, InitAtUartHandler);
#endif
}

void SetMuxUart(usart_port_t port)
{
#ifdef FEATURE_CCIO_ENABLE
    BSP_SetUart(port, InitMuxUartHandler);
#endif
}

#define UnsetAtOrLogUart(prt) UnsetUart(prt)

void UnsetUart(usart_port_t port)
{
#ifdef FEATURE_CCIO_ENABLE
    BSP_UnsetUart(port);
#endif
}

void GPR_SetUartClk(void)
{
    GPR_clockDisable(FCLK_UART0);
    GPR_clockDisable(FCLK_UART1);

    GPR_setClockSrc(FCLK_UART0, FCLK_UART0_SEL_26M);
    GPR_setClockSrc(FCLK_UART1, FCLK_UART1_SEL_26M);

    GPR_clockEnable(FCLK_UART0);
    GPR_clockEnable(FCLK_UART1);

    GPR_swReset(RST_FCLK_UART0);
    GPR_swReset(RST_FCLK_UART1);
}



#if (WDT_FEATURE_ENABLE == 1)

/*
 *  WDT Initialize, wdt timeout value is 20s
 *  Parameter:   none
 */
void BSP_WdtInit(void)
{
    // Config WDT clock, source from 40KHz and divide by WDT_TIMEOUT_VALUE
    GPR_setClockSrc(FCLK_WDG, FCLK_WDG_SEL_40K);
    GPR_setClockDiv(FCLK_WDG, WDT_TIMEOUT_VALUE);

    WdtConfig_t wdtConfig;
    wdtConfig.mode = WDT_INTERRUPT_RESET_MODE;
    wdtConfig.timeoutValue = 40000U;
    WDT_init(&wdtConfig);
}

#endif

void NMI_Handler()
{
    slpManAonWdtStop();

    ECPLAT_PRINTF(UNILOG_PLA_APP, enter_NMI_handler, P_ERROR, "WDT timeout!!! Enter NMI Handler!!!");

    // If we have been in exception handler excecution, we shall resume it.
    if(excepIsInExcephandler())
    {
        WDT_stop();
    }
    else
    {
        ResetReasonWriteAP(RESET_REASON_WDT);
        if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_FAULT_ACTION) == EXCEP_OPTION_SILENT_RESET)
        {
            #if defined TYPE_EC718S
            EC_ASSERT(0, 0, RESET_REASON_WDT_FLAG, 0);
            #else
            EC_ASSERT(0, 0, 0, RESET_REASON_WDT_FLAG);
            #endif

            ResetECSystemReset();
            while(1);
        }
        else
        {
            EC_ASSERT(0, 0, 0, 0);
        }
    }
}



void NetLightInit(void)
{
    slpManAONIOPowerOn();			// power on AONIO if we use aonio

    PadConfig_t padConfig;
    PAD_getDefaultConfig(&padConfig);

    padConfig.mux = NETLIGHT_PAD_ALT_FUNC;
    PAD_setPinConfig(NETLIGHT_PAD_INDEX, &padConfig);

    // Config PWM clock, source from 26MHz and divide by 1
    CLOCK_setClockSrc(FCLK_TIMER3, FCLK_TIMER3_SEL_26M);
    CLOCK_setClockDiv(FCLK_TIMER3, 1);

    TIMER_driverInit();

    TIMER_netlightEnable(NETLIGHT_PWM_INSTANCE);

}



/**
  \fn        WakeupPadInit(void)
  \brief     init wakeup pad
  \param[in]
  \note     pad0: key on the board  pullup
            pad1: connect USB VBUS on board, should set as floating, otherwise will leak when usb not insert
            and when usb vbus mode en , usb will init by itself
            pad2/3/4---not used, if sim hot swap en , sim will enable by itself
            pad5：also connect to USB VBUS on board should set as floating, otherwise will leak when usb not insert
            usb not use this pad. if used by others should disconnect from vbus on board
            customer could cfg as needed according to their own board
 */
void WakeupPadInit(void)
{
#if defined CHIP_EC716
    APmuWakeupPadSettings_t wakeupPadSetting;

// #if (UART_DEV_DTR_WKUP_ENABLE != 1)
//     wakeupPadSetting.negEdgeEn = true;
//     wakeupPadSetting.posEdgeEn = false;
//     wakeupPadSetting.pullDownEn = false;
//     wakeupPadSetting.pullUpEn = true;

//     slpManSetWakeupPadCfg(WAKEUP_PAD_0, true, &wakeupPadSetting);
//     NVIC_EnableIRQ(PadWakeup0_IRQn);
// #endif
    wakeupPadSetting.negEdgeEn = true;
    wakeupPadSetting.posEdgeEn = false;
    wakeupPadSetting.pullDownEn = false;
    wakeupPadSetting.pullUpEn = true;

// tuya 
    slpManSetWakeupPadCfg(WAKEUP_PAD_0, true, &wakeupPadSetting);      //默认关闭pin19中断，低功耗下开启
    NVIC_DisableIRQ(PadWakeup0_IRQn);
    slpManSetWakeupPadCfg(WAKEUP_PAD_1, false, &wakeupPadSetting);      //sim det引脚
    NVIC_DisableIRQ(PadWakeup1_IRQn);
    slpManSetWakeupPadCfg(WAKEUP_PAD_2, false, &wakeupPadSetting);
    NVIC_DisableIRQ(PadWakeup2_IRQn);
    slpManSetWakeupPadCfg(WAKEUP_PAD_4, false, &wakeupPadSetting);
    NVIC_DisableIRQ(PadWakeup4_IRQn);
    slpManSetWakeupPadCfg(WAKEUP_PAD_5, false, &wakeupPadSetting);
    NVIC_DisableIRQ(PadWakeup5_IRQn);
#else
    APmuWakeupPadSettings_t wakeupPadSetting;
    wakeupPadSetting.negEdgeEn = true;
    wakeupPadSetting.posEdgeEn = false;
    wakeupPadSetting.pullDownEn = false;
    wakeupPadSetting.pullUpEn = true;

    // CHIP_EC718/CHIP_QCX217
    slpManSetWakeupPadCfg(WAKEUP_PAD_0, true, &wakeupPadSetting);
    NVIC_EnableIRQ(PadWakeup0_IRQn);
#endif
}
void chargeStatusCallbackFunc(chargeStatus_e status)
{
    ECPLAT_PRINTF_OPT(UNILOG_PMU, chargeStatusCb_1, P_VALUE, "Charge Status update = %d", status);
}

void powerKeyStatusUpdate(pwrKeyPressStatus status)
{
    ECPLAT_PRINTF(UNILOG_PMU, powerKeyStatusUpdate_1, P_VALUE, "PowerKey Status update to = %d", status);
    if(status == PWRKEY_LONGPRESS)
    {
        ECPLAT_PRINTF(UNILOG_PMU, powerKeyStatusUpdate_2, P_WARNING, "PowerKey Start Power Off");
        uniLogFlushOut();
        pwrKeyStartPowerOff();
    }
}

void powerKeyFuncInit(void)
{
    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_PWRKEY_MODE) == 2)       // power key always connect to GND
    {
        pwrKeyDeinit(false);
    }
    else
    {
        pwrKeyDly_t pwrKeyDlyCfg;
        pwrKeyDlyCfg.pwrOffTimeout = 2500;
        pwrKeyDlyCfg.longPressTimeout = 2000;
        pwrKeyDlyCfg.repeatTimeout = 500;
        pwrKeyInit(PWRKEY_WAKEUP_LOWACTIVE_MODE, true, pwrKeyDlyCfg, powerKeyStatusUpdate);
    }
}


uint8_t usbstack_ctx_stat_issucc(void);

void BSP_UsbDeInit(void)
{
#if (RTE_USB_EN == 1)
    uint32_t ret ;
    usbd_set_mod_last_err(0, 0);

    ret = usbstack_deinit();
    EC_ASSERT((ret==0), usbd_get_mod_last_err(), 0, 0);
#endif
}

uint32_t BSP_UsbGetVBUSMode(void)
{
    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_USB_VBUS_MODE_EN) == 1)
    {
        return 1;
    }
    return 0;
}

uint32_t BSP_UsbGetVBUSWkupPad(void)
{
    return BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_USB_VBUS_WKUP_PAD);
}


uint8_t usblpw_get_retothwk_proc_stat(void);

uint8_t usblpw_rd_reg_retwkup_ctxstat(void);

void BSP_UsbInit(void)
{
#if (RTE_USB_EN == 1)
    uint32_t xTaskGetTickCount( void );
#if ((USBC_CTRL_HIB_LITE_MODE==0) )
    uint32_t save_irqflag;
#endif

    uint8_t usbinit_mode = 0;
    uint8_t usb_pwr_state = 0;
    uint8_t usb_retwkup_bootstat = 0;
    uint8_t usb_lastwkup_type = 0;
    uint32_t startTick, endTick;
    uint8_t ret;

    startTick = osKernelGetTickCount();
    ret = 1;
    usbc_trace_cfg(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_USB_SW_TRACE_FLAG));
    usblpw_clear_retothwk_proc_stat();
#if (USBC_CTRL_HIB_LITE_MODE==0)
    usb_wkmon_cmmon_clear();
#endif
    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_USB_CTRL) != 2)
    {
        ECOMM_TRACE(UNILOG_PMU, usblpw_wkup_or_init_pre_0, P_WARNING, 1, "ctx stat reg %d", usblpw_rd_reg_retwkup_ctxstat());

        if (usblpw_is_retwkup_support())
        {
            usblpw_get_pwr_info(&usb_pwr_state, &usb_retwkup_bootstat, &usb_lastwkup_type);
            ECPLAT_PRINTF(UNILOG_PMU, usblpw_wkup_or_init_0, P_VALUE, "pwr state 0x%x, bootstat 0x%x, lastwkup 0x%x, othwk stg 0x%x ", \
                    (uint32_t)usb_pwr_state, (uint32_t)usb_retwkup_bootstat, (uint32_t)usb_lastwkup_type, usblpw_retothwk_get_cur_stg() );

            if(usblpw_is_retwkups2_start())
            {
                //while(*(volatile uint32_t*)0x4d0201a0);
                slpManDrvVoteSleep(SLP_VOTE_LPUSB, SLP_IDLE_STATE);
                slpManSetUsbHighFreqFlag(true);

                ret = usblpw_retwkup_stack_restore();
                usbinit_mode  = 0x11;
                if (ret==0)
                {
                    usblpw_retwkup_stack_enable();
                }

                EC_ASSERT((ret==0), 0, 0, 0);
            }
            else if (usblpw_is_retothwks2_start())
            {
#if (USBC_CTRL_HIB_LITE_MODE==1)
                usblpw_clear_lpusbwkup_src();
                usbc_call_usbmst_slp1_retothwk_earlyinit();

                ret = usblpw_retothwk_hibslp2_stack_restore(usblpw_retothwk_stg_bt_later_idle);

                //anyway let allow sleep
                slpManDrvVoteSleep(SLP_VOTE_LPUSB, SLP_SLP1_STATE);
                slpManSetUsbHighFreqFlag(false);
                usbinit_mode  = 0x14;

                EC_ASSERT((ret==0), usbd_get_mod_last_err(), usblpw_retothwk_get_cur_stg(), usblpw_get_retothwk_proc_stat());

#endif

#if (USBC_CTRL_HIB_LITE_MODE==0)

                ECPLAT_PRINTF(UNILOG_PMU, usblpw_is_retothwks2_start_1, P_VALUE, "cur_stg 0x%x, ", usblpw_retothwk_get_cur_stg());
                //only bt pre idle or bt actv are valid here
                ret = 0;
                save_irqflag = SaveAndSetIRQMask();
                //for now, the LpusbWakeup_IRQn may enabled, so check triggered irq flag
                //usblpw_wr_reg_usb_wkdetflag(1); not called , the LpusbWakeup_IRQn is not enabled  when othwk recovery

                if (NVIC_GetEnableIRQ(LpusbWakeup_IRQn))
                {
                    ECOMM_TRACE(UNILOG_PLA_DRIVER,usblpw_is_retothwks2_start_2, P_WARNING, 0,"Disable LpusbWakeup_IRQn");
                    NVIC_DisableIRQ(LpusbWakeup_IRQn);
                }

                RestoreIRQMask(save_irqflag);

                if (usblpw_retothwk_cur_stg_bt_pre_idle())
                {
                    //no detect when boot pre idle, try bt later idle proc
                    usblpw_retothwk_set_bt_later_idle_stage();
                }

                switch (usblpw_retothwk_get_cur_stg())
                {
                    case usblpw_retothwk_stg_bt_later_idle:
                        ret = usblpw_retothwk_hibslp2_bt_later_idle_stage_proc();
                        if (ret!=0)
                        {
                            //any why assert
                            break;
                        }

                        if (usblpw_retothwk_cur_stg_hibslp2_wkmon())
                        {
                            // do not clear wkup src to avoid irq drop until wkup succ or terminated
                            //lpusbwkup_irq_flag = usblpw_clear_lpusbwkup_src();

                            //only for hibernate/slp2 mode
                            usb_wkmon_hibslp2_stat_evt_start();
                            usbinit_mode  = 0x14;
                            break;
                        }

                        if (usblpw_retothwk_cur_stg_bt_actv())
                        {
                            ret = usblpw_retothwk_hibslp2_bt_actv_stage_proc();
                            if (ret!=0)
                            {
                                //any why assert
                                break;
                            }
                        }

                        if (usblpw_retothwk_cur_stg_success()==0)
                        {
                            ret = 1;
                            break;
                        }

                        usbinit_mode  = 0x12;
                        break;

                    case usblpw_retothwk_stg_bt_actv:
                        ret = usblpw_retothwk_hibslp2_bt_actv_stage_proc();
                        if (ret!=0)
                        {
                            //any why assert
                            break;
                        }
                        if (usblpw_retothwk_cur_stg_success()==0)
                        {
                            ret = 1;
                            break;
                        }
                        usbinit_mode  = 0x13;
                        break;

                    default:
                        ret = 1;
                        break;
                }
                EC_ASSERT((ret==0), usbd_get_mod_last_err(), usblpw_retothwk_get_cur_stg(), usblpw_get_retothwk_proc_stat());
#endif

            }

        }
        if(usbinit_mode ==0)
        {
            ret = usbstack_init();
            usbinit_mode  = 0x10;
            EC_ASSERT((ret==0), usbd_get_mod_last_err(), 0, 0);
        }
    }

    if (usblpw_is_retwkup_support())
    {
        void usb_lpwkup_step_show();
        uint8_t usbstack_get_ctx_vbus_mode(void);

        usb_lpwkup_step_show();
        ECPLAT_PRINTF(UNILOG_PMU, usblpw_wkup_or_init_1, P_VALUE, "pwr state 0x%x, bootstat 0x%x, lastwkup 0x%x, inimod 0x%x ", \
                (uint32_t)usb_pwr_state, (uint32_t)usb_retwkup_bootstat, (uint32_t)usb_lastwkup_type, (uint32_t)usbinit_mode );
        if (usbstack_get_ctx_vbus_mode()==0)
        {
            ECPLAT_PRINTF(UNILOG_PMU, usblpw_wkup_or_init_2, P_VALUE, "othwk stg 0x%x, proc_stat 0x%x", usblpw_retothwk_get_cur_stg(), usblpw_get_retothwk_proc_stat() );
        }

    }
    endTick = osKernelGetTickCount();

    //check cost time only when wkup boot
    if(usbinit_mode==1)
    {
        EC_ASSERT((endTick-startTick) < 30, startTick, endTick, 0);
    }
#endif

}
slpManSlpState_t CheckUsrdefSlpStatus(void)
{
    slpManSlpState_t state = SLP_HIB_STATE;

    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_ECSCLK_CFG) == 1)
    {
        if((slpManGetWakeupPinValue() & (0x1<<0)) == 0)     // pad0 pin level is low
        {
            slpManVoteSetDozeEnable(false);
            state = SLP_IDLE_STATE;
        }
        else
        {
            slpManVoteSetDozeEnable(true);
            state = SLP_HIB_STATE;
        }
    }
	/*
	if do not sleep when ecsclk = 0
    else
    {
        state = SLP_IDLE_STATE;
    }
	*/
    return state;
}

#ifndef TYPE_EC718M
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
void psRamCodeAndDataInit(void)
{
    extern void CopyPsramCodeAndData(void);
    CopyPsramCodeAndData();
}

void pmuTimingChangeForPsram(void)
{
    pmuTimeCfg_t        cfg;

    slpManGetCurrentPmuTimingCfg(&cfg);

#if defined (FEATURE_IMS_ENABLE ) && defined (FEATURE_IMS_USE_PSRAM_ENABLE)

    cfg.preslp1_coderun_time += 0;      // nothing to do before sleep
    cfg.postslp1_coderun_time += 1;
    cfg.worth_slp1_time += 1;

#else
    cfg.preslp1_coderun_time += 0;
    cfg.postslp1_coderun_time += 0;
    cfg.worth_slp1_time += 0;
#endif
    slpManSavePmuTimingCfg(&cfg);
    return;
}

void psramCodeDataInitAndpmuTimingCfg(void)
{
    uint32_t uiPreTime = 0;
    uint32_t uiAfterTime = 0;

    slpManRegisterPSRamInitCb(psRamCodeAndDataInit);            // register callback

    uiPreTime = apmuGetBTMsCnt();
    psRamCodeAndDataInit();                                     // start initialize
    uiAfterTime = apmuGetBTMsCnt();

    /* More data mapped into PSRAM will cause ASSERT*/
    if (uiAfterTime - uiPreTime > 1)
    {
        EC_ASSERT(0, 0, 0, 0);
    }

    slpManRegisterPmuTimingCb(pmuTimingChangeForPsram);
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, psramCodeDataInitAndpmuTimingCfg_1, P_INFO, "psramCodeDataInitAndpmuTimingCfg:uiPreTime=%d,uiAfterTime=%d",uiPreTime,uiAfterTime);

    return;
}
#endif
#endif

/*
 *  custom board related init
 *  Parameter:   none
 *  note: this function shall be called in OS task context for dependency of reading configure file
 *        which is implemented based on file system
 */
void BSP_CustomInit(void)
{
    uint32_t FlashXIPSize;
    slpManWakeSrc_e wakeupSrc;
    uint32_t logPortSel, unilogUartPort;

    AdcEfuseCalCode_t* efuseCalCodePtr = NULL;
    AdcEfuseT0Code_t* efuseT0CodePtr = NULL;

    extern void mpu_init(void);
    mpu_init();

    GPR_SetUartClk();

    BSP_LoadPlatConfigFromRawFlash();

    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_FAULT_ACTION) == EXCEP_OPTION_SILENT_RESET)
        ResetLockupCfg(true, true);
    else
        ResetLockupCfg(false, false);

#ifdef FACTORY_TEST_MODE
#if ((RTE_ONE_UART_AT == 1) || (RTE_TWO_UART_AT == 1))
    SetAtUart(PORT_USART_1);
#if (RTE_TWO_UART_AT == 1)
    SetAtUart(PORT_USART_2);
#endif
#endif
#endif

    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_LOG_CONTROL) != 0)
    {
        logPortSel = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_LOG_PORT_SEL);

        if(logPortSel == PLAT_CFG_ULG_PORT_SRAM)
        {
            uniLogInitStart(RAM_FOR_UNILOG);
        }
        else if((logPortSel == PLAT_CFG_ULG_PORT_UART) || (logPortSel == PLAT_CFG_ULG_PORT_MIX))
        {
            unilogUartPort = getUnilogUartPort();

            SetUnilogUart(PORT_USART_0 + (unilogUartPort - UART_0_FOR_UNILOG), BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_LOG_BAUDRATE), true);
            uniLogInitStart(unilogUartPort);
        }
    }


    ECPLAT_PRINTF(UNILOG_PLA_STRING, build_info, P_SIG, "%s", getBuildInfo());

    ApmuWakeupProc(apmuGetAPBootFlag(), 1);

    // init cfg, should before all dedicate cfg
    WakeupPadInit();
    powerKeyFuncInit();

    #if (RTE_USB_EN == 1)
    if (BSP_UsbGetVBUSMode()==0)
    {
        if(apmuGetAPLLBootFlag()==AP_BOOT_FROM_POWER_ON)
        {
            usbstack_top_var_clear();
        }
    }
    usbstack_clear_ctx_stat();
    if (BSP_UsbGetVBUSMode()==0)
    {
#if (USB_WKMON_TASK_EXIST==1)
        usb_wkmon_pre_init();
        usb_wkmon_task_init();
#endif
        usbstack_set_ctx_vbus_mode(0,0xf);
        t_usbnet_adapt.init();
        BSP_UsbInit();
    }
    #endif
    wakeupSrc = slpManGetWakeupSrc();

    if((wakeupSrc == WAKEUP_FROM_PAD) || (wakeupSrc == WAKEUP_FROM_LPUART))
    {
        slpManStartWaitATTimer();
    }

    ResetReasonInit();

//#if ((defined TYPE_EC716S) || (defined TYPE_EC718S)) && ((defined MID_FEATURE_MODE) || (defined FEATURE_MOREROM_ENABLE))
    if(FLASH_HIB_BACKUP_EXIST == 0)
        slpManSetHibAreaNotAvaliable(true);
//#endif

    apmuGetPMUSettings(AP_STATE_IDLE);      /* should be called after log ok, and as early as possible.
                                               before calling slpManSetPmuSleepMode */

    alarmFuncInit();

#if (WDT_FEATURE_ENABLE == 1)
    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_START_WDT))
    {
        BSP_WdtInit();
        WDT_start();
    }
#endif
    slpManRegisterUsrSlpDepthCb(CheckUsrdefSlpStatus);

    FlashXIPSize = (uint32_t)&flashXIPLimit;
    ECPLAT_PRINTF(UNILOG_PMU, apflashXIPSize, P_VALUE, "AP Flash XIP Size = 0x%x", FlashXIPSize);

    efuseCalCodePtr = trimAdcGetCalCode();
    efuseT0CodePtr = trimAdcGetT0Code();

    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, adc_efuse_read, P_INFO, "adc efuse trim value, gain: 0x%x, offset: 0x%x, tcode: 0x%x, t0: 0x%x", efuseCalCodePtr->gain, efuseCalCodePtr->offset, efuseT0CodePtr->codet0, efuseT0CodePtr->t0);

#if ONENET_AUTO_UPDATE_ENABLE
    cis_deepSleepTimerCbRegister();
#endif

#if CTLWM2M_AUTO_UPDATE_ENABLE
    ctiot_deepSleepTimerCbRegister();
#endif

#ifndef TYPE_EC718M
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
    psramCodeDataInitAndpmuTimingCfg();
#endif
#else
    extern void XPI_errIrqEnable(void);
    XPI_errIrqEnable();
    extern void psramP2RamcodeDataBssInit(void);
    psramP2RamcodeDataBssInit();
#endif
}


uint32_t getAPReservedMem(uint32_t *pBase)
{
#ifdef TYPE_EC718M
    #define EC718XM_RESERVD_DEBUG_SIZE 0x80000
    *pBase = (PSRAM_END_ADDR - EC718XM_RESERVD_DEBUG_SIZE);
    return EC718XM_RESERVD_DEBUG_SIZE;
#else
    *pBase = (uint32_t)&rsvUsrRam[0];
    return sizeof(rsvUsrRam);
#endif
}

//used to get SDK_MAJOR_VERSION. should not inline(may called in lib)
__attribute__ ((noinline)) uint8_t * getSdkMajorVerion(void)
{
    return (uint8_t *)SDK_MAJOR_VERSION;
}

// tuya
extern int agpio_irq_callback(uint32_t pad_num);
/**
  \fn        void NVIC_WakeupIntHandler(void)
  \brief      NVIC wakeup interrupt handler
  \param     void
 */
void Pad0_WakeupIntHandler(void)
{
    if(slpManExtIntPreProcess(PadWakeup0_IRQn)==false)
        return;

    ECPLAT_PRINTF_OPT(UNILOG_PMU, Pad0_WakeupIntHandler_1, P_VALUE, "Pad0 Wakeup");
    uniLogFlushOut();
    // add custom code below //
    extern void pin19_irq_handle(void);
    pin19_irq_handle();
    agpio_irq_callback(19);

// #if defined CHIP_EC716
// #if (UART_DEV_DTR_WKUP_ENABLE == 1)
//     uartDevNotifySerlDtrEvt(PORT_USART_1);
// #endif
// #endif

}

pfn_PadWakeupHook p_funcPadWakeupHook = NULL;
int RegPadWakeupIntrHook(pfn_PadWakeupHook pfunc)
{
    if (p_funcPadWakeupHook)
    {
        return -1;
    }
    p_funcPadWakeupHook = pfunc;
    return 0;
}

void simcard_hotplug_ctl(bool enable)
{
    APmuWakeupPadSettings_t wakeupPadSetting;
    wakeupPadSetting.negEdgeEn = true;
    wakeupPadSetting.posEdgeEn = true;
    wakeupPadSetting.pullDownEn = false;
    wakeupPadSetting.pullUpEn = true;
    if(enable) {
        slpManSetWakeupPadCfg(WAKEUP_PAD_1, true, &wakeupPadSetting);
        NVIC_EnableIRQ(PadWakeup1_IRQn);
    } else {
        slpManSetWakeupPadCfg(WAKEUP_PAD_1, false, &wakeupPadSetting);
        NVIC_DisableIRQ(PadWakeup1_IRQn);
    }
}

extern void simcard_plug_handle(bool state);
static bool sim_insert_level = true;
void set_sim_insert_level(bool state)
{
    sim_insert_level = state;
}

extern bool tkl_cellular_get_sim_hotplug_status(uint8_t sim_id);
void Pad1_WakeupIntHandler(void)
{
    if(slpManExtIntPreProcess(PadWakeup1_IRQn)==false)
        return;

    if(tkl_cellular_get_sim_hotplug_status(0) == false) {
        ECPLAT_PRINTF_OPT(UNILOG_PMU, Pad1_WakeupIntHandler_3, P_VALUE, "Pad1 Wakeup");
        agpio_irq_callback(79);
    } else {
        NVIC_DisableIRQ(PadWakeup1_IRQn);
        uint8_t level = (slpManGetWakeupPinValue() >> WAKEUP_PAD_1) & 0x1;
        if(level == sim_insert_level) {
            //插入sim卡
            ECPLAT_PRINTF(UNILOG_PMU, Pad1_WakeupIntHandler_2, P_VALUE, "Pad1 simcard insert");
            simcard_plug_handle(true);
        } else {
            //拔出sim卡
            ECPLAT_PRINTF(UNILOG_PMU, Pad1_WakeupIntHandler_1, P_VALUE, "Pad1 simcard remove");
            simcard_plug_handle(false);
        }
    }
    uniLogFlushOut();
}

void Pad2_WakeupIntHandler(void)
{
    if(slpManExtIntPreProcess(PadWakeup2_IRQn)==false)
        return;
    ECPLAT_PRINTF_OPT(UNILOG_PMU, Pad2_WakeupIntHandler_1, P_VALUE, "Pad2 Wakeup");
    uniLogFlushOut();

    // add custom code below //

#ifdef SIM_HOT_SWAP_FEATURE
    /*
    * stop jitter timer (20ms) if running and re-start jitter timer
    */
    TIMER_stop(TIMER_INSTANCE_4);
    TIMER_start(TIMER_INSTANCE_4);
#endif
}

void Pad3_WakeupIntHandler(void)
{
    if(slpManExtIntPreProcess(PadWakeup3_IRQn)==false)
        return;

    // add custom code below //
    ECPLAT_PRINTF_OPT(UNILOG_PMU, Pad3_WakeupIntHandler_1, P_VALUE, "Pad3 Wakeup");
    uniLogFlushOut();


}

void Pad4_WakeupIntHandler(void)
{
    if(slpManExtIntPreProcess(PadWakeup4_IRQn)==false)
        return;

    // add custom code below //
    agpio_irq_callback(20);

    ECPLAT_PRINTF_OPT(UNILOG_PMU, Pad4_WakeupIntHandler_1, P_VALUE, "Pad4 Wakeup");
    uniLogFlushOut();
}

void Pad5_WakeupIntHandler(void)
{
    if(slpManExtIntPreProcess(PadWakeup5_IRQn)==false)
        return;
    // add custom code below //
    agpio_irq_callback(25);
    ECPLAT_PRINTF_OPT(UNILOG_PMU, Pad5_WakeupIntHandler_1, P_VALUE, "Pad5 Wakeup");
    uniLogFlushOut();
//     if (p_funcPadWakeupHook)
//     {
//         (*p_funcPadWakeupHook)(5);
//     }

#if defined CHIP_EC718
#if (UART_DEV_DTR_WKUP_ENABLE == 1)
    uartDevNotifySerlDtrEvt(PORT_USART_1);
#endif
#endif

}

void PwrKey_WakeupIntHandler(void)
{
    if(slpManExtIntPreProcess(PwrkeyWakeup_IRQn)==false)
        return;

    // add custom code below //
    ECPLAT_PRINTF_OPT(UNILOG_PMU, PwrKey_WakeupIntHandler_1, P_VALUE, "Pwr key Wakeup");
    uniLogFlushOut();

    pwrKeyIntHandler();
}

void ChrgPad_WakeupIntHandler(void)
{
    if(slpManExtIntPreProcess(ChrgpadWakeup_IRQn)==false)
        return;

    // add custom code below //
    ECPLAT_PRINTF_OPT(UNILOG_PMU, ChrgPad_WakeupIntHandler_1, P_VALUE, "charge Wakeup");
    uniLogFlushOut();

    chargeIntHandler();
}

/******************************************************************************
 * Usim1GpioConfig
 * Description: Configure the USIM1 PIN MUX. Called by uiccdrv task if the USIM1 enabled.
 * Input:BOOL bSetUsimFunc, whether set USIM function or not
 * Output: NULL
 * Comment: Customer can modify the actual PINs for USIM1 based on pin mux table. Here shows two options.
 * Warning: The LDOIO shall be limit at 1.8v or 3v and cannot be changed if the USIM1 enabled.
 *       Or else the voltage changed may damage the USIM card.
*******************************************************************************/
void Usim1GpioConfig(BOOL bSetUsimFunc)
{
    PadConfig_t padConfig;
    GpioPinConfig_t config;

    /*
    * Configure PIN MUX for USIM1 or GPIO
    */
#if defined CHIP_EC716 //opthion 1---GPIO 4/5/6
    if (bSetUsimFunc == TRUE)//USIM func
    {
        // GPIO4--USIM1_URSTn
        PAD_getDefaultConfig(&padConfig);
        padConfig.mux = PAD_MUX_ALT4;
        PAD_setPinConfig(USIM1_URST_OP1_PAD_INDEX, &padConfig);

        // GPIO5--USIM1_UCLK
        PAD_getDefaultConfig(&padConfig);
        padConfig.mux = PAD_MUX_ALT4;
        PAD_setPinConfig(USIM1_UCLK_OP1_PAD_INDEX, &padConfig);

        // GPIO6--USIM1_UIO
        PAD_getDefaultConfig(&padConfig);
        padConfig.mux = PAD_MUX_ALT4;
        PAD_setPinConfig(USIM1_UIO_OP1_PAD_INDEX, &padConfig);
    }
    else// GPIO func set output L as default
    {
        PAD_getDefaultConfig(&padConfig);
        padConfig.mux = PAD_MUX_ALT0;
        padConfig.pullSelect = PAD_PULL_INTERNAL;
        padConfig.pullDownEnable = PAD_PULL_DOWN_ENABLE;
        config.pinDirection = GPIO_DIRECTION_OUTPUT;
        config.misc.interruptConfig = GPIO_INTERRUPT_DISABLED;
        config.misc.initOutput = 0;

        // GPIO4--USIM1_URSTn
        GPIO_pinConfig(USIM1_URST_OP1_GPIO_INSTANCE, USIM1_URST_OP1_GPIO_PIN, &config);   // GPIO 4
        PAD_setPinConfig(USIM1_URST_OP1_PAD_INDEX, &padConfig);

        // GPIO5--USIM1_UCLK
        GPIO_pinConfig(USIM1_UCLK_OP1_GPIO_INSTANCE, USIM1_UCLK_OP1_GPIO_PIN, &config);   // GPIO 5
        PAD_setPinConfig(USIM1_UCLK_OP1_PAD_INDEX, &padConfig);

        // GPIO6--USIM1_UIO
        GPIO_pinConfig(USIM1_UIO_OP1_GPIO_INSTANCE, USIM1_UIO_OP1_GPIO_PIN, &config);   // GPIO 6
        PAD_setPinConfig(USIM1_UIO_OP1_PAD_INDEX, &padConfig);
    }
#else //option2---GPIO 12/13/14
    if (bSetUsimFunc == TRUE)//USIM func
    {
        // GPIO12--USIM1_UIO
        PAD_getDefaultConfig(&padConfig);
        padConfig.mux = PAD_MUX_ALT4;
        PAD_setPinConfig(USIM1_UIO_OP2_PAD_INDEX, &padConfig);

        // GPIO13--USIM1_URSTn
        PAD_getDefaultConfig(&padConfig);
        padConfig.mux = PAD_MUX_ALT4;
        PAD_setPinConfig(USIM1_URST_OP2_PAD_INDEX, &padConfig);

        // GPIO14--USIM1_UCLK
        PAD_getDefaultConfig(&padConfig);
        padConfig.mux = PAD_MUX_ALT4;
        PAD_setPinConfig(USIM1_UCLK_OP2_PAD_INDEX, &padConfig);
    }
    else// GPIO func
    {
        config.pinDirection = GPIO_DIRECTION_OUTPUT;
        config.misc.initOutput = 0;
        PAD_getDefaultConfig(&padConfig);
        padConfig.mux = PAD_MUX_ALT0;

        // GPIO12--USIM1_UIO
        GPIO_pinConfig(USIM1_UIO_OP2_GPIO_INSTANCE, USIM1_UIO_OP2_GPIO_PIN, &config);   // GPIO 12
        PAD_setPinConfig(USIM1_UIO_OP2_PAD_INDEX, &padConfig);

        // GPIO13--USIM1_URSTn
        GPIO_pinConfig(USIM1_URST_OP2_GPIO_INSTANCE, USIM1_URST_OP2_GPIO_PIN, &config);   // GPIO 13
        PAD_setPinConfig(USIM1_URST_OP2_PAD_INDEX, &padConfig);

        // GPIO14--USIM1_UCLK
        GPIO_pinConfig(USIM1_UCLK_OP2_GPIO_INSTANCE, USIM1_UCLK_OP2_GPIO_PIN, &config);   // GPIO 14
        PAD_setPinConfig(USIM1_UCLK_OP2_PAD_INDEX, &padConfig);
    }
#endif
}

/******************************************************************************
 * Usim1ClkLatchByAONIO
 * Description: Configure the AON IO used for USIM1 clock latch adn config USIM1 PIN as GPIO output to reduce some pulses.
 *         Called by uiccdrv task if the USIM1 enabled.
 * Input:BOOL bLatchOnHigh, whether latched on high state or not
 * Output: NULL
 * Comment: Customer can modify the actual PINs for USIM1 based on pin mux table. Here shows AONIO-26.
* Warning: The LDOIO shall be limit at 1.8v or 3v and cannot be changed if the USIM1 enabled.
*       Or else the voltage changed may damage the USIM card.
*******************************************************************************/
void Usim1ClkLatchByAONIO(BOOL bLatchOnHigh)
{
    PadConfig_t padConfig;
    GpioPinConfig_t config;

    slpManAONIOPowerOn();

    /*
    * configure AONIO-26
    */
    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(AONIO_6_PAD_INDEX, &padConfig);//gpio26
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    if (bLatchOnHigh == TRUE)
    {
        config.misc.initOutput = 1;
    }
    else
    {
        config.misc.initOutput = 0;
    }
    GPIO_pinConfig(AONIO_6_GPIO_INSTANCE, AONIO_6_GPIO_PIN, &config);//26-16

    slpManUsim1LatchEn(AonIOLatch_Enable);

    /*
    * set RST-H, CLK-H/L (bLatchOnHigh), IO-H
    */
#if 1 //opthion 1---GPIO 4/5/6
    // GPIO4--USIM1_URSTn
    PAD_getDefaultConfig(&padConfig);
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput = 1;
    GPIO_pinConfig(USIM1_URST_OP1_GPIO_INSTANCE, USIM1_URST_OP1_GPIO_PIN, &config);   // GPIO 4
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(USIM1_URST_OP1_PAD_INDEX, &padConfig);
    config.pinDirection = GPIO_DIRECTION_INPUT;
    config.misc.interruptConfig = GPIO_INTERRUPT_DISABLED;
    GPIO_pinConfig(USIM1_URST_OP1_GPIO_INSTANCE, USIM1_URST_OP1_GPIO_PIN, &config);   // GPIO 4

    // GPIO5--USIM1_UCLK
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    if (bLatchOnHigh == TRUE)
    {
        config.misc.initOutput = 1;
    }
    else
    {
        config.misc.initOutput = 0;
    }
    GPIO_pinConfig(USIM1_UCLK_OP1_GPIO_INSTANCE, USIM1_UCLK_OP1_GPIO_PIN, &config);   // GPIO 5
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(USIM1_UCLK_OP1_PAD_INDEX, &padConfig);
    config.pinDirection = GPIO_DIRECTION_INPUT;
    config.misc.interruptConfig = GPIO_INTERRUPT_DISABLED;
    GPIO_pinConfig(USIM1_UCLK_OP1_GPIO_INSTANCE, USIM1_UCLK_OP1_GPIO_PIN, &config);   // GPIO 5

    // GPIO6--USIM1_UIO
    PAD_getDefaultConfig(&padConfig);
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput = 1;
    GPIO_pinConfig(USIM1_UIO_OP1_GPIO_INSTANCE, USIM1_UIO_OP1_GPIO_PIN, &config);   // GPIO 6
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(USIM1_UIO_OP1_PAD_INDEX, &padConfig);
    config.pinDirection = GPIO_DIRECTION_INPUT;
    config.misc.interruptConfig = GPIO_INTERRUPT_DISABLED;
    GPIO_pinConfig(USIM1_UIO_OP1_GPIO_INSTANCE, USIM1_UIO_OP1_GPIO_PIN, &config);   // GPIO 6
#else//option2---GPIO 12/13/14
    // GPIO13--USIM1_URSTn
    PAD_getDefaultConfig(&padConfig);
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput = 1;
    GPIO_pinConfig(USIM1_URST_OP2_GPIO_INSTANCE, USIM1_URST_OP2_GPIO_PIN, &config);   // GPIO 13
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(USIM1_URST_OP2_PAD_INDEX, &padConfig);
    config.pinDirection = GPIO_DIRECTION_INPUT;
    config.misc.interruptConfig = GPIO_INTERRUPT_DISABLED;
    GPIO_pinConfig(USIM1_URST_OP2_GPIO_INSTANCE, USIM1_URST_OP2_GPIO_PIN, &config);   // GPIO 13

    // GPIO14--USIM1_UCLK
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    if (bLatchOnHigh == TRUE)
    {
        config.misc.initOutput = 1;
    }
    else
    {
        config.misc.initOutput = 0;
    }
    GPIO_pinConfig(USIM1_UCLK_OP2_GPIO_INSTANCE, USIM1_UCLK_OP2_GPIO_PIN, &config);   // GPIO 14
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(USIM1_UCLK_OP2_PAD_INDEX, &padConfig);
    config.pinDirection = GPIO_DIRECTION_INPUT;
    config.misc.interruptConfig = GPIO_INTERRUPT_DISABLED;
    GPIO_pinConfig(USIM1_UCLK_OP2_GPIO_INSTANCE, USIM1_UCLK_OP2_GPIO_PIN, &config);   // GPIO 14

    // GPIO12--USIM1_UIO
    PAD_getDefaultConfig(&padConfig);
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput = 1;
    GPIO_pinConfig(USIM1_UIO_OP2_GPIO_INSTANCE, USIM1_UIO_OP2_GPIO_PIN, &config);   // GPIO 12
    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(USIM1_UIO_OP2_PAD_INDEX, &padConfig);
    config.pinDirection = GPIO_DIRECTION_INPUT;
    config.misc.interruptConfig = GPIO_INTERRUPT_DISABLED;
    GPIO_pinConfig(USIM1_UIO_OP2_GPIO_INSTANCE, USIM1_UIO_OP2_GPIO_PIN, &config);   // GPIO 12
#endif
}

#ifdef SIM_HOT_SWAP_FEATURE
extern void appSetSIMHotSwapNotify(BOOL bSimPlugIn);
/******************************************************************************
 * SimHotSwapJitterTimerExpiry
 * Description: SIM hot swap jitter timer expiry function. Get the wakeup pin value after timer expired.
 * Input:NULL
 * Output: NULL
 * Comment:
*******************************************************************************/
void SimHotSwapJitterTimerIsr(void)
{
    uint8_t pinValue = ((slpManGetWakeupPinValue() & 0x04) >> 2);

    ECPLAT_PRINTF(UNILOG_PMU, SimHotSwapJitterTimerIsr_1, P_VALUE, "jitter timer expired");

    appSetSIMHotSwapNotify(pinValue);

    TIMER_stop(TIMER_INSTANCE_4);
}

/******************************************************************************
 * SimHotSwapJitterTimerConfig
 * Description: Configure hw timer for SIM hot swap jitter process.
 * Input:NULL
 * Output: NULL
 * Comment:
*******************************************************************************/
void SimHotSwapJitterTimerConfig(uint8_t timerInstance)
{
    TimerConfig_t timerConfig;
    ClockId_e clockId;
    ClockSelect_e clockSel;
    IRQn_Type timerIRQ;

    switch (timerInstance)
    {
        case 1:
            clockId = FCLK_TIMER1;
            clockSel = FCLK_TIMER1_SEL_26M;
            timerIRQ = PXIC0_TIMER1_IRQn;
            break;
        case 2:
            clockId = FCLK_TIMER2;
            clockSel = FCLK_TIMER2_SEL_26M;
            timerIRQ = PXIC0_TIMER2_IRQn;
            break;
        case 4:
            clockId = FCLK_TIMER4;
            clockSel = FCLK_TIMER4_SEL_26M;
            timerIRQ = PXIC0_TIMER4_IRQn;
            break;
        default://defalut use timer0, timer3/timer5 reserved
            clockId = FCLK_TIMER0;
            clockSel = FCLK_TIMER0_SEL_26M;
            timerIRQ = PXIC0_TIMER0_IRQn;
            break;
    }

    // Config TIMER clock, source from 26MHz and divide by 1
    CLOCK_setClockSrc(clockId, clockSel);
    CLOCK_setClockDiv(clockId, 1);

    TIMER_driverInit();

    // Config timer period as 20ms, match0 value is 26000*20 = 520 000= 0x7EF40
    TIMER_getDefaultConfig(&timerConfig);
    timerConfig.reloadOption = TIMER_RELOAD_ON_MATCH0;
    timerConfig.match0 = 0x7EF40;

    TIMER_init(timerInstance, &timerConfig);

    TIMER_interruptConfig(timerInstance, TIMER_MATCH0_INTERRUPT, TIMER_INTERRUPT_PULSE);
    TIMER_interruptConfig(timerInstance, TIMER_MATCH1_INTERRUPT, TIMER_INTERRUPT_DISABLE);
    TIMER_interruptConfig(timerInstance, TIMER_MATCH2_INTERRUPT, TIMER_INTERRUPT_DISABLE);

    // Enable IRQ
    XIC_SetVector(timerIRQ, SimHotSwapJitterTimerIsr);
    XIC_EnableIRQ(timerIRQ);

}

/******************************************************************************
 * SimHotSwapInit
 * Description: SIM hot swap init. configure wakeup pad and jitter timer
 * Input:NULL
 * Output: NULL
 * Comment: here use wakeup2 PIN to monitor SIM pluged in (high level) or out (low level) for example.
 *        It can be changed by customer.
*******************************************************************************/
void SimHotSwapInit(void)
{
    if (apmuGetAPLLBootFlag() == AP_BOOT_FROM_POWER_ON)
    {
        APmuWakeupPadSettings_t wakeupPadSetting;

        wakeupPadSetting.negEdgeEn = true;
        wakeupPadSetting.posEdgeEn = true;
        wakeupPadSetting.pullDownEn = false;
        wakeupPadSetting.pullUpEn = true;

        //clear the IRQ if enabled by WakeupPadInit()
        NVIC_DisableIRQ(PadWakeup2_IRQn);
        NVIC_ClearPendingIRQ(PadWakeup2_IRQn);

        slpManSetWakeupPadCfg(WAKEUP_PAD_2, true, &wakeupPadSetting);
    }

    /*
    * configure jitter timer
    */
    SimHotSwapJitterTimerConfig(TIMER_INSTANCE_4);

    //enable wakeup2 IRQ
    NVIC_EnableIRQ(PadWakeup2_IRQn);
}
#endif


/*===========================================================================
* Description: LdoFemVbat scenario settings.
* Called by:   ecMain
* Calls:       none.
* Input:       none.
* Output:      none.
*===========================================================================*/
void RfLdoFemVbatTypeSet( void )
{
    ShareInfoAPSetLdoFemVbatType(LDOFEMVBAT_SCENARIO_TYPE3);
}


/*===========================================================================
* Description: print ap idle percent.
* Description:
*               AT+ECIDLEP=num,enable,20ms period*num
                ex:AT+ECIDLEP=3,60ms period,support maximum period of 120ms when num = 6
*               AT+ECIDLEP=0,disable
*               AT+ECIDLEP?,read current configuration
*               AT+ECIDLEP=?,useage print
* Called by:   appInit.
* Input:       none.
* Output:      none.
*===========================================================================*/
AP_PLAT_COMMON_DATA osTimerId_t ecApPerformaceCommonTimer = NULL;
AP_PLAT_COMMON_DATA osTimerId_t ecApPerformaceApmTimer = NULL;

extern void XpIdleTimeStatisPrint(uint32_t periodMs);
extern void ApmTimeStatisPrint(uint32_t periodMs);
extern void setSysPerformaceMode(uint32_t mode);

void apPrintIdlePercent(void)
{
    //static osTimerId_t ap20msTimer;
    uint32_t performanceMode = 0;
    uint32_t performancePeriod = 0;
    #ifdef TYPE_EC718M
    uint32_t performanceApmPeriod = 0;
    #endif
    osStatus_t ret = 0;

    performanceMode = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_PERFORMANCE_INFO_MODE);
    #ifdef TYPE_EC718M
    performanceApmPeriod = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_PERFORMANCE_INFO_APM_PERIOD);
    #endif
    performancePeriod = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_PERFORMANCE_INFO_PERIOD);

    setSysPerformaceMode(performanceMode);

    if(performanceMode != EC_PER_INFO_NONE)
    {
        if(((performanceMode&EC_PER_INFO_CACHE) == EC_PER_INFO_CACHE)||((performanceMode&EC_PER_INFO_HEAP) == EC_PER_INFO_HEAP)||((performanceMode&EC_PER_INFO_IDLETASK) == EC_PER_INFO_IDLETASK))
        {
            if(ecApPerformaceCommonTimer == NULL)
            {
                ecApPerformaceCommonTimer = osTimerNew((osTimerFunc_t)XpIdleTimeStatisPrint, osTimerPeriodic, (void *)(uint32_t)performancePeriod, NULL);
            }
            ret = osTimerStart(ecApPerformaceCommonTimer, performancePeriod);
            if(ret != 0)
            {
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, ApPerformanceInfo_IdlePercent1, P_WARNING, "Start Idle Percent Timer Fail!");
            }
        }
        #ifdef TYPE_EC718M
        if(((performanceMode&EC_PER_INFO_APM0) == EC_PER_INFO_APM0)||((performanceMode&EC_PER_INFO_APM1) == EC_PER_INFO_APM1)||((performanceMode&EC_PER_INFO_APM2) == EC_PER_INFO_APM2))
        {
            if(ecApPerformaceApmTimer == NULL)
            {
                ecApPerformaceApmTimer = osTimerNew((osTimerFunc_t)ApmTimeStatisPrint, osTimerPeriodic, (void *)(uint32_t)performanceApmPeriod, NULL);
            }
            ret = osTimerStart(ecApPerformaceApmTimer, performanceApmPeriod);
            if(ret != 0)
            {
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, ApPerformanceInfo_apmPercent, P_WARNING, "Start Apm Percent Timer Fail!");
            }
        }
        #endif
    }
}

uint8_t* getDebugDVersion(void)
{
    return (uint8_t*)DB_VERSION_UNIQ_ID;
}

void mbtkInitENGpio(void)
{
    uint32_t gpio_num = mbtk_get_EN_gpio_num();
    uint32_t gpio_addr = mbtk_get_EN_gpio_addr();
    PadConfig_t padConfig;
    GpioPinConfig_t config;

    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = 0;
    PAD_setPinConfig(gpio_addr, &padConfig);

    config.pinDirection = 1;
    config.misc.initOutput = 1;
    GPIO_pinConfig((gpio_num / 16), (gpio_num % 16), &config);
    GPIO_pinWrite((gpio_num / 16), 1 << (gpio_num % 16), (1) << (gpio_num % 16));
}

