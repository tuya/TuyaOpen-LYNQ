#ifndef __AMR_COMMON_H__
#define __AMR_COMMON_H__


#include <stdint.h>

#define AMR_CODEC_TYPE              HAL_VC_AMR_NB
#define AMR_NB_HEAD                 "#!AMR\n"
#define AMR_WB_HEAD                 "#!AMR-WB\n"
#define AMR_FT_COUNT                16
#define AMR_BIT_OFFSET              0
#define PCM_BIT_WIDTH               16

#define AUDIO_AMR_RECORD_STOP_NORMAL					0
#define AUDIO_AMR_RECORD_STOP_BY_VOLTE					1

#define AUDIO_AMR_ENG_ENCODE_FLAG	(0x01)
#define AUDIO_AMR_ENG_DECODE_FLAG	(0x10)

typedef enum _audAmrRecordStatus_e{
	AMR_RECORD_IDLE,
	AMR_RECORD_START = 1,
	AMR_RECORD_ENDING_NOMAL = 2,
	AMR_RECORD_ENDING_BY_VOLTE = 3,
	AMR_RECORD_STOP = 4,
}audAmrRecordStatus_e;

typedef enum _audAmrEngStatus_e{
	AMR_ENG_STA_IDLE = 0,
	AMR_ENG_STA_ENDING = AMR_ENG_STA_IDLE,
	AMR_ENG_STA_START
}audAmrEngStatus_e;

extern const uint8_t    gAmrNbHead[];
extern const uint16_t    gAmrNbBitLength[];
extern const uint16_t    gAmrNbByteLength[];
extern const uint16_t    gAmrWbBitLength[];
extern const uint16_t    gAmrWbByteLength[];

extern uint32_t         gTimeBegin;
extern volatile uint8_t gRecordState;


int32_t  amrFifoInit(void);
bool     amrFifoIsFull(void);
bool     amrFifoIsEmpty(void);
int32_t  amrFifoWrite(uint8_t *data, uint32_t length);
int32_t  amrFifoRead(uint8_t *buffer, uint32_t size);
void     amrEngCallback(uint32_t msgId, void *msg);
void     amrEngWaitCpReply(uint32_t msgId);
BOOL 	 amrEngGetRecPcmFlag(void);
void 	 amrEngSetRecPcmFlag(BOOL value);
int8_t 	 amrEngCommonStart(void       *codecCfg,uint8_t action);
void 	 amrEngCommonStop(uint8_t action);


#endif
