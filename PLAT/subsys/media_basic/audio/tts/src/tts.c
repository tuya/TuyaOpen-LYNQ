#ifdef FEATURE_SUBSYS_TTS_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sctdef.h"
#include "cmsis_os2.h"
#include "mem_map.h"
#include "slpman.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif
#include "ivTTS.h"
#include "tts.h"
#include "audio.h"

#ifdef MBTK_OPENCPU_SUPPORT
#include "ivTTS_resource_irf.h"
#include "ol_log.h"
#endif
#include "api_codec.h"


#define TTS_HEAP_SIZE                       (80 * 1024)
#define TTS_SAMPLE_RATE                     SAMPLERATE_16K
#define BUFFER_SIZE                         3
#ifdef FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
#define FACTOR                              2
#define CHANNEL                             DUAL_CHANNEL
#else
#define FACTOR                              1
#define CHANNEL                             MONO
#endif
#define TTS_TX_SIZE                         (AUDIO_TX_TRANSFER_SIZE / FACTOR)

#ifdef EXTERNAL_TTS_LFS_ENABLE
#define TTS_CB_PARAM                        ((ivPointer)EF_IMG_TTS_LNA)
#else
#ifdef MBTK_OPENCPU_SUPPORT
#define TTS_CB_PARAM                        ((ivPointer)mlp_resource)
#else
#define TTS_CB_PARAM                        ((ivPointer)PKGFLXTTS_LNA)
#endif
#endif
#define TTS_FIRST_BUFFER_CONSUME            82


static ivHTTS gTts = NULL;
#if (PSRAM_EXIST == 1)
static PLAT_FPSRAM_ZI_CUST uint8_t gTtsHeap[TTS_HEAP_SIZE] = {0};
#else
static uint8_t gTtsHeap[TTS_HEAP_SIZE] = {0};
#endif


/* read resource callback */
ivBool ivCall readResCB(
		ivPointer		pParameter,		/* [in] user callback parameter */
		ivPointer		pBuffer,		/* [out] read resource buffer */
		ivResAddress	iPos,			/* [in] read start position */
		ivResSize		nSize )			/* [in] read size */
{
#ifdef EXTERNAL_TTS_LFS_ENABLE
    spiFlashRead((uint32_t)((uint8_t *)pParameter + iPos), (uint8_t *)pBuffer, nSize);
#else
    memcpy((uint8_t *)pBuffer, (uint8_t *)pParameter + iPos, nSize);
#endif
    return ivTrue;
}

/* output callback */
ivTTSErrID ivCall outputCB(
		ivPointer		pParameter,		/* [in] user callback parameter */
		ivUInt16		nCode,			/* [in] output data code */
		ivCPointer		pcData,			/* [in] output data buffer */
		ivSize			nSize )			/* [in] output data size */
{
    /* It is recommended to use the macro AUDIO_TX_TRANSFER_SIZE as the size of this buffer. */
#if (PSRAM_EXIST == 1)
    static __attribute__((aligned(16))) PLAT_FPSRAM_ZI_CUST uint8_t sBuffer[BUFFER_SIZE][AUDIO_TX_TRANSFER_SIZE];
#else
    static __attribute__((aligned(16))) uint8_t sBuffer[BUFFER_SIZE][AUDIO_TX_TRANSFER_SIZE];
#endif
    static volatile uint32_t sLength   = 0;
    static volatile uint32_t sIndex    = 0;
    uint32_t                 length    = 0;
    uint32_t                 remain    = 0;
    uint32_t                 ttsTxSize = TTS_TX_SIZE;
#ifdef FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
    uint8_t                  buffer[TTS_TX_SIZE];
#endif

    if (nSize > 0)
    {
        if ((sLength + nSize) > ttsTxSize)
        {
            length = ttsTxSize - sLength;
            remain = sLength + nSize - ttsTxSize;
            memcpy(&sBuffer[sIndex][sLength], pcData, length);
            memcpy(sBuffer[(sIndex + 1) % BUFFER_SIZE], (uint8_t *)pcData + length, remain);
            sLength = ttsTxSize;
        }
        else
        {
            memcpy(&sBuffer[sIndex][sLength], pcData, nSize);
            sLength += nSize;
        }
        if (sLength < ttsTxSize)
        {
            return ivTTS_ERR_OK;
        }
    }

    if (sLength > 0)
    {
        halI2sSrcAdjustVolumn((int16_t *)(sBuffer[sIndex]), sLength, audioGetVolume());

#ifdef FEATURE_SUBSYS_CHANNEL_1TO2_ENABLE
        memset(buffer, 0, sizeof(buffer));
        memcpy(buffer, sBuffer[sIndex], sLength);
        for (uint32_t i=0; i<sLength/2; i++)
        {
            for (uint32_t j=0; j<FACTOR; j++)
            {
                ((uint16_t *)(sBuffer[sIndex]))[FACTOR * i + j] = ((uint16_t *)buffer)[i];
            }
        }
#endif

        if (gCodecTx == CODEC_TX_IDLE)
        {
            apiCodecStart(TTS_SAMPLE_RATE, CHANNEL);
        }

        while(gCodecTx == CODEC_TX_START){;}
        gCodecTx = CODEC_TX_START;
        apiCodecPlay(sBuffer[sIndex], sLength * FACTOR);
        sIndex  = (sIndex + 1) % BUFFER_SIZE;
        sLength = remain;

        //osDelay(5);
    }

    return ivTTS_ERR_OK;
}

/* parameter change callback */
ivTTSErrID ivCall paramChangeCB(
		ivPointer       pParameter,		/* [in] user callback parameter */
		ivUInt32		nParamID,		/* [in] parameter id */
		ivUInt32		nParamValue )	/* [in] parameter value */
{
	return ivTTS_ERR_OK;
}

/* progress callback */
ivTTSErrID ivCall progressCB(
		ivPointer       pParameter,		/* [in] user callback parameter */
		ivUInt32		iProcPos,		/* [in] current processing position */
		ivUInt32		nProcLen )		/* [in] current processing length */
{
	return ivTTS_ERR_OK;
}

int32_t ttsInit(void)
{
    ivTTSErrID        ivReturn     = 0;
    ivTResPackDescExt tResPackDesc = {0};

    memset(&tResPackDesc, 0, sizeof(tResPackDesc));
    tResPackDesc.pCBParam = TTS_CB_PARAM;
    tResPackDesc.pfnRead  = readResCB;

    memset(gTtsHeap, 0, TTS_HEAP_SIZE);
    ivReturn = ivTTS_Create(&gTts, (ivPointer)gTtsHeap, TTS_HEAP_SIZE, ivNull, (ivPResPackDescExt)&tResPackDesc, (ivSize)1, NULL);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_Create return 0x%X\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_OUTPUT_CALLBACK, (ivUInt32)outputCB);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_OUTPUT_CALLBACK return 0x%x\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_PARAMCH_CALLBACK, (ivUInt32)paramChangeCB);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_PARAMCH_CALLBACK return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_PROGRESS_CALLBACK, (ivUInt32)progressCB);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_PROGRESS_CALLBACK return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_INPUT_CODEPAGE, ivTTS_CODEPAGE_UTF8);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_INPUT_CODEPAGE return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_AUTO);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_LANGUAGE return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_VOLUME, ivTTS_VOLUME_NORMAL);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_VOLUME return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_VOICE_SPEED, ivTTS_SPEED_NORMAL);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_VOICE_SPEED return 0x%X\r\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(gTts, ivTTS_PARAM_ROLE, ivTTS_ROLE_XIAOYAN);
    if (ivReturn)
    {
        SYSLOG_EMERG("ivTTS_SetParam ivTTS_PARAM_ROLE return 0x%X\r\n", ivReturn);
        return -1;
    }

    return 0;
}

void ttsPlay(char *text)
{
    if ((gTts != NULL) && (text != NULL))
    {
        gCodecTx = CODEC_TX_IDLE;
#ifdef EXTERNAL_TTS_LFS_ENABLE
        if (spiFlashExist() != true)
        {
            return;
        }
#endif

        apiCodecSetPaState(true);
        apiCodecBoot();

        osDelay(50);
        ivTTS_SynthText(gTts, ivText(text), strlen(text));
        outputCB(NULL, 0, NULL, 0);
        while (gCodecTx == CODEC_TX_START)
        {
            osDelay(5);
        }

        apiCodecStop();
    }
}
#endif
