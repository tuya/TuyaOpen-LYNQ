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



extern UINT32 Load$$LOAD_BOOTCODE$$Base    ;
extern UINT32 Image$$LOAD_BOOTCODE$$Base   ;
extern UINT32 Image$$LOAD_BOOTCODE$$Length ;

extern UINT32 Load$$LOAD_APOS$$Base    ;
extern UINT32 Image$$LOAD_APOS$$Base   ;
extern UINT32 Image$$LOAD_APOS$$Length ;

extern UINT32 Load$$LOAD_DRAM_SHARED$$Base  ;
extern UINT32 Image$$LOAD_DRAM_SHARED$$Base ;
extern UINT32 Image$$LOAD_DRAM_SHARED$$Length   ;
extern UINT32 Image$$LOAD_DRAM_SHARED$$ZI$$Base;
extern UINT32 Image$$LOAD_DRAM_SHARED$$ZI$$Limit;

extern UINT32 Load$$LOAD_DRAM_BSP$$Base  ;
extern UINT32 Image$$LOAD_DRAM_BSP$$Base ;
extern UINT32 Image$$LOAD_DRAM_BSP$$Length   ;
extern UINT32 Image$$LOAD_DRAM_BSP$$ZI$$Base;
extern UINT32 Image$$LOAD_DRAM_BSP$$ZI$$Limit;

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

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
extern UINT32 Load$$LOAD_AP_FIRAM_PSRAM$$Base;
extern UINT32 Image$$LOAD_AP_FIRAM_PSRAM$$Base;
extern UINT32 Image$$LOAD_AP_FIRAM_PSRAM$$Length;

extern UINT32 Load$$LOAD_PSRAM_SHARED$$Base;
extern UINT32 Image$$LOAD_PSRAM_SHARED$$Base;
extern UINT32 Image$$LOAD_PSRAM_SHARED$$Length;
extern UINT32 Image$$LOAD_PSRAM_SHARED$$ZI$$Base;
extern UINT32 Image$$LOAD_PSRAM_SHARED$$ZI$$Limit;

/* PSRAM LD variables for customer */
extern UINT32 Load$$LOAD_AP_FIRAM_PSRAM_CUST$$Base;
extern UINT32 Image$$LOAD_AP_FIRAM_PSRAM_CUST$$Base;
extern UINT32 Image$$LOAD_AP_FIRAM_PSRAM_CUST$$Length;
extern UINT32 Load$$LOAD_PSRAM_SHARED_CUST$$Base;
extern UINT32 Image$$LOAD_PSRAM_SHARED_CUST$$Base;
extern UINT32 Image$$LOAD_PSRAM_SHARED_CUST$$Length;
extern UINT32 Image$$LOAD_PSRAM_SHARED_CUST$$ZI$$Base;
extern UINT32 Image$$LOAD_PSRAM_SHARED_CUST$$ZI$$Limit;

#endif

UINT32 Image$$ER_IROM1$$Base = 0;//temp define for usbc


const mpu_setting_t mpu_region[] =
{
#if (defined CHIP_EC718)
    #ifdef FEATURE_AUDIO_ENABLE//volte
        // base_addr                    offset  size                        access          cache   excute
        {0,                              0,        ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},
        {(UINT32 *)MSMB_APMEM_END_ADDR,  0,        ARM_MPU_REGION_SIZE_4KB,    ARM_MPU_AP_RO,  1,    0},  // cp region: 0x4F1000-0x4F2000
        {(UINT32 *)MSMB_APMEM_END_ADDR,  0x1000,   ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},  // cp region: 0x4F2000-0x4F4000
        {(UINT32 *)MSMB_APMEM_END_ADDR,  0x3000,   ARM_MPU_REGION_SIZE_16KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x4F4000-0x4F8000
		{(UINT32 *)MSMB_APMEM_END_ADDR,  0x7000,   ARM_MPU_REGION_SIZE_32KB,   ARM_MPU_AP_RO,  1,	 0},  // cp region: 0x4F8000-0x500000
		{(UINT32 *)MSMB_APMEM_END_ADDR,  0xF000,   ARM_MPU_REGION_SIZE_128KB,  ARM_MPU_AP_RO,  1,    0},  // cp region: 0x500000-0x520000
        {(UINT32 *)MSMB_APMEM_END_ADDR,  0x2F000,  ARM_MPU_REGION_SIZE_64KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x520000-0x530000
		{(UINT32 *)MSMB_APMEM_END_ADDR,  0x3F000,  ARM_MPU_REGION_SIZE_32KB,   ARM_MPU_AP_RO,  1,	 0},  // cp region: 0x530000-0x538000
	#else
    // base_addr                    offset  size                        access          cache   excute
    {0,                              0,        ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0,        ARM_MPU_REGION_SIZE_128KB,  ARM_MPU_AP_RO,  1,    0},  // cp region: 0x500000-0x520000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x20000,  ARM_MPU_REGION_SIZE_64KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x520000-0x530000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x30000,  ARM_MPU_REGION_SIZE_32KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x530000-0x538000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x38000,  ARM_MPU_REGION_SIZE_16KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x538000-0x53c000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x3C000,  ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},  // cp region: 0x53c000-0x53e000
    #endif
#endif

#if (defined CHIP_EC716)

    #ifdef FEATURE_MORERAM_ENABLE
    // base_addr                    offset  size                        access          cache   excute
    {0,                              0,        ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0,        ARM_MPU_REGION_SIZE_4KB,    ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004DD000-0x004DE000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x1000,   ARM_MPU_REGION_SIZE_8KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004DE000-0x004E0000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x3000,   ARM_MPU_REGION_SIZE_64KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004E0000-0x004F0000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x13000,  ARM_MPU_REGION_SIZE_32KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004F0000-0x004F8000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x1B000,  ARM_MPU_REGION_SIZE_16KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004F8000-0x004FC000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x1F000,  ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004FC000-0x004FE000
    #else
    // base_addr                    offset  size                        access          cache   excute
    {0,                              0,        ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0,        ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004CA000-0x004CC000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x2000,   ARM_MPU_REGION_SIZE_16KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004CC000-0x004D0000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x6000,   ARM_MPU_REGION_SIZE_64KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004D0000-0x004E0000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x16000,  ARM_MPU_REGION_SIZE_64KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004E0000-0x004F0000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x26000,  ARM_MPU_REGION_SIZE_32KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004F0000-0x004F8000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x2E000,  ARM_MPU_REGION_SIZE_16KB,   ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004F8000-0x004FC000
    {(UINT32 *)MSMB_APMEM_END_ADDR,  0x32000,  ARM_MPU_REGION_SIZE_8KB,    ARM_MPU_AP_RO,  1,    0},  // cp region: 0x004FC000-0x004FE000
    #endif
#endif
};

#pragma GCC push_options
#pragma GCC optimize("O1")
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
    UINT32 stack_len = ((UINT32)&__StackTop - (UINT32)&__StackLimit);

    start_addr = &(Image$$LOAD_DRAM_SHARED$$ZI$$Base) ;
    end_addr   = &(Image$$LOAD_DRAM_SHARED$$ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length-stack_len);
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
        }
        break;

        default:
            EC_ASSERT(0, apBootFlag, 0, 0);
    }

    DisableICache();                    // flush cache when ramcode update
    EnableICache();

    #ifndef RAMCODE_COMPRESS_EN
    dst    = &(Image$$LOAD_DRAM_SHARED$$Base);
    src    = &(Load$$LOAD_DRAM_SHARED$$Base);
    length = (UINT32)&(Image$$LOAD_DRAM_SHARED$$Length);
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
    decompressRamCodeFromBin(SECTIONAP_LOAD_DRAM_SHARED);
    #endif

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

    dst    = &(Image$$LOAD_DRAM_BSP$$Base);
    src    = &(Load$$LOAD_DRAM_BSP$$Base);
    length = (UINT32)&(Image$$LOAD_DRAM_BSP$$Length);
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
    decompressRamCodeFromBin(SECTIONAP_LOAD_DRAM_BSP);
    #endif

    start_addr = &(Image$$LOAD_DRAM_BSP$$ZI$$Base) ;
    end_addr   = &(Image$$LOAD_DRAM_BSP$$ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);

#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
	start_addr = &(Image$$SHARE_EXCEP_INFO$$ZI$$Base) ;
	end_addr   = &(Image$$SHARE_EXCEP_INFO$$ZI$$Limit);
	length = (UINT32)end_addr - (UINT32)start_addr;
	__fast_memset((UINT32 *)start_addr, 0, length);
#endif

}


#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
void CopyPsramCodeAndData(void)
{
    UINT32 *start_addr;
    UINT32 *end_addr  ;
    UINT32 length;
	// now psram code prepare in bsp_customInit, decompress code should re-write and use user defined buffer
    #if 1 //#ifndef RAMCODE_COMPRESS_EN
    UINT32 *src;
    UINT32 *dst;

    dst    = &(Image$$LOAD_AP_FIRAM_PSRAM$$Base);
    src    = &(Load$$LOAD_AP_FIRAM_PSRAM$$Base);
    length = (unsigned int)&(Image$$LOAD_AP_FIRAM_PSRAM$$Length);
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
    decompressRamCodeFromBin(SECTIONAP_LOAD_AP_FIRAM_PSRAM);
    #endif

    #if 1 //#ifndef RAMCODE_COMPRESS_EN
    dst    = &(Image$$LOAD_PSRAM_SHARED$$Base);
    src    = &(Load$$LOAD_PSRAM_SHARED$$Base);
    length = (UINT32)&(Image$$LOAD_PSRAM_SHARED$$Length);
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
    decompressRamCodeFromBin(SECTIONAP_LOAD_PSRAM_SHARED);
    #endif

    start_addr = &(Image$$LOAD_PSRAM_SHARED$$ZI$$Base) ;
    end_addr   = &(Image$$LOAD_PSRAM_SHARED$$ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length);


    DisableICache();        // flush cache when ramcode update
    EnableICache();


}
#endif

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




