#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osasys.h"
#include "networkmgr.h"

#include DEBUG_LOG_HEADER_FILE
#include "ec_ring.h"

/*
 *  Description:
 *    Create a ring buffer for <data_size> characters.
 *  Param: iBuffSize, the size of buffer;
 *  Return: ring descriptor or NULL in case of error;
 *  Note: NONE
 */
ecRingT * pxEcRingCreate(INT32 iBuffSize)
{
    ecRingT * pNewRing = NULL;
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    pNewRing = (ecRingT *)pvPortMalloc_Psram(sizeof(ecRingT));
#else
	pNewRing = (ecRingT *)malloc(sizeof(ecRingT));
#endif
    if (pNewRing == NULL)
    {
        return pNewRing;
    }
    
    memset (pNewRing, 0, sizeof(ecRingT));
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
    pNewRing->pDataBuff = (UINT8 *)pvPortMalloc_Psram(iBuffSize);
#else
	pNewRing->pDataBuff = (UINT8 *)malloc(iBuffSize);
#endif
    if (pNewRing->pDataBuff == NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(pNewRing);
#else
		free(pNewRing);
#endif
        pNewRing = NULL;
        return pNewRing;
    }

    pNewRing->iBuffSize = iBuffSize;
    memset(pNewRing->pDataBuff, 0, iBuffSize);

    return pNewRing;
}

/*
 *  Description:
 *    Deletes a ring buffer and then set ring descriptor to NULL.
 *  Param: pDstRing, pointer to a ring descriptor;
 *  Return: NONE;
 *  Note: NONE
 */
void xEcRingDelete(ecRingT * pDstRing)
{
    if (pDstRing != NULL)
    {
        if (pDstRing->pDataBuff != NULL)
        {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
            vPortFree_Psram(pDstRing->pDataBuff);
#else
			free(pDstRing->pDataBuff);
#endif
            pDstRing->pDataBuff = NULL;
        }
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(pDstRing);
#else
		free(pDstRing);
#endif
        pDstRing = NULL;
    }
}

/*
 *  Description:
 *    Deletes all read elements from the buffer.
 *  Param: pDstRing, ring descriptor;
 *  Return: NONE;
 *  Note: NONE
 */
void xEcRingClear(ecRingT * pDstRing)
{
    if (pDstRing != NULL)
    {
        pDstRing->iPosRead = 0;
        pDstRing->iPosWrite= 0;
		pDstRing->idataCnt = 0;
    }
	memset(pDstRing->pDataBuff,0x00,pDstRing->iBuffSize);
}

/*
 *  Description:
 *    Put characters into ring buffer.
 *  Param: pDstRing, ring descriptor;
 *  Param: pDataBuff, the pointer of buffer;
 *  Param: iDataLen, length of data;
 *  Return: iLenWrite, the length of data has written;
 *  Note: NONE
 */
INT32 xEcRingWrite(ecRingT * pDstRing, UINT8 * pDataBuff, INT32 iDataLen)
{
    INT32 iLenWrite = 0;
    
    if (pDstRing != NULL)
    {
        while (iLenWrite < iDataLen)
        {
            INT32 iLenWrite1 = 0;

            iLenWrite1 = xEcRingWriteChar(pDstRing, pDataBuff[iLenWrite]);
            if (iLenWrite1 == 0)
            {
                break;
            }
            
            iLenWrite++;
        }
    }
    pDstRing->idataCnt = pDstRing->idataCnt + iLenWrite;
    return iLenWrite;
}

INT32 xEcRingWriteEx(ecRingT * pDstRing, UINT8 * pDataBuff, INT32 iDataLen)
{
    INT32 iLenWrite = 0;
	INT32 iWriCnt = 0;
	INT32 iNextWriPos = 0;
    INT32 iRemainSize = iDataLen;

	if(!pDstRing || !pDstRing->pDataBuff || !pDataBuff)
		return iLenWrite;
	
	iNextWriPos = pDstRing->iPosWrite + 1;
	do{
		if((iNextWriPos == pDstRing->iPosRead) || ((pDstRing->iBuffSize - iNextWriPos) == pDstRing->iPosRead))
			break;
		else if(pDstRing->iPosRead > iNextWriPos)
		{
			iWriCnt = ((iRemainSize > (pDstRing->iPosRead - pDstRing->iPosWrite)) ? pDstRing->iPosRead - pDstRing->iPosWrite : iRemainSize);
			memcpy(pDstRing->pDataBuff +pDstRing->iPosWrite,pDataBuff + iLenWrite,iWriCnt);
			iLenWrite += iWriCnt;
			pDstRing->iPosWrite += iWriCnt;
			EC_ASSERT(pDstRing->iPosWrite < pDstRing->iBuffSize,pDstRing->iPosWrite,0,0);
			break;
		}
		else if(pDstRing->iBuffSize > iNextWriPos)
		{
			iWriCnt = ((iRemainSize > (pDstRing->iBuffSize - pDstRing->iPosWrite)) ? pDstRing->iBuffSize - pDstRing->iPosWrite : iRemainSize);	
			memcpy(pDstRing->pDataBuff + pDstRing->iPosWrite, pDataBuff  + iLenWrite, iWriCnt);
			
			pDstRing->iPosWrite += iWriCnt;
			iLenWrite += iWriCnt;
			iRemainSize = iRemainSize - iWriCnt;
			if(pDstRing->iPosWrite  == pDstRing->iBuffSize)
			{
				iNextWriPos = pDstRing->iPosWrite = 0;
			}
			else
				iNextWriPos = pDstRing->iPosWrite + 1;
			EC_ASSERT(pDstRing->iPosWrite < pDstRing->iBuffSize,pDstRing->iPosWrite,0,0);
		}
		else
		{
			iWriCnt = 1;
			pDstRing->pDataBuff[pDstRing->iPosWrite] = pDataBuff[iLenWrite];
			EC_ASSERT(pDstRing->iPosWrite < pDstRing->iBuffSize,pDstRing->iPosWrite,0,0);

			iLenWrite += iWriCnt;
			iRemainSize = iRemainSize - iWriCnt;
			iNextWriPos = pDstRing->iPosWrite = 0;
		}
		
		
	}while(iRemainSize > 0);
    
    pDstRing->idataCnt = pDstRing->idataCnt + iLenWrite;
    return iLenWrite;
}

/*
 *  Description:
 *    Get a character from ring buffer(but not clear data).
 *  Param: pDstRing, ring descriptor;
 *  Param: p_char_buf, a buffer for saving character;
 *  Return: iLenRead, 0, fail;1, success.
 *  Note: NONE
 */
INT32 xEcRingPrereadGetFirstChar(ecRingT * pDstRing, UINT8 * pCharBuff)
{
    INT32 iLenRead = 0;

    if (pDstRing != NULL)
    {
        pDstRing->iPosPreread = pDstRing->iPosRead;
        if (pDstRing->iPosPreread != pDstRing->iPosWrite)
        {
            *pCharBuff = pDstRing->pDataBuff[pDstRing->iPosPreread];
            if (pDstRing->iPosPreread + 1 < pDstRing->iBuffSize)
            {
                pDstRing->iPosPreread++;
            }
            else
            {
                pDstRing->iPosPreread = 0;
            }

            iLenRead = 1;
        }
    }

    return iLenRead;
}

/*
 *  Description:
 *    Get a character from ring buffer(but not clear data).
 *  Param: pDstRing, ring descriptor;
 *  Param: p_char_buf, a buffer for saving character;
 *  Return: iLenRead, 0, fail;1, success.
 *  Note: xEcRingPrereadGetNextChar should be called and return 1.
 */
INT32 xEcRingPrereadGetNextChar(ecRingT * pDstRing, UINT8 * pCharBuff)
{
    INT32 iLenRead = 0;

    if (pDstRing != NULL)
    {
        if (pDstRing->iPosPreread != pDstRing->iPosWrite)
        {
            *pCharBuff = pDstRing->pDataBuff[pDstRing->iPosPreread];
            if (pDstRing->iPosPreread + 1 < pDstRing->iBuffSize)
            {
                pDstRing->iPosPreread++;
            }
            else
            {
                pDstRing->iPosPreread = 0;
            }

            iLenRead = 1;
        }
    }

    return iLenRead;
}

/*
 *  Description:
 *    Get a character from ring buffer.
 *  Param: pDstRing, ring descriptor;
 *  Param: p_char_buf, a buffer for saving character;
 *  Return: iLenRead, 0, fail;1, success.
 *  Note: NONE
 */
INT32 xEcRingReadChar(ecRingT * pDstRing, UINT8 * pCharBuff)
{
   	if (pDstRing != NULL)
    {
        if (pDstRing->iPosRead != pDstRing->iPosWrite)
        {
            *pCharBuff = pDstRing->pDataBuff[pDstRing->iPosRead];
            if (pDstRing->iPosRead + 1 < pDstRing->iBuffSize)
            {
                pDstRing->iPosRead++;
            }
            else
            {
                pDstRing->iPosRead = 0;
            }
        }
    }

    return 0;
}

/*
 *  Description:
 *    Put a character into ring buffer.
 *  Param: pDstRing, ring descriptor;
 *  Param: u_char_value, a character;
 *  Return: iLenWrite, 0, fail;1, success.
 *  Note: NONE
 */
INT32 xEcRingWriteChar(ecRingT * pDstRing, UINT8 uCharValue)
{
    INT32 iLenWrite = 0;
    INT32 iNextPosWrite = 0;

    if (pDstRing != NULL)
    {
        if (pDstRing->iPosWrite + 1 < pDstRing->iBuffSize)
        {
            iNextPosWrite = pDstRing->iPosWrite + 1;
        }
        else
        {
            iNextPosWrite = 0;
        }

        if (iNextPosWrite != pDstRing->iPosRead)
        {
            pDstRing->pDataBuff[pDstRing->iPosWrite] = uCharValue;
            pDstRing->iPosWrite = iNextPosWrite;

            iLenWrite = 1;
        }
    }
    
    return iLenWrite;
}

/*
 *  Description:
 *    Get characters from ring buffer(but not clear data).
 *  Param: pDstRing, ring descriptor;
 *  Param: pDataBuff, the pointer of buffer;
 *  Param: iDataLen, the size of buffer;
 *  Return: iLenRead, the length of data has been read;
 *  Note: NONE
 */
INT32 xEcRingPreread(ecRingT * pDstRing, UINT8 * pDataBuff, INT32 iBuffSize)
{
    INT32 iLenRead = 0;

    INT32 iLenReadable1 = 0;
    INT32 iLenReadable2 = 0;

    if (pDstRing != NULL)
    {
        if (pDstRing->iPosRead <= pDstRing->iPosWrite)
        {
            iLenReadable1 = pDstRing->iPosWrite - pDstRing->iPosRead;

            if (iBuffSize <= iLenReadable1)
            {
                iLenReadable1 = iBuffSize;
            }

            if (iLenReadable1 > 0)
            {
                memcpy(pDataBuff, &(pDstRing->pDataBuff[pDstRing->iPosRead]), iLenReadable1);
                iLenRead = iLenReadable1;
            }
        }
        else
        {
            iLenReadable1 = pDstRing->iBuffSize - pDstRing->iPosRead;
            iLenReadable2 = pDstRing->iPosWrite;

            if (iBuffSize <= iLenReadable1)
            {
                iLenReadable1 = iBuffSize;
                iLenReadable2 = 0;
            }
            else
            {
                if (iBuffSize <= iLenReadable1 + iLenReadable2)
                {
                    iLenReadable2 = iBuffSize - iLenReadable1;
                }
            }
            
            if (iLenReadable1 > 0)
            {
                memcpy(pDataBuff, &(pDstRing->pDataBuff[pDstRing->iPosRead]), iLenReadable1);
                iLenRead = iLenReadable1;
            }

            if (iLenReadable2 > 0)
            {
                memcpy(&(pDataBuff[iLenRead]), &(pDstRing->pDataBuff[0]), iLenReadable2);
                iLenRead += iLenReadable2;
            }
        }
    }

    return iLenRead;
}

/*
 *  Description:
 *    Get characters from ring buffer.
 *  Param: pDstRing, ring descriptor;
 *  Param: pDataBuff, the pointer of buffer;
 *  Param: iDataLen, the size of buffer;
 *  Return: iLenRead, the length of data has been read;
 *  Note: NONE
 */
INT32 xEcRingRead(ecRingT * pDstRing, UINT8 * pDataBuff, INT32 iBuffSize)
{
    INT32 iLenRead = 0;
    
    INT32 iLenReadable1 = 0;
    INT32 iLenReadable2 = 0;
    
    if (pDstRing != NULL)
    {
        if (pDstRing->iPosRead <= pDstRing->iPosWrite)
        {
            iLenReadable1 = pDstRing->iPosWrite - pDstRing->iPosRead;
            
            if (iBuffSize <= iLenReadable1)
            {
                iLenReadable1 = iBuffSize;
            }
            
            if (iLenReadable1 > 0)
            {
                memcpy(pDataBuff, &(pDstRing->pDataBuff[pDstRing->iPosRead]), iLenReadable1);
				memset(&(pDstRing->pDataBuff[pDstRing->iPosRead]),0,iLenReadable1);
                iLenRead = iLenReadable1;
                pDstRing->iPosRead += iLenRead;
            }
        }
        else
        {
            iLenReadable1 = pDstRing->iBuffSize - pDstRing->iPosRead;
            iLenReadable2 = pDstRing->iPosWrite;
            
            if (iBuffSize <= iLenReadable1)
            {
                iLenReadable1 = iBuffSize;
                iLenReadable2 = 0;
            }
            else
            {
                if (iBuffSize <= iLenReadable1 + iLenReadable2)
                {
                    iLenReadable2 = iBuffSize - iLenReadable1;
                }
            }
            
            if (iLenReadable1 > 0)
            {
                memcpy(pDataBuff, &(pDstRing->pDataBuff[pDstRing->iPosRead]), iLenReadable1);
				memset(&(pDstRing->pDataBuff[pDstRing->iPosRead]),0,iLenReadable1);
                iLenRead = iLenReadable1;
                pDstRing->iPosRead += iLenRead;
            }
            
            if (iLenReadable2 > 0)
            {
                memcpy(&(pDataBuff[iLenRead]), &(pDstRing->pDataBuff[0]), iLenReadable2);
				memset(&(pDstRing->pDataBuff[0]),0,iLenReadable2);
                iLenRead += iLenReadable2;
                pDstRing->iPosRead = iLenReadable2;
            }
        }
    }
    return iLenRead;
}

/*
 *  Description:
 *    Get information according to the type.
 *  Param: pDstRing, ring descriptor;
 *  Param: iOptType, the specified type;
 *  Return: iRetValue, the length/size of the type.
 *  Note: None
 */
INT32 xEcRingGetOption(ecRingT * pDstRing, INT32 iOptType)
{
    INT32 iRetValue = 0;

    if (pDstRing != NULL)
    {
        switch (iOptType)
        {
        case E_LRO_BUFF_SIZE:
            {
                iRetValue = pDstRing->iBuffSize;
                break;
            }
		case E_LRO_BUFF_FILL_SIZE:
            {
                iRetValue = pDstRing->idataCnt;
                break;
            }
        case E_LRO_FREE_SIZE:
            {
                if (pDstRing->iPosRead != pDstRing->iPosWrite)
                {
                    if (pDstRing->iPosRead < pDstRing->iPosWrite)
                    {
                        iRetValue = pDstRing->iBuffSize - pDstRing->iPosWrite + pDstRing->iPosRead - 1;
                    }
                    else
                    {
                        iRetValue = pDstRing->iPosRead - pDstRing->iPosWrite - 1;
                    }
                }
                else
                {
                    iRetValue = pDstRing->iBuffSize -1;
                }
                break;
            }
        case E_LRO_DATA_SIZE:
            {
                if (pDstRing->iPosRead != pDstRing->iPosWrite)
                {
                    if (pDstRing->iPosRead < pDstRing->iPosWrite)
                    {
                        iRetValue = pDstRing->iPosWrite - pDstRing->iPosRead;
                    }
                    else
                    {
                        iRetValue = pDstRing->iBuffSize - pDstRing->iPosRead + pDstRing->iPosWrite;
                    }
                }
                break;
            }
        case E_LRO_NEXT_READ_SIZE:
            {
                if (pDstRing->iPosRead != pDstRing->iPosWrite)
                {
                    if (pDstRing->iPosRead < pDstRing->iPosWrite)
                    {
                        iRetValue = pDstRing->iPosWrite - pDstRing->iPosRead;
                    }
                    else
                    {
                        iRetValue = pDstRing->iBuffSize - pDstRing->iPosRead;
                    }
                }

                break;
            }
        case E_LRO_NEXT_WRITE_SIZE:
            {
                if (pDstRing->iPosRead == pDstRing->iPosWrite)
                {
                    pDstRing->iPosRead = 0;
                    pDstRing->iPosWrite = 0;

                    iRetValue = pDstRing->iBuffSize - 1;
                }
                else
                {
                    if (pDstRing->iPosRead < pDstRing->iPosWrite)
                    {
                        iRetValue = pDstRing->iBuffSize - pDstRing->iPosWrite;
                        if (pDstRing->iPosRead == 0)
                        {
                            iRetValue = iRetValue - 1;
                        }
                    }
                    else
                    {
                        iRetValue = pDstRing->iPosRead - pDstRing->iPosWrite - 1;
                    }
                }

                break;
            }
        default:
            {
                break;
            }
        }
    }

    return iRetValue;
}


