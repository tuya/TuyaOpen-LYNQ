/* --------------------------------------------------------------------------
 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    Bootmain.c
 *      Purpose: for cm3 bootrom
 *
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#include "ec7xx.h"
#include <stdio.h>
#include "error.h"
#include "boot.h"
#include "common.h"
#include "image.h"
#include <string.h>
#include "cache.h"
#include "mem_map.h"
#include "bl_bsp.h"
#include "pwrkey.h"
#include "plat_config.h"
#include "gpr_common.h"

#if (WDT_FEATURE_ENABLE==1)
#include "wdt.h"
#endif
#include "bl_uart.h"
#include "bl_print.h"
#include "sctdef.h"

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#if (WDT_FEATURE_ENABLE==1)
#define WDT_TIMEOUT_VALUE        (20)            // in unit of second, shall be less than 256s
#endif

#define XIP_BOOT                  0
#define DOWNLOAD_BOOT             1

/* default port type */
#ifdef FEATURE_FOTA_USBURC_ENABLE
#define FOTA_DFT_URC_PORT_TYPE    PLAT_CFG_FOTA_URC_PORT_USB
#else
#define FOTA_DFT_URC_PORT_TYPE    PLAT_CFG_FOTA_URC_PORT_UART
#endif

/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                      FUNCTION DECLEARATION                                 *
 *----------------------------------------------------------------------------*/

#if (defined CHIP_EC718)
extern uint8_t FLASH_eraseSector(uint32_t BlockAddress);
#endif

extern int32_t efuseSWReadSafePolling(uint8_t ByteLoc, uint8_t ByteLen, uint8_t *PBuff);
extern int32_t EFuseSWBurn(uint32_t BitLoc, uint8_t BitLen, uint8_t *PBuff);
extern void Flash_NVICSwReset(void);


extern void FotaProcedure(void);
extern void Bltransfer_Control(uint32_t p_base_addr);

/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

DownloadCtrl            GDownloadCtrl;

volatile usart_port_t   UsartLogPortIdx   = PORT_USART_0;  // set value to PORT_USART_INVALID to disable log output
volatile usart_port_t   UsartUrcPortIdx   = PORT_USART_1;

URCSelPrintType         URCSelSerlType    = URCUart1PrintType;

uint32_t                URCBaudValue      = 115200;
uint8_t                 URCSelSerlIdx     = 0;
uint8_t                 URCEnable         = 0;
uint8_t                 URCPrintActive    = 0;
/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/

void uDelay(void)
{
    int32_t i=0;
    for(i=0;i<70;i++);
}

void mDelay(int ms)
{
    int32_t i=0;
    for(i=0;i<70000;i++);
}

void Delay(int sec)
{
    int32_t i=0;
    for(i=0;i<1000;i++)
    {
       mDelay(1);
    }
}

#ifdef CONFIG_PROJ_APP_SECURITY_BOOT
extern unsigned char gVerifyPubKey[56];
extern unsigned char blPubKey[56];

uint8_t *GetVerifyPubKey(void)
{
    return &gVerifyPubKey[0];
}
PLAT_BL_CIRAM_FLASH_TEXT uint8_t *GetBlPubKey(void)
{
    return &blPubKey[0];
}
#endif

PLAT_BL_AIRAM_PRE2_TEXT void SystemXIPFastBoot(void)
{
    //printf("bootloader try xip boot system start!\n");
#if 1
    CleanBootRomResouce();

    //flush cache
    DisableICache();
    __DSB();
    __ISB();
    EnableICache();
#endif
    Bltransfer_Control(AP_FLASH_LOAD_ADDR);
    while(1);
}

extern UINT32 flashXIPLimit;
PLAT_BL_UNCOMP_FLASH_TEXT uint32_t SystemXIPNormalBoot(void)
{
    if(UsartLogPortIdx != PORT_USART_INVALID)
    {
        BL_LOGI("bootloader flashXIPLimit(0x%x), try normal boot system start!\n", &flashXIPLimit);
    }

#ifdef CONFIG_PROJ_APP_SECURITY_BOOT//    if (IsSecuritySupport() == 1)//CONFIG_PROJ_APP_SECURITY_BOOT  == y
    {
        uint8_t* pBlKey = GetBlPubKey();
        int32_t bureResult = 0;

        if (GetBRInfo()->SecureBootEnabled == 0)//haven't burn secure en bit
        {
            //burn en bit and pubkey
            BL_TRACE("blPubKey:\n");
            for(int j=0; j<2; j++){
                for(int k=0; k<28; k++)
                {
                    BL_TRACE("0x%x,",pBlKey[k+28*j]);
                }
                BL_TRACE("\n");
            }
            BL_TRACE("burn bl pubkey...\n");
            uint8_t  tmpLen   = 56;  // Efuse ECDSA key part, total 56 bytes
            uint8_t  temphead = 0;
            uint16_t bitStart = 512; // Efuse AES part, start at bit 192

            // always on pclk_aon/cpmu/pmdig/rfdig/pclk_tmu , since very low leakage from test
            //GPR->APBGPPCLKENAP |= 0xBF;

            *(volatile uint32_t*)0x4f000010 |= 1<<5;
            *(volatile uint32_t*)0x4f08008c = 0x1;//EFuseBurnIgnorePOR1()

            do
            {
                bureResult = EFuseSWBurn(bitStart, 64, &pBlKey[temphead]); // Every time it can burn 8 bytes at most
                BL_TRACE("bureResult=%d\n",bureResult);
                temphead += 8;
                tmpLen -= 8;
                bitStart += 64;
            }while (tmpLen > 0);

            BL_TRACE("burn secure en bit...and reset\n");
            uint8_t en = 1;
            EFuseSWBurn(145, 1, &en); // burn secure en bit
#if (defined CHIP_EC718)
            //for 718 718p/s 718pm/sm etc, since fuse burned loaded from mirror not from hw fuse, 
            //so erase and update before reset, let fuse-mirror will reload once from hw fuse  next time, 
            FLASH_eraseSector(FUSE_FLASH_MIRROR_ADDR);
#endif
            *(volatile uint32_t*)0x4f02000c |= 1;//send reset command in bootrom
            Flash_NVICSwReset();
        }
        else
        {
            BL_TRACE("secure en bit has set. bl pubkey:\n");
            uint8_t readBlKey[56] = {0};
            for(int i=0; i<7; i++)
            {
                efuseSWReadSafePolling(16+i*2, 2, readBlKey+i*8); //read ecdsa key
            }
            for(int j=0; j<2; j++){
                for(int k=0; k<28; k++)
                {
                    BL_TRACE("0x%x,",readBlKey[k+28*j]);
                }
                BL_TRACE("\n");
            }
            if(memcmp(&readBlKey[0], pBlKey, sizeof(readBlKey)) == 0){
                BL_TRACE("check bl pubkey success\n");
            }
        }
        BL_TRACE("start secure boot process\n");

        //load ih from flash
        uint32_t RetValue = LoadVerifyImageHead();
        if (RetValue != NoError)
        {
            return RetValue;
        }
        //load image from flash
        RetValue = LoadVerifyImageBody();
        if (RetValue != NoError)
        {
            return RetValue;
        }
    }
#endif

    //BSP_QSPI_XIP_Mode_Enable(0);
    BL_CustomDeInit();
    BSP_DeInit();

    DisableICache();
#ifdef TYPE_EC718M
    /*
        Bootloader should to clean PSRAM-P0-Cache
        due to PSRAM-P0 used to decompress code.
    */
    extern BOOL PCacheCleanAll(uint8_t inst);
    PCacheCleanAll(0);
	PCacheCleanAll(1);
	PCacheCleanAll(2);
#endif
    __DSB();
    __ISB();

    #if (!defined FPGA_TEST && !defined SOC_TEST)
    // 718m fpga bringup
    BSP_apAccCmsbDisEn(CP_FLASH_LOAD_ADDR);
    #endif

    Bltransfer_Control(AP_FLASH_LOAD_ADDR);
    while(1);

}


PLAT_BL_UNCOMP_FLASH_TEXT void NMI_Handler()
{
    BL_TRACE("WDT timeout!!! Enter NMI Handler!!!\r\n");

    while(1);
}

PLAT_BL_UNCOMP_FLASH_TEXT void MemManage_Handler()
{
    while(1);
}

PLAT_BL_UNCOMP_FLASH_TEXT void BusFault_Handler()
{
    while(1);
}

PLAT_BL_UNCOMP_FLASH_TEXT void UsageFault_Handler()
{
    while(1);
}

PLAT_BL_UNCOMP_FLASH_TEXT void DebugMon_Handler()
{
    while(1);
}

PLAT_BL_UNCOMP_FLASH_TEXT void SVC_Handler()
{

}
PLAT_BL_UNCOMP_FLASH_TEXT void PendSV_Handler()
{

}

PLAT_BL_UNCOMP_FLASH_TEXT void  SysTick_Handler()
{

}



#if (WDT_FEATURE_ENABLE == 1)

/*
 *  WDT Initialize, wdt timeout value is 20s
 *  Parameter:   none
 */
PLAT_BL_CIRAM_FLASH_TEXT void BSP_WdtInit(void)
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

PLAT_BL_CIRAM_FLASH_TEXT void BL_SetUartPinmuxAndClk(void)
{
    // Pin initialize
    PadConfig_t config;
    PAD_getDefaultConfig(&config);

    config.mux = RTE_UART0_TX_FUNC;
    PAD_setPinConfig(RTE_UART0_TX_BIT, &config);

    config.mux = RTE_UART1_TX_FUNC;
    PAD_setPinConfig(RTE_UART1_TX_BIT, &config);

    config.pullSelect = PAD_PULL_INTERNAL;
    config.pullUpEnable = PAD_PULL_UP_ENABLE;
    config.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    config.mux = RTE_UART0_RX_FUNC;
    PAD_setPinConfig(RTE_UART0_RX_BIT, &config);

    config.mux = RTE_UART1_RX_FUNC;
    PAD_setPinConfig(RTE_UART1_RX_BIT, &config);

    GPR_clockDisable(FCLK_UART0);
    GPR_setClockSrc(FCLK_UART0, FCLK_UART0_SEL_26M);

    GPR_clockDisable(FCLK_UART1);
    GPR_setClockSrc(FCLK_UART1, FCLK_UART1_SEL_26M);

}

PLAT_BL_CIRAM_FLASH_TEXT void vcomUrcPutChar(uint8_t vcomNo, uint8_t *buff, uint32_t size)
{
#ifdef FEATURE_FOTA_USBURC_ENABLE
    int vcom_app_send_proc(uint8_t vcom_num, uint8_t* sndbuf_ptr, uint32_t size, uint8_t timeout);
    vcom_app_send_proc(vcomNo, (uint8_t*)buff, 1, 50);
#endif
}

PLAT_BL_CIRAM_FLASH_TEXT void URCPutChar(int ch)
{
    if (URCSelSerlType == URCUart1PrintType)
    {
        if(UsartUrcPortIdx != PORT_USART_INVALID)
        {
            UART_send(UsartUrcPortIdx, (uint8_t*)&ch, 1, 0xFFFFFFFF);
        }
    }
    else if(URCSelSerlType >= URCVCom0PrintType)
    {
        vcomUrcPutChar(URCSelSerlType - URCVCom0PrintType, (uint8_t*)&ch, 1);
    }
}

/*
 *  retarget for _write implementation
 *  Parameter:      ch: character will be out
 */
PLAT_BL_CIRAM_FLASH_TEXT void LOGPutChar(uint8_t ch)
{
    if(UsartLogPortIdx != PORT_USART_INVALID)
    {
        UART_send(UsartLogPortIdx, (uint8_t*)&ch, 1, 0xFFFFFFFF);
    }
}

#if defined ( __GNUC__ )

/*
 *  retarget for _write implementation
 *  Parameter:      ch: character will be out
 */
PLAT_BL_CIRAM_FLASH_TEXT int io_putchar(int ch)
{
    if(URCEnable && URCPrintActive)
    {
        URCPutChar(ch);
    }
    else
    {
        LOGPutChar(ch);
    }
    return 0;
}

/*
 *  retarget for _read implementation
 *  Parameter:      ch: character will be read
 */
PLAT_BL_CIRAM_FLASH_TEXT int io_getchar()
{
    uint8_t ch = 0;

    if(UsartLogPortIdx != PORT_USART_INVALID)
    {
        UART_receive(UsartLogPortIdx, (uint8_t*)&ch, 1, 0xFFFFFFFF);
    }

    return (ch);
}

__attribute__((weak,noreturn)) void __aeabi_assert (const char *expr, const char *file, int line)
{
    BL_TRACE("Assert, expr:%s, file: %s, line: %d\r\n", expr, file, line);
    while(1);
}


void __assert_func(const char *filename, int line, const char *assert_func, const char *expr)
{
    BL_TRACE("Assert, expr:%s, file: %s, line: %d\r\n", expr, filename, line);
    while(1);
}




#elif defined (__CC_ARM)

/*
 *  retarget for printf implementation
 *  Parameter:      ch: character will be out
 *                  f:  not used
 */
int fputc(int ch, FILE *f)
{
    LOGPutChar(ch)

    return 0;
}

/*
 *  retarget for scanf implementation
 *  Parameter:      f:  not used
 */
int fgetc(FILE *f)
{
    uint8_t ch = 0;

    if(UsartLogPortIdx != PORT_USART_INVALID)
    {
        UART_receive(UsartLogPortIdx, (uint8_t*)&ch, 1, 0xFFFFFFFF);
    }
    return (ch);
}

__attribute__((weak,noreturn)) void __aeabi_assert (const char *expr, const char *file, int line)
{
    BL_TRACE("Assert, expr:%s, file: %s, line: %d\r\n", expr, file, line);
    while(1);
}

#endif

/*
 *  URC set enable and baudrate value config
 *  Parameter:   enable uart1 init for urc, 0-disable, 1-enable
 *  Parameter:   baudrate for UART1(URC)
 */
PLAT_BL_CIRAM_FLASH_TEXT void BSP_URCSetCfg(uint8_t enable, uint32_t baud, URCSelPrintType serlType, uint8_t serlIdx)
{
    URCEnable      = enable;
    URCBaudValue   = baud;
    URCSelSerlType = serlType;
    URCSelSerlIdx  = serlIdx;

    if(serlType == URCUart1PrintType)
    {
        UsartUrcPortIdx = PORT_USART_0 + serlIdx;
    }
}

PLAT_BL_CIRAM_FLASH_TEXT void SelNormalOrURCPrint(uint32_t Sel)
{
    if(URCEnable==0)
    {
        return;
    }
    URCPrintActive = Sel;
}


/*
 * FUNCTION :BL_CustomInit, called at start of BSP_Init

 */
PLAT_BL_CIRAM_FLASH_TEXT void BL_CustomInit(void)
{
    URCSelPrintType  serlType = URCUart1PrintType;
    uint32_t         urcBaud = 115200;
    uint8_t          portType = FOTA_DFT_URC_PORT_TYPE;
    uint8_t          portIdx = 0;
    uint8_t          portSel = (portType << 4) | portIdx;

#if (WDT_FEATURE_ENABLE == 1)
    BSP_WdtInit();
    WDT_start();
#endif

    BSP_LoadPlatConfigFromRawFlash();

    urcBaud = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_AT_PORT_BAUDRATE);
    if(!urcBaud || urcBaud == 0xffffffff)
    {
        urcBaud = 115200;
    }

    portSel  = (uint8_t)BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_FOTA_URC_PORT_SEL);
    portType = (portSel >> 4);
    portIdx  = (portSel & 0xf);
    if(portType >= PLAT_CFG_FOTA_URC_PORT_MAXTYPE)
    {
        portType = FOTA_DFT_URC_PORT_TYPE;
    }

    if(portType == PLAT_CFG_FOTA_URC_PORT_USB)
    {
    #ifdef FEATURE_FOTA_USBURC_ENABLE
        if(portIdx > PLAT_CFG_FOTA_URC_USB_PORT_IDX_MAX)
        {
            portIdx = 0;  /* default portIdx of USB */
        }
        serlType = URCVCom0PrintType + portIdx;
    #else
        /* set it to 'uart1' */
        serlType = URCUart1PrintType;
        portIdx  = 1;
    #endif
    }
    else
    {
        if(portIdx > PLAT_CFG_FOTA_URC_UART_PORT_IDX_MAX)
        {
            portIdx = 1;  /* default portIdx of UART */
        }
    }

    BSP_URCSetCfg(1, urcBaud, serlType, portIdx);

    BL_SetUartPinmuxAndClk();

    uartPortFrameFormat_t fmt = {.dataBits    = 8,
                                 .parity      = 0,
                                 .stopBits    = 1,
                                 .flowControl = 0
                                };

    UART_init(UsartLogPortIdx, &fmt, 115200ul);
    if(serlType == URCUart1PrintType)
    {
        UART_init(UsartUrcPortIdx, &fmt, urcBaud);
    }

#if defined ( __GNUC__ )
    // Disable I/O buffering for stdout, so chars are sent out as soon as they are printed
    //setvbuf(stdout, NULL, _IONBF, 0);
#endif
}

PLAT_BL_CIRAM_FLASH_TEXT void BL_CustomDeInit(void)
{
    UART_Deinit(UsartLogPortIdx);
    UART_Deinit(UsartUrcPortIdx);

#if (WDT_FEATURE_ENABLE == 1)
    WDT_stop();
    WDT_deInit();
#endif
}

/*
 * FUNCTION :main

 */
PLAT_BL_CIRAM_FLASH_TEXT int ec_main(void)
{
    uint32_t RetValue = NoError;

    BSP_Init();
    BL_CustomInit();

    //void vcom_urc_test_entry(void);
    //vcom_urc_test_entry();
    if(pwrKeyGetPwrKeyMode() == PWRKEY_PWRON_MODE)
        pwrkeyPwrOnDebounce(2000);

    #ifdef FEATURE_FOTA_ENABLE
    //fotaEnable  = (uint8_t)BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_FOTA_CONTROL);
    //if(fotaEnable == 1)
    {
        extern void decompressGetAddrInfo(void);
        decompressGetAddrInfo();

        FotaProcedure();
    }
    #endif

    RetValue = SystemXIPNormalBoot();
    if (RetValue != NoError)
    {
        BL_TRACE("bootloader try normal boot system failed(0x%x)!\n", RetValue);
    }
    while(1);
}

PLAT_BL_AIRAM_PRE2_TEXT void Bltransfer_Control(uint32_t p_base_addr)
{
    uint32_t *msp_addr = (uint32_t *)p_base_addr;
    typedef void *jmp_fun(void);
    __set_MSP(*msp_addr);
    jmp_fun *jump = (jmp_fun *)(*(msp_addr+1));
    (*jump)();
}


