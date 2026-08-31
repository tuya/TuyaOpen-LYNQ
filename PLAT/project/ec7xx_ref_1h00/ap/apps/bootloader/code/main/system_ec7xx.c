/**************************************************************************//**
 * @file     system_ARMCM3.c
 * @brief    CMSIS Device System Source File for
 *           ARMCM3 Device Series
 * @version  V5.00
 * @date     07. September 2016
 ******************************************************************************/
/*
 * Copyright (c) 2009-2016 ARM Limited. All rights reserved.
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
 */

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#include "ec7xx.h"
#include "mpu_armv7.h"
#include "Driver_Common.h"
#include "cache.h"
#include "mem_map.h"
#include "slpman.h"
#include "bl_bsp.h"
#ifdef RAMCODE_COMPRESS_EN
#include "LzmaEc.h"
#endif
#include "sctdef.h"
#ifdef TYPE_EC718M
#include "xpi_psram.h"
#endif

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define CORE_CLOCK_SEL_ADDRESS    (0x4f000020)


/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
#if defined(__CC_ARM)
    extern uint32_t __Vectors;
#elif defined(__GNUC__)
    extern uint32_t __isr_vector;
#endif
#endif

RAM_BOOT_NOINIT uint32_t SystemCoreClock = 0;

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/
extern void CopyPre1RamtoImage(void);
extern void CopyPre2RamtoImage(void);
extern void CopyRWDataFromBin(void);
extern void SetZIDataToZero(void);
extern void CopyOtherCodetoImage(void);
extern void SystemXIPFastBoot(void);
extern void GPR_SetClkCc(void);
extern void GPR_setflashClockPreInit(uint8_t div);
extern void flashMirrorInitandChk( void );
extern void trimEfuseAon( void );
extern void pwrKeyStoreFirstPwrOnFlag(void);

#if (defined CHIP_EC718)
#ifdef TYPE_EC718M
extern void SetBootZIDataToZero(void);
extern uint8_t XPSRAM_init(void);
extern void PAD_setDefaultDrvStrength0(void);
#endif
extern void trimWriteEfuseResult2FlashMirror(void);
extern void trimGetTrimResult(void);
extern void trimFlashPsramAndSimoDig(bool fromReset);
#elif (defined CHIP_EC716)
extern void trimFlashAndSimoDig(void);
#endif

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTIONS                                     *
 *----------------------------------------------------------------------------*/


/**
  \fn        sysROAddrCheck(uint32_t addr)
  \brief     This function is called in flash erase or write apis to prevent
             unexpected access the bootloader image read only area..
  \param[in] Addr: flash erase or write addr
  \note      Be careful if you want change this function.
 */
PLAT_BL_UNCOMP_FLASH_TEXT static uint8_t sysROAddrCheck(uint32_t addr)
{
    //protect 2 hdr
#ifdef CONFIG_PROJ_APP_SECURITY_BOOT
    if(addr <(SYS_SEC_HAED_ADDR))
#else
    if(addr <(FUSE_FLASH_MIRROR_ADDR))
#endif
    {
        return 1;
    }

    //protect bl
    if((addr >=(BOOTLOADER_FLASH_LOAD_ADDR-AP_FLASH_XIP_ADDR))
          && (addr <(BOOTLOADER_FLASH_LOAD_ADDR+BOOTLOADER_FLASH_LOAD_SIZE-AP_FLASH_XIP_ADDR)))
    {
          return 1;
    }
    return 0;
}

 /*----------------------------------------------------------------------------*
  *                      GLOBAL FUNCTIONS                                      *
  *----------------------------------------------------------------------------*/

 /**
  \fn        sysROSpaceCheck(uint32_t addr, uint32_t size)
  \brief     This function is called in flash erase or write apis to prevent
             unexpected access the bootloader image read only area..
  \param[in] Addr: flash erase or write addr
             Addr: flash erase or write size
  \note      Be careful if you want change this function.
 */
PLAT_BL_UNCOMP_FLASH_TEXT uint8_t sysROSpaceCheck(uint32_t addr, uint32_t size)
{
    if(sysROAddrCheck(addr))
    {
        return 1;
    }
    if (sysROAddrCheck(addr+size - 1))
    {
        return 1;
    }
    return 0;
}

PLAT_UNCOMP_FLASH_TEXT void TrimFlashAndSimoPre300MCoreClk(void)
{
#if (defined CHIP_EC718)
#if (defined TYPE_EC718M)
    *(volatile uint32_t *)(0x4f0200c0) = (*(volatile uint32_t *)(0x4f0200c0)) & (~(1<<10));  //analog: dcdc11 bypass off
    PAD_setDefaultDrvStrength0();
#endif

    if(((*(uint32_t *)0x4f0201A0)&0xe0) == 0)   // power on
    {
        SystemCoreClockUpdate();
        trimGetTrimResult();
        trimFlashPsramAndSimoDig(true);
    }
    else
    {
        trimFlashPsramAndSimoDig(false);
    }
#elif (defined CHIP_EC716)

    SystemCoreClockUpdate();

#if defined(TYPE_EC716E)
    // pmudig clk enable
    *(volatile uint32_t *)(0x4f000030) |= (1<<4);
    // should delay before setting smio_cfg1[7]
    delay_us(1);
    //PMUDIG(SIMO putput + 100mV):smio_cfg1[7] = 1
    *(volatile uint32_t *)(0x4f040318) |= (1<<7);
    // pmudig clk disable
    *(volatile uint32_t *)(0x4f000030) &= ~(1<<4);
#endif

    trimFlashAndSimoDig();
#endif
    return ;
}


/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
PLAT_UNCOMP_FLASH_TEXT void SystemCoreClockUpdate (void)
{
#if (defined FPGA_TEST || defined SOC_TEST)
    SystemCoreClock = 102400000;
#else
    switch((*((uint32_t *)CORE_CLOCK_SEL_ADDRESS)) & 0x3)
    {
        case 0:
            SystemCoreClock = 26000000U;
            break;
        case 1:
#if defined TYPE_EC718M
            if((*((uint32_t *)CORE_CLOCK_SEL_ADDRESS)) & (0x1 << 15))
            {
                if((*((uint32_t *)CORE_CLOCK_SEL_ADDRESS)) & (0x1 << 12))
                {
                    SystemCoreClock = 409600000U;
                }
                else
                {
                    SystemCoreClock = 307200000U;
                }
            }
            else
            {
                SystemCoreClock = 204800000U;
            }
            break;
#else
            if((*((uint32_t *)CORE_CLOCK_SEL_ADDRESS)) & (0x1 << 15))
            {
                SystemCoreClock = 307200000U;
            }
            else
            {
                SystemCoreClock = 204800000U;
            }
            break;
#endif
        case 2:
            SystemCoreClock = 102400000U;
            break;
        case 3:
            SystemCoreClock = 32768U;
            break;
    }
#endif
}



/* make sure the ramcode is not inlined
set as soon as possiable, so need a seperate section for the setting ramode
*/
PLAT_BL_AIRAM_PRE1_TEXT void SetFlashCoreClkPreInitInBl(void)
{
    uint32_t delay_loop = 0x10;
    GPR_SetClkCc();

    #if (defined CHIP_EC718) && (defined TYPE_EC718M) 
    #if (defined FPGA_TEST || defined SOC_TEST)
    GPR_setflashClockPreInit(3);//70/3=23M
    #else
    GPR_setflashClockPreInit(6);//612/6=102M
    #endif
    #elif (defined CHIP_EC718) && !(defined TYPE_EC718M) 
    GPR_setflashClockPreInit(7);//612/7=87M	
	#elif defined CHIP_EC716
    GPR_setflashClockPreInit(6);//612/6=102M
    #endif
    while(delay_loop--);
}


/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/
extern void decompressRamCodeGetAddrInfo(void);

PLAT_BL_UNCOMP_FLASH_TEXT void SystemInit (void)
{
    DisableICache();

    EnableICache();

    // Fix bug that UART0 IER is enabled in bootrom
    USART_0->IER = 0;

    CopyPre1RamtoImage();

    TrimFlashAndSimoPre300MCoreClk();

    SetFlashCoreClkPreInitInBl();

    CopyPre2RamtoImage();

    if ((*(uint32_t *)0x4f0201A0)&0xe0)//wakeup
    {
        #ifdef TYPE_EC718M
        SystemCoreClockUpdate();
        XPSRAM_init();
        XPSRAM_postSleepFlow();
        #endif
        if (SystemWakeUpBootInit()==TRUE)
        {
            SystemXIPFastBoot();
        }
    }

    #if (!defined FPGA_TEST && !defined SOC_TEST)
    BSP_apAccCmsbEn();
    #endif

#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
#if defined(__CC_ARM)
    SCB->VTOR = (uint32_t) &__Vectors;
#elif defined (__GNUC__)
    SCB->VTOR = (uint32_t) &__isr_vector;
#endif
#endif

#ifdef TYPE_EC718M
    SetBootZIDataToZero();

    SystemCoreClockUpdate();

    XPSRAM_init();

    XPSRAM_setRetMode(false, XPSRAM_RET_SLEEP1);
#endif

    #ifdef RAMCODE_COMPRESS_EN
    decompressRamCodeGetAddrInfo();
    #endif

    CopyOtherCodetoImage();

    /*move the RW data in the image to its excution region*/
    CopyRWDataFromBin();

    /*append the ZI region. TODO: maybe ZI data need not to be 0,
    random value maybe aslo fine for application, if so we could
    remove this func call, since it takes a lot of time*/
    SetZIDataToZero();

    SystemNormalBootInit();

#ifndef TYPE_EC718M
    SystemCoreClockUpdate();
#endif

#if (defined CHIP_EC718) 
    // 718 719 write efuse to flash mirror
    trimWriteEfuseResult2FlashMirror();
#endif

    pwrKeyStoreFirstPwrOnFlag();

    trimEfuseAon();//set to aon only when por, and aon clock should default en

}

