/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History: initiated by xuwang
*
*  Notes:
*
******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include "fota_pub.h"
#include "fota_sal.h"
#include "fota_mem.h"
#include "sctdef.h"
#include "cmsis_os2.h"
//uint32_t __heap_base  = 0;
//uint32_t __heap_limit = 0;

//extern uint32_t Image$$LOAD_DRAM_FOTA$$ZI$$Base;     /* Length of LOAD_DRAM_FOTA region */
//extern uint32_t Image$$LOAD_DRAM_FOTA$$ZI$$Length ;  /* Length of LOAD_DRAM_FOTA region */

#define FOTA_BLOCK_HEAD_MAGIC         0xbeafdead
#define FOTA_BLOCK_TAIL_MAGIC         0xdeadbeaf

#define FOTA_BLOCK_TOTAL_OVERHEAD     (FOTA_BLOCK_HEAD_OVERHEAD + FOTA_BLOCK_TAIL_OVERHEAD)
#define FOTA_BLOCK_HEAD_OVERHEAD      sizeof(FotaBlockHead_t)
#define FOTA_BLOCK_TAIL_OVERHEAD      sizeof(FotaBlockTail_t)

typedef struct
{
    uint8_t  magic[8];
    uint32_t size;
}FotaBlockHead_t;

typedef struct
{
    uint8_t  magic[8];
}FotaBlockTail_t;


/******************************************************************************
 * @brief : fotaIsSuppDynMem
 * @author: Xu.Wang
 * @note  : is dynamic memory scheme supported or not?
 ******************************************************************************/
uint8_t fotaIsSuppDynMem(void)
{
#if (FOTA_PRESET_RAM_ENABLE == 1)
    return 0;
#else //other chips
    return 1;
#endif
}


/******************************************************************************
 * @brief : fotaInitMem
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT int32_t fotaInitMem(uint8_t *buf, uint32_t size)
{
//    __heap_base  = (uint32_t)(&Image$$LOAD_DRAM_FOTA$$ZI$$Base);
//    __heap_limit = (uint32_t)(&Image$$LOAD_DRAM_FOTA$$ZI$$Length);

    return 0;
}

/******************************************************************************
 * @brief : fotaAllocMem
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT void* fotaAllocMem(uint32_t size)
{
    uint32_t       adjust = FOTA_ALIGN_UP(size,4) + FOTA_BLOCK_TOTAL_OVERHEAD;
    uint8_t          *ptr = NULL;
    void            *body = NULL;
    FotaBlockHead_t *head = NULL;
    FotaBlockTail_t *tail = NULL;

    if(fotaIsSuppDynMem())
    {
        if((ptr = mallocEc(adjust)) == NULL) return NULL;

        body = (void*)(ptr + FOTA_BLOCK_HEAD_OVERHEAD);
        head = (FotaBlockHead_t*)ptr;
        tail = (FotaBlockTail_t*)(ptr + FOTA_BLOCK_HEAD_OVERHEAD + FOTA_ALIGN_UP(size,4));

        head->size = size;
        *(uint32_t*)(&head->magic[0]) = FOTA_BLOCK_HEAD_MAGIC;
        *(uint32_t*)(&head->magic[4]) = FOTA_BLOCK_HEAD_MAGIC;
        *(uint32_t*)(&tail->magic[0]) = FOTA_BLOCK_TAIL_MAGIC;
        *(uint32_t*)(&tail->magic[4]) = FOTA_BLOCK_TAIL_MAGIC;

        //ECPLAT_PRINTF(UNILOG_FOTA, FOTA_MALLOC, P_INFO, "malloc(%d) @0x%x\n", size, body);
    }

    return body;
}

/******************************************************************************
 * @brief : fotaFreeMem
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT void fotaFreeMem(void *buf)
{
    FotaBlockHead_t *head = NULL;
    FotaBlockTail_t *tail = NULL;

    if(fotaIsSuppDynMem() && buf)
    {
        head = (FotaBlockHead_t*)((uint8_t*)buf - FOTA_BLOCK_HEAD_OVERHEAD);
        if(*(uint32_t*)(&head->magic[0]) != FOTA_BLOCK_HEAD_MAGIC || \
           *(uint32_t*)(&head->magic[4]) != FOTA_BLOCK_HEAD_MAGIC)
        {
            ECPLAT_PRINTF(UNILOG_FOTA, FOTA_FREE_1, P_ERROR, "buf(0x%x) head bound chk err!\n", buf);
            return;
        }

        tail = (FotaBlockTail_t*)((uint8_t*)buf + FOTA_ALIGN_UP(head->size,4));
        if(*(uint32_t*)(&tail->magic[0]) != FOTA_BLOCK_TAIL_MAGIC || \
           *(uint32_t*)(&tail->magic[4]) != FOTA_BLOCK_TAIL_MAGIC)
        {
            ECPLAT_PRINTF(UNILOG_FOTA, FOTA_FREE_2, P_ERROR, "buf(0x%x) tail bound chk err!\n", buf);
            return;
        }

       //ECPLAT_PRINTF(UNILOG_FOTA, FOTA_FREE_3, P_INFO, "free(%d) @addr(0x%x)\n", head->size, buf);

       if(head) freeEc((void*)head);
    }
}


