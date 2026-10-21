/****************************************************************************
 *
 * Description:  TTS play entry source file
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "cmsis_os2.h"
#include "osasys.h"

#include "ivTTS_resource_irf.h"
#include "ivTTS.h"
#include "ol_log.h"
#include "tts_api.h"


#define ivTTS_HEAP_SIZE                     (80 * 1024)

PLAT_FPSRAM_ZI static uint8_t gTtsHeap[ivTTS_HEAP_SIZE] = {0};


/* read resource callback */
ivBool ivCall readResCB(
    ivPointer       pParameter,     /* [in] user callback parameter */
    ivPointer       pBuffer,        /* [out] read resource buffer */
    ivResAddress    iPos,           /* [in] read start position */
    ivResSize       nSize)          /* [in] read size */
{
    memcpy((uint8_t*)pBuffer, (uint8_t*)pParameter + iPos, nSize);
    return ivTrue;
}

/* output callback */
ivTTSErrID ivCall outputCB(
    ivPointer       pParameter,     /* [in] user callback parameter */
    ivUInt16        nCode,          /* [in] output data code */
    ivCPointer      pcData,         /* [in] output data buffer */
    ivSize          nSize)          /* [in] output data size */
{
    OL_LOG_INFO("ivTTS outputCB : output data size = %d", nSize);

    //audio_play_tts_data(pcData, nSize);

    return ivTTS_ERR_OK;
}

/* progress callback */
ivTTSErrID ivCall progressCB(
    ivPointer       pParameter,     /* [in] user callback parameter */
    ivUInt32        iProcPos,       /* [in] current processing position */
    ivUInt32        nProcLen )      /* [in] current processing length */
{
    OL_LOG_INFO("ivTTS progressCB : Pos = %d, Len = %d", iProcPos, nProcLen);
    return ivTTS_ERR_OK;
}

ivSize tts_data_codepage(tts_decode_enum decode)
{
    ivSize codepage = ivTTS_CODEPAGE_GBK;

    switch(decode)
    {
        case TTS_TYPE_GBK:
            codepage = ivTTS_CODEPAGE_GBK;
            break;
        case TTS_TYPE_UTF8:
            codepage = ivTTS_CODEPAGE_UTF8;
            break;
        case TTS_TYPE_UTF16LE:
            codepage = ivTTS_CODEPAGE_UTF16LE;
            break;
        case TTS_TYPE_UTF16BE:
            codepage = ivTTS_CODEPAGE_UTF16BE;
            break;
    }

    return codepage;
}

int tts_data_convert(char* p_text, uint32_t p_length, tts_decode_enum decode)
{
    ivHTTS hTTS;
    ivTResPackDescExt tResPackDesc;
    ivTTSErrID ivReturn = 0;

    if(NULL == p_text || p_length <= 0)
    {
        OL_LOG_PRINTF("ivTTS input param check error");
        return -1;
    }

    memset(gTtsHeap, 0, ivTTS_HEAP_SIZE);
    memset(&tResPackDesc, 0, sizeof(tResPackDesc));

    tResPackDesc.pCBParam = (ivPointer)mlp_resource;
    tResPackDesc.pfnRead = readResCB;
    tResPackDesc.pfnMap = NULL;
    tResPackDesc.nSize = 0;

    ivReturn = ivTTS_Create(&hTTS, (ivPointer)gTtsHeap, ivTTS_HEAP_SIZE, ivNull, (ivPResPackDescExt)&tResPackDesc, (ivSize)1, NULL);
    if(ivTTS_ERR_OK != ivReturn)
    {
        OL_LOG_PRINTF("ivTTS_Create return 0x%X\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_OUTPUT_CALLBACK, (ivUInt32)outputCB);
    if(ivTTS_ERR_OK != ivReturn)
    {
        OL_LOG_PRINTF("ivTTS_SetParam ivTTS_PARAM_OUTPUT_CALLBACK return 0x%x\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_PROGRESS_CALLBACK, (ivUInt32)progressCB);
    if (ivReturn)
    {
        OL_LOG_PRINTF("ivTTS_SetParam ivTTS_PARAM_PROGRESS_CALLBACK return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivSize codepage = tts_data_codepage(decode);
    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_INPUT_CODEPAGE, codepage);
    if(ivTTS_ERR_OK != ivReturn)
    {
        OL_LOG_PRINTF("ivTTS_SetParam ivTTS_PARAM_INPUT_CODEPAGE return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_AUTO);
    if(ivTTS_ERR_OK != ivReturn)
    {
        OL_LOG_PRINTF("ivTTS_SetParam ivTTS_PARAM_LANGUAGE return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_VOLUME, ivTTS_VOLUME_NORMAL);
    if(ivTTS_ERR_OK != ivReturn)
    {
        OL_LOG_PRINTF("ivTTS_SetParam ivTTS_PARAM_VOLUME return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_VOICE_SPEED, ivTTS_SPEED_NORMAL);
    if(ivTTS_ERR_OK != ivReturn)
    {
        OL_LOG_PRINTF("ivTTS_SetParam ivTTS_PARAM_VOICE_SPEED return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_XIAOYAN);
    if(ivTTS_ERR_OK != ivReturn)
    {
        OL_LOG_PRINTF("ivTTS_SetParam ivTTS_PARAM_ROLE return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SynthText(hTTS, ivText(p_text), (ivSize)p_length);
    OL_LOG_PRINTF("ivTTS_SynthText ivReturn 0x%x", ivReturn);

    ivReturn = ivTTS_Destroy(hTTS);
    OL_LOG_PRINTF("ivTTS_Destroy ivReturn 0x%x", ivReturn);

    return 0;
}

