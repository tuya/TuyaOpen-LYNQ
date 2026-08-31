#ifndef __EC_RING_H__
#define __EC_RING_H__
#include "commontypedef.h"
typedef struct _ecRingT
{
    UINT8 * pDataBuff;
	INT32   idataCnt;
    INT32   iBuffSize;
    INT32   iPosRead;                                   
    INT32   iPosWrite;
    INT32   iPosPreread;
} ecRingT;

typedef enum _EC_RING_OPTION_E
{
    E_LRO_BUFF_SIZE,
	E_LRO_BUFF_FILL_SIZE,
    E_LRO_FREE_SIZE,
    E_LRO_DATA_SIZE,   
    E_LRO_NEXT_READ_SIZE,
    E_LRO_NEXT_WRITE_SIZE,
    E_LRO_INVALID =0xFF
} EC_RING_OPTION_E;

/*
 *  Description:
 *    Create a ring buffer for <data_size> characters.
 *  Param: iBuffSize, the size of buffer;
 *  Return: ring descriptor or NULL in case of error;
 *  Note: NONE
 */
ecRingT * pxEcRingCreate(INT32 iBuffSize);

/*
 *  Description:
 *    Get characters from ring buffer(but not clear data).
 *  Param: pDstRing, ring descriptor;
 *  Param: pDataBuff, the pointer of buffer;
 *  Param: i_data_len, the size of buffer;
 *  Return: i_len_read, the length of data has been read;
 *  Note: NONE
 */
INT32 xEcRingPreread(ecRingT * pDstRing, UINT8 * pDataBuff, INT32 iBuffSize);

/*
 *  Description:
 *    Get characters from ring buffer.
 *  Param: pDstRing, ring descriptor;
 *  Param: pDataBuff, the pointer of buffer;
 *  Param: i_data_len, the size of buffer;
 *  Return: i_len_read, the length of data has been read;
 *  Note: NONE
 */
INT32 xEcRingRead(ecRingT * pDstRing, UINT8 * pDataBuff, INT32 iBuffSize);

/*
 *  Description:
 *    Put characters into ring buffer.
 *  Param: pDstRing, ring descriptor;
 *  Param: pDataBuff, the pointer of buffer;
 *  Param: i_data_len, length of data;
 *  Return: i_len_write, the length of data has been written;
 *  Note: NONE
 */
INT32 xEcRingWrite(ecRingT * pDstRing, UINT8 * pDataBuff, INT32 iDataLen);
/*
 *  Description:
 *    Put characters into ring buffer.
 *  Param: pDstRing, ring descriptor;
 *  Param: pDataBuff, the pointer of buffer;
 *  Param: i_data_len, length of data;
 *  Return: i_len_write, the length of data has been written;
 *  Note: NONE
*/
INT32 xEcRingWriteEx(ecRingT * pDstRing, UINT8 * pDataBuff, INT32 iDataLen);

/*
 *  Description:
 *    Get a character from ring buffer.
 *  Param: pDstRing, ring descriptor;
 *  Param: pCharBuff, a buffer for saving character;
 *  Return: i_len_read, 0, fail;1, success.
 *  Note: NONE
 */
INT32 xEcRingReadChar(ecRingT * pDstRing, UINT8 * pCharBuff);

/*
 *  Description:
 *    Put a character into ring buffer.
 *  Param: pDstRing, ring descriptor;
 *  Param: uCharValue, a character;
 *  Return: i_len_write, 0, fail;1, success.
 *  Note: NONE
 */
INT32 xEcRingWriteChar(ecRingT * pDstRing, UINT8 uCharValue);

/*
 *  Description:
 *    Get a character from ring buffer(but not clear data).
 *  Param: pDstRing, ring descriptor;
 *  Param: pCharBuff, a buffer for saving character;
 *  Return: i_len_read, 0, fail;1, success.
 *  Note: NONE
 */
INT32 xEcRingPrereadGetFirstChar(ecRingT * pDstRing, UINT8 * pCharBuff);

/*
 *  Description:
 *    Get a character from ring buffer(but not clear data).
 *  Param: pDstRing, ring descriptor;
 *  Param: pCharBuff, a buffer for saving character;
 *  Return: i_len_read, 0, fail;1, success.
 *  Note: ec_ring_preread_getfirstchar should be called and return 1.
 */
INT32 xEcRingPrereadGetNextChar(ecRingT * pDstRing, UINT8 * pCharBuff);

/*
 *  Description:
 *    Get information according to the type.
 *  Param: pDstRing, ring descriptor;
 *  Param: iOptType, the specified type;
 *  Return: i_ret_value, the length/size of the type.
 *  Note: None
 */
INT32 xEcRingGetOption(ecRingT * pDstRing, INT32 iOptType);

/*
 *  Description:
 *    Deletes all read elements from the buffer.
 *  Param: pDstRing, ring descriptor;
 *  Return: NONE;
 *  Note: NONE
 */
void xEcRingClear(ecRingT * pDstRing);

/*
 *  Description:
 *    Deletes a ring buffer and then set ring descriptor to NULL.
 *  Param: pDstRing, pointer to a ring descriptor;
 *  Return: NONE;
 *  Note: NONE
 */
void xEcRingDelete(ecRingT * pDstRing);

#endif /* __EC_RING_H__ */

