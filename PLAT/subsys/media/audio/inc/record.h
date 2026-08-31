#ifndef __RECORD_H__
#define __RECORD_H__

#include <stdint.h>

typedef struct
{
    char     *fileName;
    uint32_t  fileSize;
} RecordInfoT;

typedef void (*RecordCallbackT)(int32_t result);
typedef void ( *RecordGetDataCb)(void *,uint32_t);

typedef enum
{
	AUDIO_RECORD_CODEC_IDLE = 0,
	AUDIO_RECORD_CODEC_AMR,			///< recoord AMR
    AUDIO_RECORD_CODEC_G726, 		///< record G726
    AUDIO_RECORD_CODEC_ADPCM,		///< record ADPCM
    AUDIO_RECORD_CODEC_PCM,  		///< record PCM
    AUDIO_RECORD_CODEC_PCM_3A,		///< record 3A	
    AUDIO_RECORD_CODEC_EOF = 0xff
} openRecorderCodecT;

typedef enum 
{
	AV_RET_RECORD_ERROR = -1,
	AV_RET_RECORD_START = 0,
	AV_RET_RECORD_STOP,
	AV_RET_RECORD_EOF
}audRecordRespT;

typedef struct
{
	uint8_t channel; 		///< play channel mono or dual
	uint8_t samplerate; 	///< samplerate
	uint8_t codec; 			///< the codec to be used,reserved
	BOOL 	bContinue; 		///< record continue
	uint32_t time; 			///< record time:second,0----record continuously
	RecordGetDataCb dataCb; ///< record data callback
	uint32_t resv1; 		///reserved 1
	uint32_t resv2; 		///reserved 2
} RecordParamT;

typedef struct
{
    UINT8      		recType;
	RecordParamT    recordParam;
    uint32_t        length;
    char           *buffer;
    RecordCallbackT callback;
} QueueRecordT;

int32_t EC_audioRecord(char *path, RecordParamT recordParam, RecordCallbackT callback);
int32_t recordInit(void);

//mbtk change
typedef void (*ol_RecordCallbackT)(char* r_data, uint32_t r_size);
int32_t ol_audioRecord(RecordParamT recordParam, ol_RecordCallbackT callback);
void ol_audioRecordStop(void);

#endif
