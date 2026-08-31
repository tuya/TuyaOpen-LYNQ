#include <string.h>
#include "ec7xx.h"
#include "mem_map.h"
#ifdef RAMCODE_COMPRESS_EN
#include "LzmaEc.h"
#endif
#include "sctdef.h"


typedef unsigned long UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;


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



extern UINT32 Load$$AP_IRAM_XIP_START$$Base ;
extern UINT32 Image$$AP_IRAM_XIP_START$$Base    ;
extern UINT32 Image$$AP_IRAM_XIP_START$$Length  ;

extern UINT32 Load$$LOAD_AIRAM_PRE1$$Base   ;
extern UINT32 Image$$LOAD_AIRAM_PRE1$$Base  ;
extern UINT32 Image$$LOAD_AIRAM_PRE1$$Length    ;

extern UINT32 Load$$LOAD_AIRAM_PRE2$$Base   ;
extern UINT32 Image$$LOAD_AIRAM_PRE2$$Base  ;
extern UINT32 Image$$LOAD_AIRAM_PRE2$$Length    ;

extern UINT32 Load$$LOAD_AIRAM_FLASH$$Base  ;
extern UINT32 Image$$LOAD_AIRAM_FLASH$$Base ;
extern UINT32 Image$$LOAD_AIRAM_FLASH$$Length;

extern UINT32 Load$$LOAD_AIRAM_SHARED_DATA$$Base    ;
extern UINT32 Image$$LOAD_AIRAM_SHARED_DATA$$Base;
extern UINT32 Image$$LOAD_AIRAM_SHARED_DATA$$Length;
extern UINT32 Image$$LOAD_MIRAM_SHARED$$ZI$$Base;
extern UINT32 Image$$LOAD_MIRAM_SHARED$$ZI$$Limit;

extern UINT32 Load$$LOAD_AIRAM_OTHER_RAMCODE$$Base;
extern UINT32 Image$$LOAD_AIRAM_OTHER_RAMCODE$$Base;
extern UINT32 Image$$LOAD_AIRAM_OTHER_RAMCODE$$Length;

extern UINT32 Load$$LOAD_CIRAM_RAMCODE$$Base;
extern UINT32 Image$$LOAD_CIRAM_RAMCODE$$Base;
extern UINT32 Image$$LOAD_CIRAM_RAMCODE$$Length;


extern UINT32 __StackTop;
extern UINT32 __StackLimit;

extern void DisableICache(void);
extern void EnableICache(void);

#pragma GCC push_options
#pragma GCC optimize("O1")
PLAT_BL_UNCOMP_FLASH_TEXT void SetZIDataToZero(void)
{
    UINT32 *start_addr;
    UINT32 *end_addr  ;
    UINT32 length;
    UINT32 stack_len = ((UINT32)&__StackTop - (UINT32)&__StackLimit);
    start_addr = &(Image$$LOAD_MIRAM_SHARED$$ZI$$Base) ;
    end_addr   = &(Image$$LOAD_MIRAM_SHARED$$ZI$$Limit);
    length = (UINT32)end_addr - (UINT32)start_addr;
    __fast_memset((UINT32 *)start_addr, 0, length-stack_len);
}


PLAT_BL_UNCOMP_FLASH_TEXT void CopyXipStartRwToImage(void)
{
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;
    dst    = &(Image$$AP_IRAM_XIP_START$$Base);
    src    = &(Load$$AP_IRAM_XIP_START$$Base);
    length = (unsigned int)&(Image$$AP_IRAM_XIP_START$$Length);
    length /= sizeof(unsigned int);
    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }
}


PLAT_BL_UNCOMP_FLASH_TEXT void CopyPre1RamtoImage(void)
{
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;
    dst    = &(Image$$LOAD_AIRAM_PRE1$$Base);
    src    = &(Load$$LOAD_AIRAM_PRE1$$Base);
    length = (unsigned int)&(Image$$LOAD_AIRAM_PRE1$$Length);
    length /= sizeof(unsigned int);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }

    DisableICache();
    EnableICache();
}

PLAT_BL_UNCOMP_FLASH_TEXT void CopyPre2RamtoImage(void)
{
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;
    dst    = &(Image$$LOAD_AIRAM_PRE2$$Base);
    src    = &(Load$$LOAD_AIRAM_PRE2$$Base);
    length = (unsigned int)&(Image$$LOAD_AIRAM_PRE2$$Length);
    length /= sizeof(unsigned int);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }
    DisableICache();
    EnableICache();
}

PLAT_BL_UNCOMP_FLASH_TEXT void CopyOtherCodetoImage(void)
{
    #ifndef RAMCODE_COMPRESS_EN
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;
    dst    = &(Image$$LOAD_AIRAM_OTHER_RAMCODE$$Base);
    src    = &(Load$$LOAD_AIRAM_OTHER_RAMCODE$$Base);
    length = (unsigned int)&(Image$$LOAD_AIRAM_OTHER_RAMCODE$$Length);
    length /= sizeof(unsigned int);

    if(dst != src)
    {
        while(length >0)
        {
            dst[length-1]=src[length-1];
            length--;
        }
    }

    dst    = &(Image$$LOAD_CIRAM_RAMCODE$$Base);
    src    = &(Load$$LOAD_CIRAM_RAMCODE$$Base);
    length = (unsigned int)&(Image$$LOAD_CIRAM_RAMCODE$$Length);
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
    decompressRamCodeFromBin(SECTIONBL_LOAD_AIRAM_OTHER_RAMCODE);
    decompressRamCodeFromBin(SECTIONBL_LOAD_CIRAM_RAMCODE);
    #endif

    DisableICache();
    EnableICache();
}

PLAT_BL_UNCOMP_FLASH_TEXT void CopyRWDataFromBin(void)
{
    #ifndef RAMCODE_COMPRESS_EN
    UINT32 *src;
    UINT32 *dst;
    UINT32 length;
    dst    = &(Image$$LOAD_AIRAM_SHARED_DATA$$Base);
    src    = &(Load$$LOAD_AIRAM_SHARED_DATA$$Base);
    length = (unsigned int)&(Image$$LOAD_AIRAM_SHARED_DATA$$Length);
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
    decompressRamCodeFromBin(SECTIONBL_LOAD_AIRAM_SHARED);
    #endif
    DisableICache();
    EnableICache();
}


#pragma GCC pop_options



