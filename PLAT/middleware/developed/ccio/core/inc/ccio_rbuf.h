/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_rbuf.h
*
*  Description:
*
*  History: 2021/1/19 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef CCIO_RBUF_H
#define CCIO_RBUF_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "ccio_base.h"


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
/* definition of 'bmPtMode' field
 * bit0: PT(pass through) mode or not?
 * bit1: full PT/NPT mode or not?
 */
#define CCIO_RBUF_HALF_NPT_MODE     0x0
#define CCIO_RBUF_FULL_NPT_MODE     0x2
#define CCIO_RBUF_HALF_PT_MODE      0x1
#define CCIO_RBUF_FULL_PT_MODE      0x3

#define CCIO_RBUF_PT_MODE           0x1

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef uint8_t  (*rbufIsRxWinEnFunc)(uint8_t chanNo);
typedef uint32_t (*rbufGetRxWinFunc)(uint8_t chanNo);
typedef uint32_t (*rbufChgRxWinFunc)(uint8_t chanNo, uint32_t size);
typedef int32_t  (*rbufDecapPduFunc)(void **ulpdu, uint16_t *frameLen, void *extras);
typedef int32_t  (*rbufCheckObzpFunc)(void *ulpdu, uint16_t dataLen);

typedef struct
{
    uint32_t  bmPtMode  :2; /* bit1: full PT/NPT or not?, bit0: PT(pass through) or not? */
    uint32_t  alignVal  :3;
    uint32_t  cctSize   :14;
    uint32_t  rsvdBits  :13;
}CcioRbufConfig_t;

/**
 * Structure which holds the supplemental info for a ring buffer.
 */
typedef struct
{
    uint32_t  isRecvNow :1;
    uint32_t  isPreGet  :1;
    uint32_t  xtraSize  :6;
    uint32_t  ahcbSize  :6;  /* Additional-Header-Control-Block */
    uint32_t  isThresEn :1;
    uint32_t  avlbThres :14;
    uint32_t  rsvdBits  :3;
    CcioRbufConfig_t  config;
    rbufIsRxWinEnFunc isRxWinEnFn;
    rbufGetRxWinFunc  getRxWinFn;
    rbufChgRxWinFunc  chgRxWinFn;
    rbufDecapPduFunc  decapPduFn;
    rbufCheckObzpFunc checkObzpFn;
}CcioRbufAttr_t;

/* @4 bytes */
typedef struct
{
    RbufSize_t  dumSti;
    RbufSize_t  dumCnt;
}CcioDummyXfer_t;

/* @16 bytes */
typedef struct
{
    int16_t   remnSize;
    uint16_t  totalSize;
    uint16_t  mayPadding;
    uint16_t  rsvd;
    UlPduBlockList_t list;
}CcioFrameXfer_t;

/* @32 bytes */
typedef struct
{
    uint16_t    isXferChk :1;
    uint16_t    isCmplt   :1;
    uint16_t    isClrRemn :1;
    uint16_t    rsvdBits  :13;
    uint16_t    rsvd;
    RbufSize_t  xferCnt;          /* payload size, uldp/ulpdu ctrl block size is excluded! */
    RbufSize_t  batchSize;        /* total size of the xfer batch */
    RbufSize_t  startIdx;         /* record to calc latest complete xfer size, only updated by SW */
    RbufSize_t  nextSti;          /* expected next startIdx of xfer, only updated by SW */
    CcioDummyXfer_t dummy;
    CcioFrameXfer_t frame;
}CcioXferBatch_t;

/**
 * Structure which holds a ring buffer.
 * The buffer contains a buffer array as well as metadata for the ring buffer.
 * @84 bytes
 */
typedef struct CcioRbuf
{
    uint32_t     rbufId    :8;     /* guid associated with channel device, auto-generated */
    uint32_t     hasInit   :1;     /* rbuf is initialized or not */
    uint32_t     bmPtMode  :2;     /* bit1: full PT/NPT or not?, bit0: PT(pass through) or not? */
    uint32_t     isCctEn   :1;     /* set for readIdx tail alignment in PT mode */
    uint32_t     cctSize   :14;    /* cycle ctrl tail size. usb nonPT: CCIO_RBUF_NPT_HDR_SIZE, usb PT: 0; uart: user-specified size */
    uint32_t     ahcbSize  :6;     /* Additional-Header-Control-Block */
    uint32_t     alignVal  :3;
    uint32_t     isVolMode :1;     /* is rbuf mode(XPT) volatile or not? and won't be cleared if set! */
    uint32_t     isAdjRwEn :1;     /* adjust rbuf Rd&Wr zone or not? */
    uint32_t     rsvdBits  :27;
    uint16_t     rsvdBits1 :1;
    uint16_t     isThresEn :1;
    uint16_t     avlbThres :14;
    RbufSize_t   writeIdx;        /* update when ULDP HW fill the ring buffer, only update by HW, SW may read it */
    RbufSize_t   readIdx;         /* update when upper layer consumed the rbuf data and free it, only updated by SW */
    RbufSize_t   totalSize;       /* total size(CCIO_RBUF_RING_SIZE) of this ring buffer */
    uint8_t     *buffer;          /* real RX buffer starting address of a specific OUT EP/port */
    uint8_t     *xtraBuf;         /* extra rsvd buf for managing the second UlPduBlock_t */
    void        *chent;           /* entity the rbuf belongs to */

    CcioXferBatch_t   xbatch;
    rbufIsRxWinEnFunc isRxWinEnFn;
    rbufGetRxWinFunc  getRxWinFn;
    rbufChgRxWinFunc  chgRxWinFn;
    rbufDecapPduFunc  decapPduFn;
    rbufCheckObzpFunc checkObzpFn;

    UlPduBlock_t     *pendList;   /* pending ulpdu to be free, in addr order, only valid @PT mode */
}CcioRbuf_t;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
/**
 * @brief ccioRbufGet(void)
 * @details get a rbuf ctrl block for some entity
 *
 * @return !NULL the rbuf ctrl block; NULL failure.
 */
CcioRbuf_t* ccioRbufGet(void);

/**
 * @brief ccioRbufUnget(CcioRbuf_t *rbuf)
 * @details free a rbuf ctrl block
 *
 * @param rbuf The ring buffer to be freed
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufUnget(CcioRbuf_t *rbuf);

/**
 * @brief ccioRbufInit(CcioRbuf_t *rbuf, uint8_t *buf, RbufSize_t bufSize, void *chent, CcioRbufAttr_t *attr)
 *
 * @param rbuf    The ring buffer to be created
 * @param buf     The real buffer that rbuf points to
 * @param bufSize The size of pointed buffer by rbuf
 * @param chent   The channel entity the rbuf belongs to
 * @param attr    The the pre-inited info of the rbuf
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufInit(CcioRbuf_t *rbuf, uint8_t *buf, RbufSize_t bufSize, void *chent, CcioRbufAttr_t *attr);

/**
 * @brief ccioRbufDeinit(CcioRbuf_t *rbuf)
 * @details can also be used to reset the rbuf
 *
 * @param rbuf The ring buffer to be destroied
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufDeinit(CcioRbuf_t *rbuf);

/**
 * @brief ccioRbufGetTotalSize(uint8_t rbufId)
 * @details get the total size of a specific rbuf.
 *
 * @param rbufId   The buffer ID whose total size is get.
 * @return 0 succ; < 0 failure with errno.
 */
RbufSize_t ccioRbufGetTotalSize(uint8_t rbufId);

/**
 * @brief ccioRbufGetPtUlPcb(void)
 * @details get a ulpdu ctrl block for PT xfer
 *
 * @return !NULL succ; NULL failure.
 */
UlPduBlock_t* ccioRbufGetPtUlPcb(void);

/**
 * @brief ccioRbufUngetPtUlPcb(UlPduBlock_t *pcb)
 * @details give back the ulpdu ctrl block
 *
 * @param pcb  The ulpdu pcb that is applied before.
 * @return void.
 */
void ccioRbufUngetPtUlPcb(UlPduBlock_t *pcb);

/**
 * @brief ccioRbufSplitUlPayload(UlPduBlock_t *orgin, UlPduBlock_t *remain, uint8_t *splitPtr)
 * @details split a rbuf into 2 segments
 *
 * @param orgin    The original rbuf segment
 * @param remain   The splited remaining rbuf segment
 * @param splitPtr The rbuf ptr to be splited
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufSplitUlPayload(UlPduBlock_t *orgin, UlPduBlock_t *remain, uint8_t *splitPtr);

/**
 * @brief ccioRbufMergeUlPayload(UlPduBlockList_t *list, uint32_t *newSize)
 * @details merge the physical adjacent PT rbuf in the list
 *
 * @param list      The ulpdu list to be merged
 * @param newSize   The data length for all the block in list
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufMergeUlPayload(UlPduBlockList_t *list, uint32_t *newSize);

/**
 * @brief ccioRbufQueue(uint8_t rbufId, RbufSize_t size, uint32_t flags)
 * @details Adds an array of bytes to a rbuf by HW.
 *
 * @param rbufId The buffer ID in which the data should be placed.
 * @param size   The size of the array.
 * @param flags  The flags for enqueue.
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufQueue(uint8_t rbufId, RbufSize_t size, uint32_t flags);

/**
 * @brief ccioRbufDequeue(CcioRbuf_t *rbuf, UlPduBlock_t *ulpdu)
 * @details free the ulpdu data in rbuf by upper layer.
 *
 * @param rbufId The buffer ID from which the data is freed.
 * @param ulpdu  The UlPduBlock starting addr which is going to be freed.
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufDequeue(uint8_t rbufId, UlPduBlock_t *ulpdu);

/**
 * @brief ccioRbufFreeUlPdu(UlPduBlock_t *list)
 * @details free ulpdu list of rbuf type.
 *
 * @param list   The UlPduBlock list which is going to be freed.
 * @return void.
 */
void ccioRbufFreeUlPdu(UlPduBlock_t *list);

/**
 * @brief ccioRbufSelFlushXfer(uint8_t rbufId, RbufSize_t startIdx, RbufSize_t size)
 * @details selectively flush xfer data in rbuf as a result of exception/need.
 *
 * @param rbufId   The buffer ID from which the data is freed.
 * @param startIdx The xfer startIdx(need alignment) in rbuf to be freed.
 * @param size     The xfer size in rbuf to be freed.
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufSelFlushXfer(uint8_t rbufId, RbufSize_t startIdx, RbufSize_t size);

/**
 * @brief ccioRbufStartRecvXfer(uint8_t rbufId, uint8_t isReset, void *args, chdevUserSetFunc userSetFunc)
 * @details start to recv xfer data in rbuf.
 *
 * @param rbufId      The buffer ID from which the data is freed.
 * @param isReset     start to recv xfer from the beginning of rbuf or not.
 * @param args        The user's arguments.
 * @param userSetFunc The function for user to set config.
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufStartRecvXfer(uint8_t rbufId, uint8_t isReset, void *args, chdevUserSetFunc userSetFunc);

/**
 * @brief ccioRbufSetWorkState(uint8_t rbufId, uint8_t isSetData, CcioRbufConfig_t *config)
 *
 * @param rbuf      The ring buffer to be set.
 * @param isSetData set new data conf to hw or not.
 * @param config    rbuf config for working.
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ccioRbufSetWorkState(uint8_t rbufId, uint8_t isSetData, CcioRbufConfig_t *config);

#ifdef __cplusplus
}
#endif
#endif

