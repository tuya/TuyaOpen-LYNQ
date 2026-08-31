#include <string.h>
#include "commontypedef.h"
#include "ec7xx.h"
#include "cache.h"
#include "mpu_armv7.h"
#include "mem_map.h"
#include "exception_process.h"
#include "apmu_external.h"
#ifdef FEATURE_LZMA_ENABLE
#include "LzmaEc.h"
#endif

typedef struct
{
    UINT32 *base_addr;
    INT32 offset;
    UINT32 size;
    UINT8 access_permission;
    UINT8 cacheable;
    UINT8 excute_disabled;
}mpu_setting_t;


#if defined(__CC_ARM)
__asm void __fast_memset(UINT32 *dst, UINT32 value, UINT32 length)
{
    push {r4-r9}
    cmp r2, #0
    beq memset_return
    mov r4, r1
    mov r5, r1
    mov r6, r1
    mov r7, r1
    and r8, r2,#0xf
    mov r9, r8
    cmp r8, #0
    beq clr_16Byte
clr_4Byte
    stmia r0!,{r4}
    subs r8,r8,#4
    cmp r8,#0
    bne clr_4Byte
    cmp r2, #0xf
    bls memset_return
    sub r2,r2,r9
clr_16Byte
    stmia r0!,{r4,r5,r6,r7}
    subs r2,r2,#16
    cmp r2,#0
    bne clr_16Byte
memset_return
    pop {r4-r9}
    bx lr
}
#elif defined(__GNUC__)
__attribute__((__noinline__)) void __fast_memset(UINT32 *dst, UINT32 value, UINT32 length)
{
    asm volatile(
    "cmp r2, #0\n\t"
    "beq memset_return\n\t"
    "mov r4, r1\n\t"
    "mov r5, r1\n\t"
    "mov r6, r1\n\t"
    "mov r7, r1\n\t"
    "and r8, r2,#0xf\n\t"
    "mov r9, r8\n\t"
    "cmp r8, #0\n\t"
    "beq clr_16Byte\n\t"
"clr_4Byte:\n\t"
    "stmia r0!,{r4}\n\t"
    "subs r8,r8,#4\n\t"
    "cmp r8,#0\n\t"
    "bne clr_4Byte\n\t"
    "cmp r2, #0xf\n\t"
    "bls memset_return\n\t"
    "sub r2,r2,r9\n\t"
"clr_16Byte:\n\t"
    "stmia r0!,{r4,r5,r6,r7}\n\t"
    "subs r2,r2,#16\n\t"
    "cmp r2,#0\n\t"
    "bne clr_16Byte\n\t"
"memset_return:\n\t"
    : /* no outputs. */
    : "r" (dst), "r" (value), "r" (length)
    : "cc", "r0", "r1", "r2", "r4", "r5", "r6", "r7", "r8", "r9"

    );
}
#endif

// Boot data/bss in MSMB
extern UINT32 Image$$LOAD_AP_BOOT_MSMB_BSS$$Base;
extern UINT32 Image$$LOAD_AP_BOOT_MSMB_BSS$$Limit;

extern UINT32 Image$$LOAD_AP_MSMB_BSS$$Base;
extern UINT32 Image$$LOAD_AP_MSMB_BSS$$Limit;

extern UINT32 Load$$LOAD_AP_MSMB_DATA$$Base;
extern UINT32 Image$$LOAD_AP_MSMB_DATA$$Base;
extern UINT32 Image$$LOAD_AP_MSMB_DATA$$Length;

extern UINT32 Load$$LOAD_AP_FPSRAM_P0_RAM_PLAT$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P0_RAM_PLAT$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P0_RAM_PLAT$$Length;

extern UINT32 Load$$LOAD_AP_FPSRAM_P0_RAM_PS$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P0_RAM_PS$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P0_RAM_PS$$Length;

extern UINT32 Load$$LOAD_AP_FPSRAM_P0_DATA$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P0_DATA$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P0_DATA$$Length;

extern UINT32 Image$$LOAD_AP_FPSRAM_P0_ZI$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P0_ZI$$Limit;

extern UINT32 Load$$LOAD_AP_FPSRAM_P1_DATA$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P1_DATA$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P1_DATA$$Length;

extern UINT32 Image$$LOAD_AP_FPSRAM_P1_ZI$$Base;
extern UINT32 Image$$LOAD_AP_FPSRAM_P1_ZI$$Limit;


extern UINT32 Load$$LOAD_AP_PSRAM_P2_RAM$$Base;
extern UINT32 Image$$LOAD_AP_PSRAM_P2_RAM$$Base;
extern UINT32 Image$$LOAD_AP_PSRAM_P2_RAM$$Length;

extern UINT32 Load$$LOAD_AP_PSRAM_P2_DATA$$Base;
extern UINT32 Image$$LOAD_AP_PSRAM_P2_DATA$$Base;
extern UINT32 Image$$LOAD_AP_PSRAM_P2_DATA$$Length;

extern UINT32 Image$$LOAD_AP_PSRAM_P2_ZI$$Base;
extern UINT32 Image$$LOAD_AP_PSRAM_P2_ZI$$Limit;

extern UINT32 Load$$LOAD_BOOTCODE$$Base    ;
extern UINT32 Image$$LOAD_BOOTCODE$$Base   ;
extern UINT32 Image$$LOAD_BOOTCODE$$Length ;

extern UINT32 Load$$LOAD_APOS$$Base    ;
extern UINT32 Image$$LOAD_APOS$$Base   ;
extern UINT32 Image$$LOAD_APOS$$Length ;

extern UINT32 Load$$LOAD_PPSRAM_P0_BSP_DATA$$Base  ;
extern UINT32 Image$$LOAD_PPSRAM_P0_BSP_DATA$$Base ;
extern UINT32 Image$$LOAD_PPSRAM_P0_BSP_DATA$$Length;
extern UINT32 Image$$LOAD_PPSRAM_P0_BSP$$ZI$$Base;
extern UINT32 Image$$LOAD_PPSRAM_P0_BSP$$ZI$$Limit;

extern UINT32 Load$$LOAD_AP_FIRAM_MSMB$$Base    ;
extern UINT32 Image$$LOAD_AP_FIRAM_MSMB$$Base   ;
extern UINT32 Image$$LOAD_AP_FIRAM_MSMB$$Length ;

extern UINT32 Load$$LOAD_AP_PIRAM_ASMB$$Base  ;
extern UINT32 Image$$LOAD_AP_PIRAM_ASMB$$Base ;
extern UINT32 Image$$LOAD_AP_PIRAM_ASMB$$Length   ;

extern UINT32 Load$$LOAD_AP_PIRAM_MSMB$$Base  ;
extern UINT32 Image$$LOAD_AP_PIRAM_MSMB$$Base ;
extern UINT32 Image$$LOAD_AP_PIRAM_MSMB$$Length   ;

extern UINT32 Load$$LOAD_AP_FIRAM_ASMB$$Base  ;
extern UINT32 Image$$LOAD_AP_FIRAM_ASMB$$Base ;
extern UINT32 Image$$LOAD_AP_FIRAM_ASMB$$Length   ;

extern UINT32 Load$$LOAD_AP_FDATA_ASMB$$RW$$Base;
extern UINT32 Image$$LOAD_AP_FDATA_ASMB$$RW$$Base;
extern UINT32 Image$$LOAD_AP_FDATA_ASMB$$Length;
extern UINT32 Image$$LOAD_AP_FDATA_ASMB$$ZI$$Base;
extern UINT32 Image$$LOAD_AP_FDATA_ASMB$$ZI$$Limit;

extern UINT32 Load$$LOAD_PS_FDATA_ASMB$$RW$$Base;
extern UINT32 Image$$LOAD_PS_FDATA_ASMB$$RW$$Base;
extern UINT32 Image$$LOAD_PS_FDATA_ASMB$$RW$$Length;
extern UINT32 Image$$LOAD_PS_FDATA_ASMB$$ZI$$Base;
extern UINT32 Image$$LOAD_PS_FDATA_ASMB$$ZI$$Limit;

extern UINT32 Load$$LOAD_AP_PIRAM_UNCOMP_MSMB$$Base;
extern UINT32 Image$$LOAD_AP_PIRAM_UNCOMP_MSMB$$Base;
extern UINT32 Image$$LOAD_AP_PIRAM_UNCOMP_MSMB$$Length;

#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
extern UINT32 Image$$SHARE_EXCEP_INFO$$ZI$$Base;
extern UINT32 Image$$SHARE_EXCEP_INFO$$ZI$$Limit;
#endif

extern UINT32 __StackTop;
extern UINT32 __StackLimit;



UINT32 Image$$ER_IROM1$$Base = 0;//temp define for usbc


const mpu_setting_t mpu_region[] =
{
    // base_addr                       offset  size                        access          cache   excute
    {0,                                0,        ARM_MPU_REGION_SIZE_4KB,    ARM_MPU_AP_RO,  1,    0},
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x0,      ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},   // cp region 0x8002000-0x8004000
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x2000,   ARM_MPU_REGION_SIZE_16KB,   ARM_MPU_AP_RO,  1,    0},   // cp region 0x8004000-0x8008000
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x6000,   ARM_MPU_REGION_SIZE_32KB,   ARM_MPU_AP_RO,  1,    0},   // cp region 0x8008000-0x8010000
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0xE000,   ARM_MPU_REGION_SIZE_64KB,   ARM_MPU_AP_RO,  1,    0},   // cp region 0x8010000-0x8020000

    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x1E000,  ARM_MPU_REGION_SIZE_64KB,   ARM_MPU_AP_RO,  1,    0},   // cp region 0x8020000-0x8030000
    #if (defined OPEN_CPU_MODE)
    #if (defined FEATURE_IMS_ENABLE)
    #if (defined FEATURE_IMS_SMS_ENABLE) && !defined(FEATURE_AUDIO_ENABLE)
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x2E000,  ARM_MPU_REGION_SIZE_16KB,  ARM_MPU_AP_RO,  1,    0},   // cp region 0x8030000-0x8034000
    #else //(defined FEATURE_IMS_SMS_ENABLE) && !defined(FEATURE_AUDIO_ENABLE)
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x2E000,  ARM_MPU_REGION_SIZE_64KB,  ARM_MPU_AP_RO,  1,    0},   // cp region 0x8030000-0x8040000
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x3E000,  ARM_MPU_REGION_SIZE_32KB,    ARM_MPU_AP_RO,  1,    0},   // cp region 0x8040000-0x8048000
    #endif
    #else //(defined FEATURE_IMS_ENABLE)
    #if defined(FEATURE_AUDIO_ENABLE)
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x2E000,  ARM_MPU_REGION_SIZE_64KB,  ARM_MPU_AP_RO,  1,    0},   // cp region 0x8030000-0x8040000
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x3E000,  ARM_MPU_REGION_SIZE_32KB,    ARM_MPU_AP_RO,  1,    0},   // cp region 0x8040000-0x8048000
    #else //defined(FEATURE_AUDIO_ENABLE)
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x2E000,  ARM_MPU_REGION_SIZE_16KB,  ARM_MPU_AP_RO,  1,    0},   // cp region 0x8030000-0x8034000
    #endif
    #endif
    #else // (defined OPEN_CPU_MODE)
    {(UINT32  *)CP_PSRAM_P0_START_ADDR,0x2E000,  ARM_MPU_REGION_SIZE_16KB,  ARM_MPU_AP_RO,  1,    0},   // cp region 0x8030000-0x8034000
    #endif

};

#pragma GCC push_options
#pragma GCC optimize("O1")
void bootDataBssInMsmbInit()
{
    UINT32 length;
    UINT32 *start_addr;
    UINT32 *end_addr  ;

    start_addr = &(Image$$LOAD_AP_BOOT_MSMB_BSS$$Base) ;
    end_addr   = &(Image$$LOAD_AP_BOOT_MSMB_BSS$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);
}

void psramP1RamcodeDataBssInit(void)
{
#ifndef RAMCODE_COMPRESS_EN
    UINT32 *src;
    UINT32 *dst;
#endif
    UINT32 length;
    UINT32 *start_addr;
    UINT32 *end_addr  ;

#ifndef RAMCODE_COMPRESS_EN
    // psram p1 data
    dst    = &(Image$$LOAD_AP_FPSRAM_P1_DATA$$Base);
    src    = &(Load$$LOAD_AP_FPSRAM_P1_DATA$$Base);
    length = (UINT32)&(Image$$LOAD_AP_FPSRAM_P1_DATA$$Length);
    length /= sizeof(UINT32);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }
#else
    decompressRamCodeFromBin(SECTIONAP_LOAD_AP_FPSRAMP1_DATA);
#endif

    // psram p1 bss 
    start_addr = &(Image$$LOAD_AP_FPSRAM_P1_ZI$$Base) ;
    end_addr   = &(Image$$LOAD_AP_FPSRAM_P1_ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);

}


void psramP2RamcodeDataBssInit(void)
{
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;
    UINT32 *start_addr;
    UINT32 *end_addr  ;

    // psram p2 ramcode init 
    dst    = &(Image$$LOAD_AP_PSRAM_P2_RAM$$Base);
    src    = &(Load$$LOAD_AP_PSRAM_P2_RAM$$Base);
    length = (UINT32)&(Image$$LOAD_AP_PSRAM_P2_RAM$$Length);
    length /= sizeof(UINT32);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }

    // psram p2 data
    dst    = &(Image$$LOAD_AP_PSRAM_P2_DATA$$Base);
    src    = &(Load$$LOAD_AP_PSRAM_P2_DATA$$Base);
    length = (UINT32)&(Image$$LOAD_AP_PSRAM_P2_DATA$$Length);
    length /= sizeof(UINT32);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }

    // psram p2 bss 
    start_addr = &(Image$$LOAD_AP_PSRAM_P2_ZI$$Base) ;
    end_addr   = &(Image$$LOAD_AP_PSRAM_P2_ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);

    DisableICache();                    // flush cache when ramcode update
    EnableICache();

}


void psFDataInAsmbInit(void)
{
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;
    UINT32 *start_addr;
    UINT32 *end_addr  ;

    dst    = &(Image$$LOAD_PS_FDATA_ASMB$$RW$$Base);
    src    = &(Load$$LOAD_PS_FDATA_ASMB$$RW$$Base);
    length = (UINT32)&(Image$$LOAD_PS_FDATA_ASMB$$RW$$Length);
    length /= sizeof(UINT32);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }

    start_addr = &(Image$$LOAD_PS_FDATA_ASMB$$ZI$$Base) ;
    end_addr   = &(Image$$LOAD_PS_FDATA_ASMB$$ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);

}


void platFDataInAsmbInit(void)
{
    UINT32 *start_addr;
    UINT32 *end_addr  ;
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;

    dst    = &(Image$$LOAD_AP_FDATA_ASMB$$RW$$Base);
    src    = &(Load$$LOAD_AP_FDATA_ASMB$$RW$$Base);
    length = (UINT32)&(Image$$LOAD_AP_FDATA_ASMB$$Length);
    length /= sizeof(UINT32);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }

    start_addr = &(Image$$LOAD_AP_FDATA_ASMB$$ZI$$Base) ;
    end_addr   = &(Image$$LOAD_AP_FDATA_ASMB$$ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);

}


void SetZIDataToZero(void)
{
    UINT32 *start_addr;
    UINT32 *end_addr  ;
    UINT32 length;

    start_addr = &(Image$$LOAD_AP_FPSRAM_P0_ZI$$Base) ;
    end_addr   = &(Image$$LOAD_AP_FPSRAM_P0_ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);

    start_addr = &(Image$$LOAD_AP_MSMB_BSS$$Base) ;
    end_addr   = &(Image$$LOAD_AP_MSMB_BSS$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);
}

void CopyRWDataFromBin(void)
{
#ifndef RAMCODE_COMPRESS_EN
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;
#endif
    APBootFlag_e apBootFlag = apmuGetAPBootFlag();

    switch(apBootFlag)
    {
        case AP_BOOT_FROM_POWER_ON:
        case AP_BOOT_FROM_AO:
        case AP_BOOT_FROM_AH:
        {
            #ifndef RAMCODE_COMPRESS_EN
            dst    = &(Image$$LOAD_AP_FIRAM_ASMB$$Base);
            src    = &(Load$$LOAD_AP_FIRAM_ASMB$$Base);
            length = (unsigned int)&(Image$$LOAD_AP_FIRAM_ASMB$$Length);
            length /= sizeof(unsigned int);

            if(dst != src)
            {
                while(length >0)
                {
                    dst[length-1]=src[length-1];
                    length--;
                }
            }
            #else
            decompressRamCodeFromBin(SECTIONAP_LOAD_AP_FIRAM_ASMB);
            #endif
        }
        case AP_BOOT_FROM_AS2:
        {
            #ifndef RAMCODE_COMPRESS_EN
            dst    = &(Image$$LOAD_AP_FIRAM_MSMB$$Base);
            src    = &(Load$$LOAD_AP_FIRAM_MSMB$$Base);
            length = (unsigned int)&(Image$$LOAD_AP_FIRAM_MSMB$$Length);
            length /= sizeof(unsigned int);

            if(dst != src)
            {
                while(length >0)
                {
                    dst[length-1]=src[length-1];
                    length--;
                }
            }
            #else
            decompressRamCodeFromBin(SECTIONAP_LOAD_AP_FIRAM_MSMB);
            #endif

            #ifndef RAMCODE_COMPRESS_EN
            dst    = &(Image$$LOAD_APOS$$Base);
            src    = &(Load$$LOAD_APOS$$Base);
            length = (unsigned int)&(Image$$LOAD_APOS$$Length);
            length /= sizeof(unsigned int);

            if(dst != src)
            {
                while(length >0)
                {
                    dst[length-1]=src[length-1];
                    length--;
                }
            }
            #else
            decompressRamCodeFromBin(SECTIONAP_LOAD_APOS);
            #endif

            #ifndef RAMCODE_COMPRESS_EN
            // part of platFMRamcode_test moved to load_ap_psram_p0_ram_plat 
            dst    = &(Image$$LOAD_AP_FPSRAM_P0_RAM_PLAT$$Base);
            src    = &(Load$$LOAD_AP_FPSRAM_P0_RAM_PLAT$$Base);
            length = (UINT32)&(Image$$LOAD_AP_FPSRAM_P0_RAM_PLAT$$Length);
            length /= sizeof(UINT32);
            
            if(dst != src)
            {
                while(length >0)
                {
                    dst[length-1]=src[length-1];
                    length--;
                }
            }
            #else
            decompressRamCodeFromBin(SECTIONAP_LOAD_AP_FPSRAMP0_RAMCODE_PLAT);
            #endif
        }
        break;

        default:
            EC_ASSERT(0, apBootFlag, 0, 0);
    }

    DisableICache();                    // flush cache when ramcode update
    EnableICache();

    #ifndef RAMCODE_COMPRESS_EN
        // psram p0 data
        dst    = &(Image$$LOAD_AP_FPSRAM_P0_DATA$$Base);
        src    = &(Load$$LOAD_AP_FPSRAM_P0_DATA$$Base);
        length = (UINT32)&(Image$$LOAD_AP_FPSRAM_P0_DATA$$Length);
        length /= sizeof(UINT32);
    
        if(dst != src)
        {
            while(length >0)
            {
                dst[length-1]=src[length-1];
                length--;
            }
        }
    #else
    decompressRamCodeFromBin(SECTIONAP_LOAD_AP_FPSRAMP0_DATA);
    #endif

    #ifndef RAMCODE_COMPRESS_EN
    // MSMB data
    dst    = &(Image$$LOAD_AP_MSMB_DATA$$Base);
    src    = &(Load$$LOAD_AP_MSMB_DATA$$Base);
    length = (UINT32)&(Image$$LOAD_AP_MSMB_DATA$$Length);
    length /= sizeof(UINT32);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }
    #else
    decompressRamCodeFromBin(SECTIONAP_LOAD_AP_DATA_MSMB);
    #endif

    psramP1RamcodeDataBssInit();

    DisableICache();                    // flush cache when ramcode update
    EnableICache();

}


void CopyRamCodeForDeepSleep(void)
{
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;
    APBootFlag_e apBootFlag = apmuGetAPLLBootFlag();

    switch(apBootFlag)
    {
        case AP_BOOT_FROM_POWER_ON:
        case AP_BOOT_FROM_AO:
        case AP_BOOT_FROM_AH:
        {
            #ifndef RAMCODE_COMPRESS_EN
            dst    = &(Image$$LOAD_BOOTCODE$$Base);
            src    = &(Load$$LOAD_BOOTCODE$$Base);
            length = (unsigned int)&(Image$$LOAD_BOOTCODE$$Length);
            length /= sizeof(unsigned int);

            if(dst != src)
            {
                while(length >0)
                {
                    dst[length-1]=src[length-1];
                    length--;
                }
            }
            #else
            decompressRamCodeFromBin(SECTIONAP_LOAD_BOOTCODE);
            #endif

            #ifndef RAMCODE_COMPRESS_EN
            dst    = &(Image$$LOAD_AP_PIRAM_ASMB$$Base);
            src    = &(Load$$LOAD_AP_PIRAM_ASMB$$Base);
            length = (unsigned int)&(Image$$LOAD_AP_PIRAM_ASMB$$Length);
            length /= sizeof(unsigned int);

            if(dst != src)
            {
                while(length >0)
                {
                    dst[length-1]=src[length-1];
                    length--;
                }
            }
            #else
            decompressRamCodeFromBin(SECTIONAP_LOAD_AP_PIRAM_ASMB);
            #endif
        }
        case AP_BOOT_FROM_AS2:
        {
            dst    = &(Image$$LOAD_AP_PIRAM_UNCOMP_MSMB$$Base);
            src    = &(Load$$LOAD_AP_PIRAM_UNCOMP_MSMB$$Base);
            length = (unsigned int)&(Image$$LOAD_AP_PIRAM_UNCOMP_MSMB$$Length);
            length /= sizeof(unsigned int);

            if(dst != src)
            {
                while(length >0)
                {
                    dst[length-1]=src[length-1];
                    length--;
                }
            }

            #ifndef RAMCODE_COMPRESS_EN
            dst    = &(Image$$LOAD_AP_PIRAM_MSMB$$Base);
            src    = &(Load$$LOAD_AP_PIRAM_MSMB$$Base);
            length = (unsigned int)&(Image$$LOAD_AP_PIRAM_MSMB$$Length);
            length /= sizeof(unsigned int);

            if(dst != src)
            {
                while(length >0)
                {
                    dst[length-1]=src[length-1];
                    length--;
                }
            }
            #else
            decompressRamCodeFromBin(SECTIONAP_LOAD_AP_PIRAM_MSMB);
            #endif
        }
        break;

        default:
            EC_ASSERT(0, apBootFlag, 0, 0);
    }

    DisableICache();        // flush cache when ramcode update
    EnableICache();

}


void CopyDataRWZIForDeepSleep(void)
{
    UINT32 *start_addr;
    UINT32 *end_addr  ;
    UINT32 length;
    #ifndef RAMCODE_COMPRESS_EN
    UINT32 *src;
    UINT32 *dst;

    dst    = &(Image$$LOAD_PPSRAM_P0_BSP_DATA$$Base);
    src    = &(Load$$LOAD_PPSRAM_P0_BSP_DATA$$Base);
    length = (UINT32)&(Image$$LOAD_PPSRAM_P0_BSP_DATA$$Length);
    length /= sizeof(UINT32);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }
    #else
    decompressRamCodeFromBin(SECTIONAP_LOAD_PPSRAM_P0_BSP_DATA);
    #endif

    start_addr = &(Image$$LOAD_PPSRAM_P0_BSP$$ZI$$Base) ;
    end_addr   = &(Image$$LOAD_PPSRAM_P0_BSP$$ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);

#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
    start_addr = &(Image$$SHARE_EXCEP_INFO$$ZI$$Base) ;
    end_addr   = &(Image$$SHARE_EXCEP_INFO$$ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);
#endif

}

#pragma GCC pop_options

void mpu_init(void)
{
    int i=0;
    uint8_t region_num = 0;

    if(MPU->TYPE==0)
        return;

    ARM_MPU_Disable();

    for(i=0;i<8;i++)
        ARM_MPU_ClrRegion(i);

    region_num = sizeof(mpu_region)/sizeof(mpu_setting_t);

    for(i=0;i<region_num;i++)
    {
        ARM_MPU_SetRegionEx(i,((uint32_t)mpu_region[i].base_addr)+mpu_region[i].offset,ARM_MPU_RASR(mpu_region[i].excute_disabled,mpu_region[i].access_permission,0,0,mpu_region[i].cacheable,1,0,mpu_region[i].size));
    }

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

}


void mpu_deinit()
{
    int i=0;

    if(MPU->TYPE==0)
        return;

    ARM_MPU_Disable();

    for(i=0;i<8;i++)
        ARM_MPU_ClrRegion(i);

}




