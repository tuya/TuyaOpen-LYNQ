#if (defined(FEATURE_SUBSYS_AMR_ENABLE) || defined(FEATURE_SUBSYS_AMR_RECORD_ENABLE))
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sctdef.h"
#include "cmsis_os2.h"
#include DEBUG_LOG_HEADER_FILE
#include "i2s_device.h"
#include "ccio_audio.h"
#include "hal_voice_eng.h"
#include "hal_voice_eng_mem.h"
#include "audAmrCommon.h"
#include "ec_ring.h"
#include "medDataHandle.h"


#ifdef MBTK_OPENCPU_SUPPORT
#include "ol_log.h"
#endif

#define AMR_FIFO_BUFFER_SIZE        HAL_PCM_FRAME_SIZE
#define AMR_FIFO_SIZE               10

typedef struct
{
	uint8_t status;
	uint8_t engRunFlag;
	HalVoiceCodecConfigReq  codecCfg;
}amrEngInfo;

typedef struct
{
    uint32_t length;
    uint8_t  buffer[AMR_FIFO_BUFFER_SIZE];
} FifoNodeT;

typedef struct
{
    uint32_t  indexWrite;
    uint32_t  indexRead;
    FifoNodeT fifo[AMR_FIFO_SIZE];
} AmrFifoT;


static PLAT_FPSRAM_ZI_CUST AmrFifoT gAmrFifo           = {0};
static volatile uint32_t            gVoiceEngCnf       = 0;
static BOOL 						gVoiceRecordPcm   = FALSE;
const uint8_t                       gAmrNbHead[]       = {0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x34, 0x3C, 0x44, 0x4C, 0x54, 0x5C, 0x64, 0x6C, 0x74, 0x7C};
const uint16_t                       gAmrNbBitLength[]  = {95, 103, 118, 134, 148, 159, 204, 244, 39, 0, 0, 0, 0, 0, 0, 0};
const uint16_t                       gAmrNbByteLength[] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};

const uint16_t                       gAmrWbBitLength[]  = {132, 177, 253, 285, 317, 365, 397, 461, 477, 40, 0, 0, 0, 0, 0, 0};
const uint16_t                       gAmrWbByteLength[] = {17, 22, 32, 36, 40, 46, 50, 58, 60, 5, 0, 0, 0, 0, 0, 0};


uint32_t                            gTimeBegin         = 0;
amrEngInfo                          gAmrEngInfo = {AMR_ENG_STA_IDLE,0,{0}};


int32_t amrFifoInit(void)
{
    memset(&gAmrFifo, 0, sizeof(gAmrFifo));

    return 0;
}

bool amrFifoIsFull(void)
{
    return (((gAmrFifo.indexWrite + 1) % AMR_FIFO_SIZE) == gAmrFifo.indexRead);
}

bool amrFifoIsEmpty(void)
{
    return (gAmrFifo.indexRead == gAmrFifo.indexWrite);
}

int32_t amrFifoWrite(uint8_t *data, uint32_t length)
{
    int32_t retVal = -1;

    if ((data == NULL) || (length == 0) || (length > AMR_FIFO_BUFFER_SIZE))
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrFifoWrite_1, P_INFO, "[APP AMR] Param error");
        return retVal;
    }

    if (((gAmrFifo.indexWrite + 1) % AMR_FIFO_SIZE) == gAmrFifo.indexRead)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrFifoWrite_2, P_INFO, "[APP AMR] AMR FIFO is full");
        return retVal;
    }

    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrFifoWrite_3, P_INFO, "[APP AMR] indexWrite=%d, length=%d", gAmrFifo.indexWrite, length);
    memset(&gAmrFifo.fifo[gAmrFifo.indexWrite], 0, AMR_FIFO_BUFFER_SIZE);
    memcpy(gAmrFifo.fifo[gAmrFifo.indexWrite].buffer, data, length);
    gAmrFifo.fifo[gAmrFifo.indexWrite].length = length;

    gAmrFifo.indexWrite = (gAmrFifo.indexWrite + 1) % AMR_FIFO_SIZE;
    retVal = 0;

    return retVal;
}

int32_t amrFifoRead(uint8_t *buffer, uint32_t size)
{
    int32_t retVal = -1;

    if ((buffer == NULL) || (size == 0))
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrFifoRead_invalid_1, P_ERROR, "[APP AMR] Param error");
        return retVal;
    }

    if (gAmrFifo.indexRead == gAmrFifo.indexWrite)
    {
        // ECPLAT_PRINTF(UNILOG_PLA_APP, amrFifoRead_2, P_INFO, "[APP AMR] AMR FIFO is empty");
        return retVal;
    }

    if (size < gAmrFifo.fifo[gAmrFifo.indexRead].length)
    {
        ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrFifoRead_1, P_WARNING, "[APP AMR] Size too small: %d, %d", size, gAmrFifo.fifo[gAmrFifo.indexRead].length);
        return retVal;
    }

    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrFifoRead_2, P_INFO, "[APP AMR] indexRead=%d, length=%d", gAmrFifo.indexRead, gAmrFifo.fifo[gAmrFifo.indexRead].length);
    memset(buffer, 0, size);
    memcpy(buffer, gAmrFifo.fifo[gAmrFifo.indexRead].buffer, gAmrFifo.fifo[gAmrFifo.indexRead].length);

    retVal             = gAmrFifo.fifo[gAmrFifo.indexRead].length;
    gAmrFifo.indexRead = (gAmrFifo.indexRead + 1) % AMR_FIFO_SIZE;

    return retVal;
}
extern void ccioRbufUngetPtUlPcb(UlPduBlock_t *pcb);

void amrEngCallback(uint32_t msgId, void *msg)
{
    uint8_t            buffer[HAL_RTP_ONE_AMR_FRAME_MAX_SIZE] = {0};
    uint32_t           length = 0;
    HalVoiceEncodeCnf *msgEn  = (HalVoiceEncodeCnf *)msg;
    HalVoiceDecodeCnf *msgDe  = (HalVoiceDecodeCnf *)msg;

    switch (msgId)
    {
        case HAL_VOICE_ENG_START_CNF:
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_0, P_INFO, "[APP AMR] <HAL_VOICE_ENG_START_CNF>");
            break;

        case HAL_VOICE_ENG_STOP_CNF:
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_1, P_INFO, "[APP AMR] <HAL_VOICE_ENG_STOP_CNF>");
			amrEngSetRecPcmFlag(FALSE);
            break;

        case HAL_VOICE_CODEC_CONFIG_CNF:
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_2, P_INFO, "[APP AMR] <HAL_VOICE_CODEC_CONFIG_CNF>");
            break;

        case HAL_VOICE_ENCODE_CNF:
            if (gTimeBegin == 0)
            {
                gTimeBegin = osKernelGetTickCount();
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_3, P_WARNING, "[APP AMR] gTimeBegin=%d", gTimeBegin);
            }
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_4, P_INFO,
                          "[APP AMR] rc=%d, codecType=%d, sn=%d, frameType=%d, outBitOffset=%d, amrBitLen=%d, pAmrData=0x%X, pPcmData=0x%X",
                          msgEn->rc, msgEn->codecType, msgEn->sn, msgEn->frameType, msgEn->outBitOffset, msgEn->amrBitLen, msgEn->pAmrData, msgEn->pPcmData);

			ECPLAT_DUMP(UNILOG_PLA_APP, audioDataInput_dump, P_INFO,"rec pcm: ",320, msgEn->pPostVemPcmData);
            if ((msgEn->codecType != HAL_VC_AMR_NB) && (msgEn->codecType != HAL_VC_AMR_WB))
            {
                ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_invalid_1, P_ERROR, "[APP AMR] Codec type error");
            }
            else
            {
                memset(buffer, 0, sizeof(buffer));
				if(amrEngGetRecPcmFlag())
				{
					length = (msgEn->codecType == HAL_VC_AMR_NB ? HAL_8K_PCM_FRAME_SIZE : HAL_16K_PCM_FRAME_SIZE);
					amrFifoWrite(msgEn->pPostVemPcmData, length);
				}
				else
				{
					if (msgEn->codecType == HAL_VC_AMR_NB)
	                {
	                    if ((msgEn->frameType > HAL_AMR_NB_FT_8) || ((msgEn->frameType == HAL_AMR_NB_FT_8) && (msgEn->amrBitLen == 0)))
	                    {
	                        msgEn->frameType = HAL_AMR_NB_FT_15;
	                    }
	                    buffer[0] = gAmrNbHead[msgEn->frameType];
	                    length = 1;
	                }
	                else if (msgEn->codecType == HAL_VC_AMR_WB)
	                {
	                	if (msgEn->frameType > HAL_AMR_WB_FT_9)
	                    {
	                        msgEn->frameType = HAL_AMR_WB_FT_15;
	                    }
	                    buffer[0] = gAmrNbHead[msgEn->frameType];
	                    length = 1;
	                }
	                if (length == 1)
	                {
	                    if (msgEn->amrBitLen > 0)
	                    {
	                        length += msgEn->outBitOffset;
							if(msgEn->codecType == HAL_VC_AMR_NB)
							{
		                        memcpy(&buffer[length], msgEn->pAmrData, gAmrNbByteLength[msgEn->frameType]);
		                        length += gAmrNbByteLength[msgEn->frameType];
							}
							else
								{
		                        memcpy(&buffer[length], msgEn->pAmrData, gAmrWbByteLength[msgEn->frameType]);
		                        length += gAmrWbByteLength[msgEn->frameType];
							}
	                    }

	                    amrFifoWrite(buffer, length);
	                    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_5, P_INFO, "[APP AMR] AMR length=%d", length);
	                }
				}
                
                halVEFreeAmrEnFrameBuf(&msgEn->pExtra0);
#ifdef FEATURE_SUBSYS_AMR_RECORD_ENABLE
                //if ((gRecordState != AMR_RECORD_STOP) && (gRecordState != AMR_RECORD_ENDING_BY_VOLTE))
                if (gRecordState < AMR_RECORD_ENDING_BY_VOLTE) 
                {
                    ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_6, P_INFO, "[APP AMR] amrEngCallback free pExtra1:%p", msgEn->pExtra1);
                    audioFreeRecordBuf(msgEn->pExtra1);
                }
				else
				{
					ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_7, P_INFO, "[APP AMR] ccioRbufUngetPtUlPcb: %p", msgEn->pExtra1);
					ccioRbufUngetPtUlPcb(msgEn->pExtra1);
					gRecordState = AMR_RECORD_STOP;

				}
#endif
            }
            break;

        case HAL_VOICE_DECODE_CNF:
        	{
        	ecRingT *pcmCache = NULL;
			int32_t wrSize = 0;
            ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, voiceEngCallback_8, P_INFO,
                          "[APP AMR] rc=%d, codecType=%d, sn=%d, frameType=%d, pcmDataLen=%d, pPcmData=0x%x, pAmrData=0x%x",
                          msgDe->rc, msgDe->codecType, msgDe->sn, msgDe->frameType, msgDe->pcmDataLen, msgDe->pPcmData, msgDe->pAmrData);
			pcmCache = medGetDataCache();
			if(pcmCache)
			{
				wrSize = xEcRingWriteEx(pcmCache,msgDe->pPcmData,msgDe->pcmDataLen);
				if(wrSize < msgDe->pcmDataLen)
					ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, voiceEngCallback_invalid_2, P_ERROR,"[APP AMR] wr[%d] < pcmDataLen");				
			}
			ECPLAT_DUMP(UNILOG_PLA_APP, audioDataInupt_dump, P_VALUE,
                       "amr decode, body data: ",
                       msgDe->pcmDataLen, msgDe->pPcmData);
            break;
    	}
        default:
        	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCallback_invalid_3, P_ERROR, "[APP AMR] unexpected msgId<%d>",msgId);
            break;
    }

    gVoiceEngCnf |= (1 << msgId);
}

void amrEngWaitCpReply(uint32_t msgId)
{
    uint32_t mask = 1 << msgId;

    while ((gVoiceEngCnf & mask) == 0)
    {
        osDelay(1);
    }
    gVoiceEngCnf &= ~mask;
}

BOOL amrEngGetRecPcmFlag(void)
{
	return gVoiceRecordPcm;
}
void amrEngSetRecPcmFlag(BOOL value)
{
	gVoiceRecordPcm = value;
}

int8_t amrEngCommonStart(void       *codecCfg,uint8_t action)
{
	int8_t ret = 0;
	if(codecCfg == NULL)
		return -1;
	HalVoiceCodecConfigReq *cfg = (HalVoiceCodecConfigReq *)codecCfg;
	if(gAmrEngInfo.status <= AMR_ENG_STA_ENDING)
	{
		memcpy(&gAmrEngInfo.codecCfg,cfg,sizeof(HalVoiceCodecConfigReq));
		//TODO check start:
		halSetVoiceEngRetCallback(amrEngCallback);
		halVoiceEngStartReq();
	    amrEngWaitCpReply(HAL_VOICE_ENG_START_CNF);
	    halVoiceCodecConfigReq(&gAmrEngInfo.codecCfg);
	    amrEngWaitCpReply(HAL_VOICE_CODEC_CONFIG_CNF);
		gAmrEngInfo.status = AMR_ENG_STA_START;
	}
	else
	{
		if(cfg->codecType != gAmrEngInfo.codecCfg.codecType)
		{
			ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, amrEngCommonStart, P_ERROR, "bad codeType <%d>,curr<%d> ",cfg->codecType,gAmrEngInfo.codecCfg.codecType);
			return -1;
		}
	}
	gAmrEngInfo.engRunFlag |= action;
	return ret;
}

void amrEngCommonStop(uint8_t action)
{
	gAmrEngInfo.engRunFlag &= ~action;
	if(gAmrEngInfo.engRunFlag)
		return;
	halVoiceEngStopReq();
	amrEngWaitCpReply(HAL_VOICE_ENG_STOP_CNF);
	memset(&gAmrEngInfo.codecCfg,0x00,sizeof(HalVoiceCodecConfigReq));
	gAmrEngInfo.status = AMR_ENG_STA_ENDING;
}


#endif
