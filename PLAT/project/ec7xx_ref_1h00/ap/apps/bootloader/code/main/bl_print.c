#include "bl_print.h"
#include <string.h>
#include "sctdef.h"

extern int io_putchar(int ch);

PLAT_BL_CIRAM_FLASH_TEXT static void EC_flush_fullBuf(const char *pBuf)
{
    while('\0' != *pBuf)
    {
        io_putchar(*pBuf++);
    }
    return;
}

PLAT_BL_CIRAM_FLASH_TEXT void EC_Ita(char * pStr, uint32_t nNum, EC_NumBase_E eBase)
{
    static char * pTemp;
    pTemp =pStr;
    if(nNum != 0)
    {
        EC_Ita(pStr, nNum / eBase, eBase);
        if (eBase == EC_NUM_BASE_10)

        {
            *pTemp++ = nNum % eBase + '0';
        }
        else
        {
            if (nNum % eBase <= 9)
            {
                *pTemp++ = nNum % eBase + '0';
            }
            else
            {
                *pTemp++ = nNum % eBase - 10 + 'a';
            }
        }
    }
    *pTemp = '\0';
}

PLAT_BL_CIRAM_FLASH_TEXT char * EC_Itoa(char * pStr, const int rInt, EC_NumBase_E eBase, uint8_t ucWidth, uint8_t ucFillVal)
{
    if (eBase == EC_NUM_BASE_10)
    {
        if(rInt ==0)
        {
            *pStr++ = '0';
            *pStr = '\0';
        }
        else if(rInt > 0)
        {
            EC_Ita(pStr, rInt, eBase);
        }
        else
        {
            *pStr = '-';
            EC_Ita(pStr + 1, -rInt, eBase);
        }
    }
    else
    {
        char *pTmp = pStr;
//        *pTmp++ = '0';
//        *pTmp++ = 'x';
        if (rInt ==0)
        {
            *pTmp++ = '0';
            *pTmp++ = '0';
            *pTmp = '\0';
        }
        else
        {
            char cBuf[9] = "";
            EC_Ita(cBuf, rInt, eBase);
            if (strlen((char *)cBuf) >= ucWidth)
            {
                memcpy((void *)pTmp, (void *)cBuf, strlen((char *)cBuf));
            }
            else
            {
                for (int i = ucWidth - strlen((char *)cBuf); i > 0; i--)
                {
                    *pTmp++ = ucFillVal;
                }
                memcpy((void *)pTmp, (void *)cBuf, strlen((char *)cBuf));
            }
            pTmp[strlen((char *)cBuf)] = '\0';
        }
    }
    return pStr;
}

PLAT_BL_CIRAM_FLASH_TEXT int EC_Snprintf(char *pBuf, int size, const char * pFormat, ...)
{
	if ((NULL == pBuf) || (NULL == pFormat) || (0 >= size))
	{
		return -1;
	}

	const int * pMove = (const int *)(&pFormat + 1);
	char Szbuff[32] = "";
	const char * pTemp;
	const char * pBufHead = pBuf;
	uint8_t ucHexWidth = 0;
	uint8_t ucHexFilVal = 0;

	while(('\0' != *pFormat) && (1 < size))
    {
        if('%' == *pFormat)
        {
            ++pFormat;
            switch(*pFormat)
            {
                case 'x':
                case 'X':
                {
                    EC_Itoa(Szbuff, *pMove++, EC_NUM_BASE_16, 0, 0);
                    pTemp = Szbuff;
                    while('\0' != *pTemp)
                    {
                        *pBuf++ = *pTemp++;
                        if (--size == 1)
                        {
                            break;
                        }
                    }
                    break;
                }
                case 's':
                case 'S':
                {
                    pTemp = (const char *)(*pMove++);
                    while ('\0' != *pTemp)
                    {
                        *pBuf++ = *pTemp++;
                        if (--size == 1)
                        {
                            break;
                        }
                    }
                    break;
                }
                case 'd':
                case 'D':
                {
                    EC_Itoa(Szbuff, *pMove++, EC_NUM_BASE_10, 0, 0);
                    pTemp = Szbuff;
                    while('\0' != *pTemp)
                    {
                        *pBuf++ = *pTemp++;
                        if (--size == 1)
                        {
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    // suppose that the format is like %02x or %02X
                    ucHexFilVal = *pFormat++;
                    ucHexWidth = *pFormat++ - '0';
                    if (('x' != *pFormat) && ('X' != *pFormat))
                    {
                        EC_flush_fullBuf("This fomart is not support, should like %02x or %02X\r\n");
                        return -1;
                    }
                    EC_Itoa(Szbuff, *pMove++, EC_NUM_BASE_16, ucHexWidth, ucHexFilVal);
                    pTemp = Szbuff;
                    while('\0' != *pTemp)
                    {
                        *pBuf++ = *pTemp++;
                        if (--size == 1)
                        {
                            break;
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            *pBuf++ = *pFormat;
            size--;
        }
        ++pFormat;
    }
    *pBuf = '\0';
	return  (int)(pBuf - pBufHead);
}

PLAT_BL_CIRAM_FLASH_TEXT int EC_Sprintf(char *pBuf, const char * pFormat, ...)
{
    if ((NULL == pBuf) || (NULL == pFormat))
    {
        return -1;
    }

    const int * pMove = (const int *)(&pFormat + 1);
    char Szbuff[32] = "";
    const char * pTemp;
	const char * pBufHead = pBuf;
	uint8_t ucHexWidth = 0;
	uint8_t ucHexFilVal = 0;

    while('\0' != *pFormat)
    {
        if('%' == *pFormat)
        {
            ++pFormat;
            switch(*pFormat)
            {
                case 'x':
                case 'X':
                {
                    EC_Itoa(Szbuff, *pMove++, EC_NUM_BASE_16, 0, 0);
                    pTemp = Szbuff;
                    while('\0' != *pTemp)
                    {
                        *pBuf++ = *pTemp++;
                    }
                    break;
                }
                case 's':
                case 'S':
                {
                    pTemp = (const char *)(*pMove++);
                    while ('\0' != *pTemp)
                    {
                        *pBuf++ = *pTemp++;
                    }
                    break;
                }
                case 'd':
                case 'D':
                {
                    EC_Itoa(Szbuff, *pMove++, EC_NUM_BASE_10, 0, 0);
                    pTemp = Szbuff;
                    while('\0' != *pTemp)
                    {
                        *pBuf++ = *pTemp++;
                    }
                    break;
                }
                case '%':
                {
                    *pBuf++ = '%';
                    break;
                }
                default:
                {
                    // suppose that the format is like %02x or %02X
                    ucHexFilVal = *pFormat++;
                    ucHexWidth = *pFormat++ - '0';
                    if (('x' != *pFormat) && ('X' != *pFormat))
                    {
                        EC_flush_fullBuf("This fomart is not support, should like %02x or %02X\r\n");
                        return -1;
                    }

                    EC_Itoa(Szbuff, *pMove++, EC_NUM_BASE_16, ucHexWidth, ucHexFilVal);
                    pTemp = Szbuff;
                    while('\0' != *pTemp)
                    {
                        *pBuf++ = *pTemp++;
                    }
                    break;
                }
            }
        }
        else
        {
            *pBuf++ = *pFormat;
        }
        ++pFormat;
    }
    *pBuf = '\0';
    return  (int)(pBuf - pBufHead);
}

PLAT_BL_CIRAM_FLASH_TEXT int EC_Printf(const char * pFormat, ...)
{
    if(NULL != pFormat)
    {
        const int * pMove = (const int *)(&pFormat + 1);
        char SzBuff[EC_PRINTF_BUFFER_LENGTH] = "";
        char Szbuff[32] = "";
        char * pSzBuff = SzBuff;
        const char * pTemp;
        int iTmpFlag = 0;
        int iCounter = 0;

        while('\0' != *pFormat)
        {
            if('%' == *pFormat)
            {
                ++pFormat;
                switch(*pFormat)
                {
                    case 'x':
                    case 'X':
                    {
                        EC_Itoa(Szbuff, *pMove++, EC_NUM_BASE_16, 0, 0);
                        pTemp = Szbuff;
                        while('\0' != *pTemp)
                        {
                            *pSzBuff++ = *pTemp++;
                            iTmpFlag++;
                            iCounter++;
                            if (iTmpFlag >= EC_PRINTF_BUFFER_LENGTH - 1)
                            {
                                *pSzBuff = '\0';
                                EC_flush_fullBuf(SzBuff);
                                pSzBuff = SzBuff;
                                iTmpFlag = 0;
                            }
                        }
                        break;
                    }
                    case 's':
                    case 'S':
                    {
                        pTemp = (const char *)(*pMove++);
                        while ('\0' != *pTemp)
                        {
                            *pSzBuff++ = *pTemp++;
                            iTmpFlag++;
                            iCounter++;
                            if (iTmpFlag >= EC_PRINTF_BUFFER_LENGTH - 1)
                            {
                                *pSzBuff = '\0';
                                EC_flush_fullBuf(SzBuff);
                                pSzBuff = SzBuff;
                                iTmpFlag = 0;
                            }
                        }
                        break;
                    }
                    case 'd':
                    case 'D':
                    {
                        EC_Itoa(Szbuff, *pMove++, EC_NUM_BASE_10, 0, 0);
                        pTemp = Szbuff;
                        while('\0' != *pTemp)
                        {
                            *pSzBuff++ = *pTemp++;
                            iTmpFlag++;
                            iCounter++;
                            if (iTmpFlag >= EC_PRINTF_BUFFER_LENGTH - 1)
                            {
                                *pSzBuff = '\0';
                                EC_flush_fullBuf(SzBuff);
                                pSzBuff = SzBuff;
                                iTmpFlag = 0;
                            }
                        }
                        break;
                    }
                    case '%':
                    {
                        iTmpFlag++;
                        iCounter++;
                        *pSzBuff++ = '%';
                        if (iTmpFlag >= EC_PRINTF_BUFFER_LENGTH - 1)
                        {
                            *pSzBuff = '\0';
                            EC_flush_fullBuf(SzBuff);
                            pSzBuff = SzBuff;
                            iTmpFlag = 0;
                        }
                        break;
                    }
                    default:
                    {
                        pTemp = pFormat;
                        int i = 0;
                        Szbuff[i++] = '%';
                        while ((*pTemp != 'X') && (*pTemp != 'x') && \
                            (*pTemp != 'S') && (*pTemp != 's') && \
                            (*pTemp != 'd') && (*pTemp != 'D'))
                        {
                            Szbuff[i++] = *pTemp++;
                        }
                        Szbuff[i++] = *pTemp;;
                        Szbuff[i] = '\0';
                        EC_Sprintf(SzBuff, "This format (%s) is not support, just support %s\r\n",Szbuff,"%x, %s and %d");
                        EC_flush_fullBuf(SzBuff);
                        return -1;
                    }
                }
            }
            else
            {
                iTmpFlag++;
                iCounter++;
                *pSzBuff++ = *pFormat;
                if (iTmpFlag >= EC_PRINTF_BUFFER_LENGTH - 1)
                {
                    *pSzBuff = '\0';
                    EC_flush_fullBuf(SzBuff);
                    pSzBuff = SzBuff;
                    iTmpFlag = 0;
                }
            }
            ++pFormat;
        }
        *pSzBuff = '\0';
        pSzBuff = SzBuff;
        while('\0' != *pSzBuff)
        {
            io_putchar(*pSzBuff++);
        }
        return iCounter;
    }
    return -1;
}

